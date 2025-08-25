#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Forward declaration
class AltiverbSurroundEditor;

// Custom DocumentWindow that can handle close button
class AltiverbDocumentWindow : public juce::DocumentWindow {
public:
    AltiverbDocumentWindow(AltiverbSurroundEditor* editor);
    void closeButtonPressed() override;
    
private:
    AltiverbSurroundEditor* parentEditor;
};

class AltiverbSurroundEditor : public juce::AudioProcessorEditor,
                               private juce::Timer
{
public:
    AltiverbSurroundEditor(AltiverbSurroundProcessor&);
    ~AltiverbSurroundEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;

private:
    AltiverbSurroundProcessor& audioProcessor;
    
    // Simple button GUI components
    juce::TextButton openButton;
    juce::TextButton browseButton;
    juce::Label statusLabel;
    juce::Label pathLabel;
    
    // Popup window for Altiverb
    std::unique_ptr<AltiverbDocumentWindow> altiverbWindow;
    
    void openAltiverbWindow();
    void browseForVST2Path();
    
public:
    void closeAltiverbWindow();
    
private:
    
    // Legacy embedded approach (kept for reference)
    #if JUCE_WINDOWS && 0  // Disabled - causes crashes
    class VST2Window : public juce::Component {
    public:
        VST2Window(VST2Loader* loader);
        ~VST2Window();
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        
        void* getNativeHandle() { return hwnd; }
        
    private:
        VST2Loader* vst2Loader;
        void* hwnd = nullptr;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VST2Window)
    };
    
    std::unique_ptr<VST2Window> vst2Window;
    #endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AltiverbSurroundEditor)
};