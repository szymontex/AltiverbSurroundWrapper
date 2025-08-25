#include "PluginProcessor.h"
#include "PluginEditor.h"

#ifdef _WIN32
#include <windows.h>
#endif

// VST2 path is now configurable via registry and GUI

AltiverbSurroundProcessor::AltiverbSurroundProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::create5point1(), true)
                       .withOutput ("Output", juce::AudioChannelSet::create5point1(), true))
{
    // Create VST2Loader but don't load plugin yet
    vst2Loader = std::make_unique<VST2Loader>();
}

AltiverbSurroundProcessor::~AltiverbSurroundProcessor() {
    vst2Loader.reset();
}

const juce::String AltiverbSurroundProcessor::getName() const {
    return "Altiverb 7 Surround";
}

bool AltiverbSurroundProcessor::acceptsMidi() const { return false; }
bool AltiverbSurroundProcessor::producesMidi() const { return false; }
bool AltiverbSurroundProcessor::isMidiEffect() const { return false; }
double AltiverbSurroundProcessor::getTailLengthSeconds() const { return 0.0; }

int AltiverbSurroundProcessor::getNumPrograms() {
    if (!pluginLoaded) return 1;
    return vst2Loader->getNumPrograms();
}

int AltiverbSurroundProcessor::getCurrentProgram() {
    if (!pluginLoaded) return 0;
    return vst2Loader->getCurrentProgram();
}

void AltiverbSurroundProcessor::setCurrentProgram(int index) {
    if (pluginLoaded) {
        vst2Loader->setCurrentProgram(index);
    }
}

const juce::String AltiverbSurroundProcessor::getProgramName(int index) {
    if (!pluginLoaded) return {};
    return vst2Loader->getProgramName(index);
}

void AltiverbSurroundProcessor::changeProgramName(int index, const juce::String& newName) {
    // Not implemented for VST2 wrapper
}

void AltiverbSurroundProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    
    // Prepare internal buffers for 6 channels (5.1)
    internalInputBuffer.setSize(6, samplesPerBlock);
    internalOutputBuffer.setSize(6, samplesPerBlock);
    
    // Setup channel pointers
    inputChannelPtrs.resize(6);
    outputChannelPtrs.resize(6);
    
    // Try to load Altiverb if not loaded yet
    if (!pluginLoaded) {
        juce::String vst2Path = getVST2Path();
        
        if (vst2Loader->loadPlugin(vst2Path.toUTF8())) {
            pluginLoaded = true;
        }
    }
    
    if (pluginLoaded) {
        vst2Loader->setSampleRate(sampleRate);
        vst2Loader->setBlockSize(samplesPerBlock);
        vst2Loader->resume();
        
        // Force 5.1 configuration after resume
        auto* effect = vst2Loader->getEffect();
        if (effect) {
            // Force 6 channels
            effect->numInputs = 6;
            effect->numOutputs = 6;
            
            // Setup 5.1 speaker arrangements
            VstSpeakerArrangement inputs, outputs;
            inputs.type = kSpeakerArr51;
            inputs.numChannels = 6;
            outputs.type = kSpeakerArr51;
            outputs.numChannels = 6;
            
            // Set speaker arrangement
            effect->dispatcher(effect, effSetSpeakerArrangement, 0, 
                             (VstIntPtr)&inputs, &outputs, 0.0f);
        }
    }
}

void AltiverbSurroundProcessor::releaseResources() {
    if (pluginLoaded) {
        vst2Loader->suspend();
    }
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AltiverbSurroundProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    // Simple: only support 5.1 in and 5.1 out
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::create5point1())
        return false;
    
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::create5point1())
        return false;
    
    return true;
}
#endif

void AltiverbSurroundProcessor::mapInputChannels(const juce::AudioBuffer<float>& buffer) {
    int numSamples = buffer.getNumSamples();
    
    for (int ch = 0; ch < 6 && ch < buffer.getNumChannels(); ++ch) {
        internalInputBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
    }
    
    for (int ch = buffer.getNumChannels(); ch < 6; ++ch) {
        internalInputBuffer.clear(ch, 0, numSamples);
    }
}

void AltiverbSurroundProcessor::mapOutputChannels(juce::AudioBuffer<float>& buffer) {
    int numSamples = buffer.getNumSamples();
    
    for (int ch = 0; ch < 6 && ch < buffer.getNumChannels(); ++ch) {
        buffer.copyFrom(ch, 0, internalOutputBuffer, ch, 0, numSamples);
    }
}

void AltiverbSurroundProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    
    if (!pluginLoaded) {
        // Simple passthrough mode
        return;
    }
    
    // Process with Altiverb
    int numSamples = buffer.getNumSamples();
    
    // Ensure buffers are the right size
    if (internalInputBuffer.getNumSamples() < numSamples) {
        internalInputBuffer.setSize(6, numSamples);
        internalOutputBuffer.setSize(6, numSamples);
    }
    
    // Map input channels
    mapInputChannels(buffer);
    
    // Setup channel pointers for VST2 processing
    for (int ch = 0; ch < 6; ++ch) {
        inputChannelPtrs[ch] = internalInputBuffer.getWritePointer(ch);
        outputChannelPtrs[ch] = internalOutputBuffer.getWritePointer(ch);
    }
    
    // Process through VST2
    if (vst2Loader && vst2Loader->getEffect()) {
        vst2Loader->processReplacing(inputChannelPtrs.data(), outputChannelPtrs.data(), numSamples);
    }
    
    // Map output channels back
    mapOutputChannels(buffer);
}

bool AltiverbSurroundProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessorEditor* AltiverbSurroundProcessor::createEditor() {
    return new AltiverbSurroundEditor(*this);
}

void AltiverbSurroundProcessor::getStateInformation(juce::MemoryBlock& destData) {
    destData.setSize(0);
}

void AltiverbSurroundProcessor::setStateInformation(const void* data, int sizeInBytes) {
    // State restoration not currently implemented
}

// VST2 Path Configuration Methods
juce::String AltiverbSurroundProcessor::getVST2Path() {
    // Try to load from registry first
    juce::String savedPath = loadVST2PathFromRegistry();
    if (savedPath.isNotEmpty() && juce::File(savedPath).existsAsFile()) {
        return savedPath;
    }
    
    // Try common default locations
    std::vector<juce::String> commonPaths = {
        "C:\\Program Files\\VSTPlugins\\Altiverb 7\\Altiverb 7.dll",
        "C:\\!_SYMLINK\\Audioease - Altiverb 7 XL 7.2.8 VST x64 (NO INSTALL, SymLink Installer) [VSEGDA KRASIVO REPACK 21.06.2022]\\C\\Program Files\\VSTPlugins\\Altiverb 7\\Altiverb 7.dll",
        "C:\\Program Files (x86)\\VSTPlugins\\Altiverb 7\\Altiverb 7.dll",
        "C:\\Program Files\\Audio Ease\\Altiverb 7\\Altiverb 7.dll"
    };
    
    for (const auto& path : commonPaths) {
        if (juce::File(path).existsAsFile()) {
            // Save the found path for next time
            saveVST2Path(path);
            return path;
        }
    }
    
    // Return default if nothing found
    return "C:\\Program Files\\VSTPlugins\\Altiverb 7\\Altiverb 7.dll";
}

void AltiverbSurroundProcessor::saveVST2Path(const juce::String& path) {
    #ifdef _WIN32
    // Save to Windows Registry
    HKEY hKey;
    LONG result = RegCreateKeyEx(HKEY_CURRENT_USER, 
                                L"SOFTWARE\\AltiverbWrapper", 
                                0, NULL, REG_OPTION_NON_VOLATILE,
                                KEY_WRITE, NULL, &hKey, NULL);
    
    if (result == ERROR_SUCCESS) {
        std::wstring widePath = path.toWideCharPointer();
        RegSetValueEx(hKey, L"VST2Path", 0, REG_SZ, 
                     (BYTE*)widePath.c_str(), 
                     (widePath.length() + 1) * sizeof(wchar_t));
        RegCloseKey(hKey);
        
    }
    #endif
}

juce::String AltiverbSurroundProcessor::loadVST2PathFromRegistry() {
    #ifdef _WIN32
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, 
                              L"SOFTWARE\\AltiverbWrapper", 
                              0, KEY_READ, &hKey);
    
    if (result == ERROR_SUCCESS) {
        wchar_t buffer[MAX_PATH];
        DWORD bufferSize = sizeof(buffer);
        DWORD type;
        
        result = RegQueryValueEx(hKey, L"VST2Path", NULL, &type, 
                               (BYTE*)buffer, &bufferSize);
        
        if (result == ERROR_SUCCESS && type == REG_SZ) {
            RegCloseKey(hKey);
            return juce::String(buffer);
        }
        RegCloseKey(hKey);
    }
    #endif
    
    return juce::String();
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new AltiverbSurroundProcessor();
}