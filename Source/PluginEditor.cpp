#include "PluginProcessor.h"
#include "PluginEditor.h"

#ifdef _WIN32
#include <windows.h>
#endif

// Implementation of AltiverbDocumentWindow
AltiverbDocumentWindow::AltiverbDocumentWindow(AltiverbSurroundEditor* editor)
    : DocumentWindow("Altiverb 7 XL", juce::Colours::darkgrey, 
                     juce::DocumentWindow::closeButton | juce::DocumentWindow::minimiseButton),
      parentEditor(editor)
{
}

void AltiverbDocumentWindow::closeButtonPressed() {
    if (parentEditor) {
        parentEditor->closeAltiverbWindow();
    }
}

AltiverbSurroundEditor::AltiverbSurroundEditor(AltiverbSurroundProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Create simple "Open Altiverb" button interface
    setSize(400, 160);
    
    // Add "Open Altiverb Editor" button
    openButton.setButtonText("Open Altiverb Editor");
    openButton.onClick = [this] {
        openAltiverbWindow();
    };
    addAndMakeVisible(openButton);
    
    // Add "Browse VST2 Path" button
    browseButton.setButtonText("Browse VST2 Path...");
    browseButton.onClick = [this] {
        browseForVST2Path();
    };
    addAndMakeVisible(browseButton);
    
    // Add status label
    statusLabel.setText("Altiverb 7 XL Surround Wrapper", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);
    
    // Add path label to show current VST2 path
    juce::String currentPath = audioProcessor.getVST2Path();
    juce::String shortPath = currentPath.substring(currentPath.lastIndexOf("\\") + 1);
    pathLabel.setText("VST2: " + shortPath, juce::dontSendNotification);
    pathLabel.setJustificationType(juce::Justification::centred);
    pathLabel.setFont(juce::Font(11.0f));
    addAndMakeVisible(pathLabel);
}

AltiverbSurroundEditor::~AltiverbSurroundEditor() {
    closeAltiverbWindow();
}

void AltiverbSurroundEditor::paint(juce::Graphics& g) {
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    if (!audioProcessor.getVST2Loader()) {
        g.setColour(juce::Colours::white);
        g.drawText("Altiverb VST2 not loaded", getLocalBounds(),
                  juce::Justification::centred, true);
    }
}

void AltiverbSurroundEditor::resized() {
    auto bounds = getLocalBounds();
    
    // Layout simple button interface
    statusLabel.setBounds(bounds.removeFromTop(25).reduced(10, 5));
    pathLabel.setBounds(bounds.removeFromTop(20).reduced(10, 2));
    
    auto buttonArea = bounds.reduced(20, 10);
    openButton.setBounds(buttonArea.removeFromTop(40));
    buttonArea.removeFromTop(10); // spacing
    browseButton.setBounds(buttonArea.removeFromTop(40));
}

void AltiverbSurroundEditor::timerCallback() {
    // Simplified - no idle updates needed for button interface
}

void AltiverbSurroundEditor::openAltiverbWindow() {
    // Check if Altiverb is loaded and has editor
    auto* loader = audioProcessor.getVST2Loader();
    if (!loader || !loader->hasEditor()) {
        return;
    }
    
    // Prevent multiple opens
    if (altiverbWindow) {
        altiverbWindow->toFront(true);
        return;
    }
    
    try {
        // Get Altiverb editor size
        int width = 800, height = 600;
        loader->getEditorSize(width, height);
        
        // Create custom DocumentWindow for Altiverb with proper close handling
        altiverbWindow = std::make_unique<AltiverbDocumentWindow>(this);
        
        altiverbWindow->setResizable(false, false);
        altiverbWindow->setSize(width, height);
        altiverbWindow->centreWithSize(width, height);
        
        // Make window visible BEFORE opening VST2 editor
        altiverbWindow->setVisible(true);
        
        // Small delay to ensure window is fully created
        juce::Thread::sleep(50);
        
        // Create VST2 editor content for popup window
        #if JUCE_WINDOWS
        auto* hwnd = (HWND)altiverbWindow->getWindowHandle();
        if (hwnd) {
            loader->openEditor(hwnd);
        } else {
            closeAltiverbWindow();
            return;
        }
        #endif
    }
    catch (...) {
        closeAltiverbWindow();
    }
}

void AltiverbSurroundEditor::closeAltiverbWindow() {
    if (altiverbWindow) {
        // Let the DAW handle VST2 editor lifecycle
        altiverbWindow.reset();
    }
}

void AltiverbSurroundEditor::browseForVST2Path() {
    // Create file chooser for VST2 DLL files
    auto chooser = std::make_unique<juce::FileChooser>("Select Altiverb VST2 Plugin",
                                                      juce::File("C:\\Program Files"),
                                                      "*.dll");
    
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    
    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
        auto results = fc.getResults();
        if (results.size() > 0) {
            juce::File selectedFile = results[0];
            juce::String selectedPath = selectedFile.getFullPathName();
            
            // Save the path to registry
            audioProcessor.saveVST2Path(selectedPath);
            
            // Update the path label
            juce::String shortPath = selectedPath.substring(selectedPath.lastIndexOf("\\") + 1);
            pathLabel.setText("VST2: " + shortPath, juce::dontSendNotification);
        }
    });
}