#include "PluginProcessor.h"
#include "PluginEditor.h"

#ifdef _WIN32
#include <windows.h>
#endif

// Path to Altiverb 7 XL - now configurable via registry
// const char* ALTIVERB_PATH = "C:\\Program Files\\VSTPlugins\\Altiverb 7\\Altiverb 7.dll";

AltiverbSurroundProcessor::AltiverbSurroundProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::create5point1(), true)
                       .withOutput ("Output", juce::AudioChannelSet::create5point1(), true))
{
    // WORKING VERSION - instance-specific logging to avoid handle conflicts
    #ifdef _WIN32
    errno_t err = fopen_s(&logFile, "C:\\IT\\code\\altiverb_wrapper_log.txt", "a");
    if (err == 0 && logFile) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        fprintf(logFile, "==========================================\n");
        fprintf(logFile, "NEW INSTANCE: %02d:%02d:%02d.%03d\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        fprintf(logFile, "AltiverbSurroundProcessor: Constructor started\n");
        fprintf(logFile, "==========================================\n");
        fflush(logFile);
    }
    #endif
    
    // Create VST2Loader but don't load plugin yet
    vst2Loader = std::make_unique<VST2Loader>();
    
    if (logFile) {
        fprintf(logFile, "AltiverbSurroundProcessor: Constructor finished\n");
        fflush(logFile);
    }
}

AltiverbSurroundProcessor::~AltiverbSurroundProcessor() {
    #ifdef _WIN32
    if (logFile) {
        fprintf(logFile, "=== DESTRUCTOR CALLED ===\n");
        fflush(logFile);
        
        if (vst2Loader) {
            fprintf(logFile, "Resetting VST2 loader...\n");
            fflush(logFile);
            vst2Loader.reset();
            fprintf(logFile, "VST2 loader reset complete\n");
            fflush(logFile);
        }
        
        fprintf(logFile, "=== DESTRUCTOR COMPLETE ===\n");
        fflush(logFile);
        fclose(logFile);
        logFile = nullptr;
    }
    #endif
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
    
    // Try to load Altiverb if not loaded yet (working version pattern)
    if (!pluginLoaded) {
        printf("=== ATTEMPTING TO LOAD ALTIVERB VST2 ===\n");
        
        // Get configurable VST2 path
        juce::String vst2Path = getVST2Path();
        printf("Path: %s\n", vst2Path.toUTF8());
        fflush(stdout);
        
        if (vst2Loader->loadPlugin(vst2Path.toUTF8())) {
            pluginLoaded = true;
            printf("=== SUCCESS: ALTIVERB VST2 LOADED! ===\n");
            fflush(stdout);
            
            int numParams = vst2Loader->getNumParameters();
            printf("Altiverb has %d parameters\n", numParams);
            printf("Altiverb has editor: %s\n", vst2Loader->hasEditor() ? "YES" : "NO");
            fflush(stdout);
        }
    }
    
    if (pluginLoaded) {
        vst2Loader->setSampleRate(sampleRate);
        vst2Loader->setBlockSize(samplesPerBlock);
        vst2Loader->resume();
        
        // FORCE 5.1 CONFIGURATION AFTER RESUME
        printf("=== FORCING 5.1 CONFIGURATION ===\n");
        fflush(stdout);
        
        auto* effect = vst2Loader->getEffect();
        if (effect) {
            // Force 6 channels
            effect->numInputs = 6;
            effect->numOutputs = 6;
            printf("Forced channels to 6/6\n");
            
            // Setup 5.1 speaker arrangements
            VstSpeakerArrangement inputs, outputs;
            inputs.type = kSpeakerArr51;
            inputs.numChannels = 6;
            outputs.type = kSpeakerArr51;
            outputs.numChannels = 6;
            
            // Try setting speaker arrangement
            VstIntPtr result = effect->dispatcher(effect, effSetSpeakerArrangement, 0, 
                                                 (VstIntPtr)&inputs, &outputs, 0.0f);
            printf("effSetSpeakerArrangement result: %lld\n", (long long)result);
            fflush(stdout);
        }
        
        printf("Altiverb prepared: %f Hz, %d samples\n", sampleRate, samplesPerBlock);
        fflush(stdout);
    }
}

void AltiverbSurroundProcessor::releaseResources() {
    printf("=== releaseResources called ===\n");
    fflush(stdout);
    
    if (pluginLoaded) {
        printf("Suspending Altiverb...\n");
        fflush(stdout);
        vst2Loader->suspend();
        printf("Altiverb suspended successfully\n");
        fflush(stdout);
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
    
    static int logCounter = 0;
    if (++logCounter % 4800 == 0) { // Log every ~10 seconds at 48kHz
        printf("=== PROCESSING AUDIO ===\n");
        printf("Channels: %d, Samples: %d\n", buffer.getNumChannels(), buffer.getNumSamples());
        printf("Altiverb: %s\n", pluginLoaded ? "LOADED & PROCESSING" : "PASSTHROUGH");
        fflush(stdout);
    }
    
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
    printf("=== hasEditor() called - returning TRUE for button GUI ===\n");
    fflush(stdout);
    
    // Return true like working version
    return true;
}

juce::AudioProcessorEditor* AltiverbSurroundProcessor::createEditor() {
    printf("=== createEditor() called - creating simple button GUI ===\n");
    fflush(stdout);
    
    auto* editor = new AltiverbSurroundEditor(*this);
    printf("Simple GUI Editor created successfully: %p\n", editor);
    fflush(stdout);
    return editor;
}

void AltiverbSurroundProcessor::getStateInformation(juce::MemoryBlock& destData) {
    printf("=== getStateInformation called ===\n");
    fflush(stdout);
    
    destData.setSize(0);
    printf("State info: returned empty data\n");
    fflush(stdout);
}

void AltiverbSurroundProcessor::setStateInformation(const void* data, int sizeInBytes) {
    printf("=== setStateInformation called ===\n");
    printf("Set state: %d bytes\n", sizeInBytes);
    fflush(stdout);
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
        "C:\\Program Files\\Audio Ease\\Altiverb 7 XL\\Altiverb 7.dll",
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
    LONG result = RegCreateKeyExA(HKEY_CURRENT_USER, 
                                "SOFTWARE\\AltiverbWrapper", 
                                0, NULL, REG_OPTION_NON_VOLATILE,
                                KEY_WRITE, NULL, &hKey, NULL);
    
    if (result == ERROR_SUCCESS) {
        juce::String pathStr = path;
        RegSetValueExA(hKey, "VST2Path", 0, REG_SZ, 
                      (BYTE*)pathStr.toRawUTF8(), 
                      pathStr.length() + 1);
        RegCloseKey(hKey);
        
        if (logFile) {
            fprintf(logFile, "VST2 path saved to registry: %s\n", path.toUTF8());
            fflush(logFile);
        }
    }
    #endif
}

juce::String AltiverbSurroundProcessor::loadVST2PathFromRegistry() {
    #ifdef _WIN32
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, 
                               "SOFTWARE\\AltiverbWrapper", 
                               0, KEY_READ, &hKey);
    
    if (result == ERROR_SUCCESS) {
        char buffer[MAX_PATH];
        DWORD bufferSize = sizeof(buffer);
        DWORD type;
        
        result = RegQueryValueExA(hKey, "VST2Path", NULL, &type, 
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