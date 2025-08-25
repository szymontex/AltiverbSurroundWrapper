#include "VST2Loader.h"

AudioMasterCallback VST2Loader::originalHostCallback = nullptr;

// Static speaker arrangements for 5.1 setup
VstSpeakerArrangement VST2Loader::inputArrangement;
VstSpeakerArrangement VST2Loader::outputArrangement;

VST2Loader::VST2Loader() {
    setup51Arrangement(inputArrangement);
    setup51Arrangement(outputArrangement);
}

VST2Loader::~VST2Loader() {
    try {
        unloadPlugin();
    }
    catch (...) {
        // Silent error handling in destructor
    }
}

void VST2Loader::setup51Arrangement(VstSpeakerArrangement& arrangement) {
    arrangement.type = kSpeakerArr51;
    arrangement.numChannels = 6;
    
    // L, R, C, LFE, Ls, Rs
    const char* names[6] = { "L", "R", "C", "LFE", "Ls", "Rs" };
    const float azimuths[6] = { -30.0f, 30.0f, 0.0f, 0.0f, -110.0f, 110.0f };
    
    for (int i = 0; i < 6; ++i) {
        arrangement.speakers[i].azimuth = azimuths[i];
        arrangement.speakers[i].elevation = 0.0f;
        arrangement.speakers[i].radius = 1.0f;
        arrangement.speakers[i].reserved = 0.0f;
        strncpy(arrangement.speakers[i].name, names[i], 63);
        arrangement.speakers[i].name[63] = 0;
        arrangement.speakers[i].type = i;
    }
}

VstIntPtr VSTCALLBACK VST2Loader::hostCallback(AEffect* effect, VstInt32 opcode, 
                                               VstInt32 index, VstIntPtr value, 
                                               void* ptr, float opt) {
    switch (opcode) {
        case audioMasterVersion:
            return 2400;  // VST 2.4
            
        case audioMasterAutomate:
            return 0;
            
        case audioMasterGetTime:
            return 0;
            
        case audioMasterIOChanged:
            return 1;
            
        case audioMasterGetInputSpeakerArrangement:
            if (ptr) {
                VstSpeakerArrangement* arrangement = static_cast<VstSpeakerArrangement*>(ptr);
                *arrangement = VST2Loader::getInputArrangement();
            }
            return 1;
            
        case audioMasterGetOutputSpeakerArrangement:
            if (ptr) {
                VstSpeakerArrangement* arrangement = static_cast<VstSpeakerArrangement*>(ptr);
                *arrangement = VST2Loader::getOutputArrangement();
            }
            return 1;
            
        case audioMasterCanDo:
            if (ptr) {
                const char* feature = static_cast<const char*>(ptr);
                if (strcmp(feature, "sendVstEvents") == 0) return 1;
                if (strcmp(feature, "sendVstMidiEvent") == 0) return 1;
                if (strcmp(feature, "receiveVstEvents") == 0) return 1;
                if (strcmp(feature, "receiveVstMidiEvent") == 0) return 1;
                if (strcmp(feature, "reportConnectionChanges") == 0) return 1;
                if (strcmp(feature, "acceptIOChanges") == 0) return 1;
                if (strcmp(feature, "sizeWindow") == 0) return 1;
                if (strcmp(feature, "sendVstSpeakerArrangement") == 0) return 1;
                if (strcmp(feature, "receiveVstSpeakerProperties") == 0) return 1;
                if (strcmp(feature, "supplyIdle") == 0) return 1;
            }
            return 0;
            
        case audioMasterCurrentId:
            return 0;
            
        case audioMasterIdle:
            return 0;
            
        case audioMasterGetCurrentProcessLevel:
            return 0;
            
        case audioMasterGetAutomationState:
            return 0;
            
        case audioMasterGetLanguage:
            return 0;
            
        case audioMasterGetProductString:
            if (ptr) {
                strcpy((char*)ptr, "Studio One");
            }
            return 1;
            
        case audioMasterGetVendorString:
            if (ptr) {
                strcpy((char*)ptr, "PreSonus");
            }
            return 1;
            
        case audioMasterGetVendorVersion:
            return 6500;  // Studio One version
            
        default:
            return 0;
    }
}

bool VST2Loader::loadPlugin(const juce::String& path) {
    unloadPlugin();
    
    // Load the VST2 DLL
    pluginModule = LoadLibraryA(path.toRawUTF8());
    if (!pluginModule) {
        return false;
    }
    
    // Get the main entry point
    typedef AEffect* (*MainEntryPoint)(AudioMasterCallback);
    MainEntryPoint mainEntry;
    
    mainEntry = (MainEntryPoint)GetProcAddress(pluginModule, "VSTPluginMain");
    
    if (!mainEntry) {
        mainEntry = (MainEntryPoint)GetProcAddress(pluginModule, "main");
    }
    
    if (!mainEntry) {
        FreeLibrary(pluginModule);
        pluginModule = nullptr;
        return false;
    }
    
    // Create the effect with our intercepting callback
    try {
        effect = mainEntry(hostCallback);
    }
    catch (...) {
        effect = nullptr;
    }
    
    if (!effect) {
        FreeLibrary(pluginModule);
        pluginModule = nullptr;
        return false;
    }
    
    // Validate magic number (but continue even if it fails for compatibility)
    if (effect->magic != 0x56737450) {  // 'VstP' as hex
        // Some plugins may have different magic numbers, continue anyway
    }
    
    // Try opening effect
    try {
        effect->dispatcher(effect, effOpen, 0, 0, nullptr, 0.0f);
    }
    catch (...) {
        // Continue even if effOpen fails
    }
    
    // Set speaker arrangement to 5.1
    VstSpeakerArrangement inputs, outputs;
    setup51Arrangement(inputs);
    setup51Arrangement(outputs);
    
    VstIntPtr result = effect->dispatcher(effect, effSetSpeakerArrangement, 0, 
                                         (VstIntPtr)&inputs, &outputs, 0.0f);
    
    if (result == 1) {
        wantsSurround = true;
        // Update effect's I/O counts to reflect 5.1
        effect->numInputs = 6;
        effect->numOutputs = 6;
    } else {
        // Try alternative approach - force the I/O configuration
        effect->numInputs = 6;
        effect->numOutputs = 6;
        
        // Try calling the speaker arrangement again after forcing I/O
        result = effect->dispatcher(effect, effSetSpeakerArrangement, 0, 
                                   (VstIntPtr)&inputs, &outputs, 0.0f);
        
        // Force surround mode regardless of result since our wrapper handles 6 channels
        wantsSurround = true;
    }
    
    // Additional enforcement - call canDo checks to trigger more callbacks
    effect->dispatcher(effect, effCanDo, 0, 0, (void*)"sendVstSpeakerArrangement", 0.0f);
    effect->dispatcher(effect, effCanDo, 0, 0, (void*)"receiveVstSpeakerProperties", 0.0f);
    
    return true;
}

void VST2Loader::unloadPlugin() {
    if (effect) {
        closeEditor();
        suspend();
        effect->dispatcher(effect, effClose, 0, 0, nullptr, 0.0f);
        effect = nullptr;
    }
    
    if (pluginModule) {
        FreeLibrary(pluginModule);
        pluginModule = nullptr;
    }
}

void VST2Loader::processReplacing(float** inputs, float** outputs, int sampleFrames) {
    if (effect && effect->processReplacing) {
        effect->processReplacing(effect, inputs, outputs, sampleFrames);
    }
}

void VST2Loader::setParameter(int index, float value) {
    if (effect && effect->setParameter) {
        effect->setParameter(effect, index, value);
    }
}

float VST2Loader::getParameter(int index) {
    if (effect && effect->getParameter) {
        return effect->getParameter(effect, index);
    }
    return 0.0f;
}

int VST2Loader::getNumParameters() const {
    return effect ? effect->numParams : 0;
}

juce::String VST2Loader::getParameterName(int index) {
    if (!effect) return "";
    
    char name[256] = {0};
    effect->dispatcher(effect, effGetParamName, index, 0, name, 0.0f);
    return juce::String(name);
}

juce::String VST2Loader::getParameterText(int index) {
    if (!effect) return "";
    
    char text[256] = {0};
    effect->dispatcher(effect, effGetParamDisplay, index, 0, text, 0.0f);
    return juce::String(text);
}

bool VST2Loader::hasEditor() const {
    return effect && (effect->flags & effFlagsHasEditor);
}

void* VST2Loader::openEditor(void* parentWindow) {
    if (!hasEditor()) {
        return nullptr;
    }
    
    if (editorWindow) {
        closeEditor();
    }
    
    try {
        VstIntPtr result = effect->dispatcher(effect, effEditOpen, 0, 0, parentWindow, 0.0f);
        
        if (result != 0) {
            editorWindow = parentWindow;
            return editorWindow;
        }
        return nullptr;
    }
    catch (...) {
        return nullptr;
    }
}

void VST2Loader::closeEditor() {
    if (editorWindow && effect) {
        try {
            effect->dispatcher(effect, effEditClose, 0, 0, nullptr, 0.0f);
        }
        catch (...) {
            // Silent error handling
        }
        
        editorWindow = nullptr;
    }
}

void VST2Loader::getEditorSize(int& width, int& height) {
    if (!effect) {
        width = height = 0;
        return;
    }
    
    struct ERect { short top, left, bottom, right; };
    ERect* rect = nullptr;
    
    effect->dispatcher(effect, effEditGetRect, 0, 0, &rect, 0.0f);
    
    if (rect) {
        width = rect->right - rect->left;
        height = rect->bottom - rect->top;
    } else {
        width = height = 400;
    }
}

int VST2Loader::getNumPrograms() const {
    return effect ? effect->numPrograms : 0;
}

int VST2Loader::getCurrentProgram() const {
    if (!effect) return 0;
    return (int)effect->dispatcher(effect, effGetProgram, 0, 0, nullptr, 0.0f);
}

void VST2Loader::setCurrentProgram(int index) {
    if (effect) {
        effect->dispatcher(effect, effSetProgram, 0, index, nullptr, 0.0f);
    }
}

juce::String VST2Loader::getProgramName(int index) {
    if (!effect) return "";
    
    char name[256] = {0};
    effect->dispatcher(effect, effGetProgramName, index, 0, name, 0.0f);
    return juce::String(name);
}

void VST2Loader::setSpeakerArrangement(VstSpeakerArrangement* inputs, VstSpeakerArrangement* outputs) {
    if (effect && inputs && outputs) {
        effect->dispatcher(effect, effSetSpeakerArrangement, 0, (VstIntPtr)inputs, outputs, 0.0f);
        inputArrangement = *inputs;
        outputArrangement = *outputs;
    }
}

bool VST2Loader::getSpeakerArrangement(VstSpeakerArrangement** inputs, VstSpeakerArrangement** outputs) {
    if (!effect) return false;
    
    *inputs = &inputArrangement;
    *outputs = &outputArrangement;
    
    VstIntPtr result = effect->dispatcher(effect, effGetSpeakerArrangement, 0, 
                                         (VstIntPtr)*inputs, *outputs, 0.0f);
    return result == 1;
}

void VST2Loader::setSampleRate(double sampleRate) {
    if (effect) {
        effect->dispatcher(effect, effSetSampleRate, 0, 0, nullptr, (float)sampleRate);
    }
}

void VST2Loader::setBlockSize(int blockSize) {
    if (effect) {
        effect->dispatcher(effect, effSetBlockSize, 0, blockSize, nullptr, 0.0f);
    }
}

void VST2Loader::suspend() {
    if (effect) {
        effect->dispatcher(effect, effMainsChanged, 0, 0, nullptr, 0.0f);
    }
}

void VST2Loader::resume() {
    if (effect) {
        effect->dispatcher(effect, effMainsChanged, 0, 1, nullptr, 0.0f);
    }
}

bool VST2Loader::getChunk(void** data, bool isPreset) {
    if (!effect) return false;
    
    VstIntPtr byteSize = effect->dispatcher(effect, effGetChunk, isPreset ? 1 : 0, 0, data, 0.0f);
    return byteSize > 0;
}

bool VST2Loader::setChunk(void* data, int byteSize, bool isPreset) {
    if (!effect) return false;
    
    VstIntPtr result = effect->dispatcher(effect, effSetChunk, isPreset ? 1 : 0, byteSize, data, 0.0f);
    return result == 1;
}