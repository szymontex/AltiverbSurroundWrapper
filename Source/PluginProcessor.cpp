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
    // Add dummy parameter to ensure state management is called
    addParameter(dummyParam = new juce::AudioParameterFloat("dummy", "Dummy", 0.0f, 1.0f, 0.0f));
    
    // Create VST2Loader and try to load plugin immediately
    vst2Loader = std::make_unique<VST2Loader>();
    
    // Try to load Altiverb early so it's available for state management
    juce::String vst2Path = getVST2Path();
    if (juce::File(vst2Path).existsAsFile()) {
        if (vst2Loader->loadPlugin(vst2Path.toUTF8())) {
            pluginLoaded = true;
            
        }
    }
}

AltiverbSurroundProcessor::~AltiverbSurroundProcessor() {
    // Clean shutdown
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
        
        // FORCE 5.1 CONFIGURATION AFTER RESUME
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
            
            // Try setting speaker arrangement
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
    
    // Create XML to store our wrapper state + VST2 state
    std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("AltiverbSurroundWrapperState"));
    
    // Always save basic wrapper state
    xml->setAttribute("version", "1.1.0");
    xml->setAttribute("pluginLoaded", pluginLoaded ? "true" : "false");
    
    // Save VST2 path for this project
    juce::String currentPath = getVST2Path();
    xml->setAttribute("vst2Path", currentPath);
    
    // Get state from loaded Altiverb VST2 plugin
    if (pluginLoaded && vst2Loader) {
        auto* effect = vst2Loader->getEffect();
        if (effect) {
            bool supportsChunks = (effect->flags & effFlagsProgramChunks) != 0;
            int numParams = vst2Loader->getNumParameters();
            
            
            // Hybrid approach: Save BOTH chunks and parameters for maximum reliability
            bool chunkSaved = false;
            if (supportsChunks) {
                void* chunkData = nullptr;
                int chunkSize = vst2Loader->getChunk(&chunkData, false);
                if (chunkSize > 0 && chunkData != nullptr) {
                    juce::MemoryBlock vstState(chunkData, chunkSize);
                    juce::String encodedState = vstState.toBase64Encoding();
                    xml->setAttribute("vstState", encodedState);
                    xml->setAttribute("chunkSize", chunkSize);
                    chunkSaved = true;
                }
            }
            
            // ALWAYS save individual parameters as backup (even with chunks)
            if (numParams > 0) {
                juce::XmlElement* paramsXml = xml->createNewChildElement("Parameters");
                for (int i = 0; i < numParams; ++i) {
                    float value = vst2Loader->getParameter(i);
                    paramsXml->setAttribute("param" + juce::String(i), value);
                }
                // Also save current program
                paramsXml->setAttribute("currentProgram", vst2Loader->getCurrentProgram());
                
            }
        }
    }
    
    // Convert XML to memory block
    copyXmlToBinary(*xml, destData);
}

void AltiverbSurroundProcessor::setStateInformation(const void* data, int sizeInBytes) {
    
    if (sizeInBytes == 0) return;
    
    // Parse XML from memory block
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml == nullptr) return;
    
    // Restore VST2 path for this project
    if (xml->hasAttribute("vst2Path")) {
        juce::String projectVst2Path = xml->getStringAttribute("vst2Path");
        
        // Try to load the VST2 plugin with the saved path
        if (juce::File(projectVst2Path).existsAsFile()) {
            // Load the VST2 plugin if not already loaded
            if (!pluginLoaded && vst2Loader) {
                if (vst2Loader->loadPlugin(projectVst2Path.toUTF8())) {
                    pluginLoaded = true;
                    
                    // Setup audio processing
                    if (currentSampleRate > 0) {
                        vst2Loader->setSampleRate(currentSampleRate);
                        vst2Loader->setBlockSize(currentBlockSize);
                        vst2Loader->resume();
                    }
                }
            }
        }
    }
    
    // Restore VST2 plugin state (presets, parameters)
    if (pluginLoaded && vst2Loader) {
        auto* effect = vst2Loader->getEffect();
        if (effect) {
            // Hybrid restoration: Try chunks first, then ALWAYS restore parameters as well
            bool chunkRestored = false;
            if (xml->hasAttribute("vstState") && (effect->flags & effFlagsProgramChunks)) {
                juce::String encodedState = xml->getStringAttribute("vstState");
                juce::MemoryBlock vstState;
                vstState.fromBase64Encoding(encodedState);
                
                if (vstState.getSize() > 0) {
                    if (vst2Loader->setChunk(vstState.getData(), (int)vstState.getSize(), false)) {
                        chunkRestored = true;
                        // Give plugin time to process chunk
                        juce::Thread::sleep(50);
                    }
                }
            }
            
            // ALWAYS restore parameters as well (even if chunks worked)
            if (auto* paramsXml = xml->getChildByName("Parameters")) {
                // Step 1: Restore current program first
                if (paramsXml->hasAttribute("currentProgram")) {
                    int program = paramsXml->getIntAttribute("currentProgram", 0);
                    vst2Loader->setCurrentProgram(program);
                    
                    // Give plugin time to process program change
                    juce::Thread::sleep(10);
                }
                
                // Step 2: Suspend plugin before parameter restore
                vst2Loader->suspend();
                
                // Step 3: Restore all parameters
                int numParams = vst2Loader->getNumParameters();
                int restoredCount = 0;
                for (int i = 0; i < numParams; ++i) {
                    juce::String paramName = "param" + juce::String(i);
                    if (paramsXml->hasAttribute(paramName)) {
                        float value = (float)paramsXml->getDoubleAttribute(paramName, 0.0);
                        vst2Loader->setParameter(i, value);
                        restoredCount++;
                    }
                }
                
                // Step 4: Resume plugin after parameter restore
                if (currentSampleRate > 0) {
                    vst2Loader->setSampleRate(currentSampleRate);
                    vst2Loader->setBlockSize(currentBlockSize);
                }
                vst2Loader->resume();
                
            }
        }
    }
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