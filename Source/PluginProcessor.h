#pragma once
#include <JuceHeader.h>
#include "VST2Loader.h"

class AltiverbSurroundProcessor : public juce::AudioProcessor
{
public:
    AltiverbSurroundProcessor();
    ~AltiverbSurroundProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // VST2 access for working version
    VST2Loader* getVST2Loader() { return vst2Loader.get(); }
    
    // VST2 path configuration access
    juce::String getVST2Path();
    void saveVST2Path(const juce::String& path);

private:
    std::unique_ptr<VST2Loader> vst2Loader;
    
    // VST3 parameters (required for state management)
    juce::AudioParameterFloat* dummyParam;
    
    // Audio buffers for VST2 processing
    std::vector<float*> inputChannelPtrs;
    std::vector<float*> outputChannelPtrs;
    juce::AudioBuffer<float> internalInputBuffer;
    juce::AudioBuffer<float> internalOutputBuffer;
    
    bool pluginLoaded = false;
    double currentSampleRate = 48000.0;
    int currentBlockSize = 512;
    
    // Instance-specific logging
    void logMessage(const char* format, ...);
    
    // VST2 path configuration (private helper)
    juce::String loadVST2PathFromRegistry();
    
    // Channel mapping for 5.1
    void mapInputChannels(const juce::AudioBuffer<float>& buffer);
    void mapOutputChannels(juce::AudioBuffer<float>& buffer);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AltiverbSurroundProcessor)
};