#pragma once
#include <JuceHeader.h>
#include <Windows.h>
#include <Psapi.h>
#include <memory>

// VST2 SDK definitions (minimal subset needed)
typedef int32_t VstInt32;
typedef intptr_t VstIntPtr;

#ifndef VSTCALLBACK
  #ifdef _WIN32
    #define VSTCALLBACK __cdecl
  #else
    #define VSTCALLBACK
  #endif
#endif

enum VstAEffectFlags {
    effFlagsCanReplacing = 1 << 4,
    effFlagsHasEditor = 1 << 0,
    effFlagsIsSynth = 1 << 8
};

enum VstOpcodes {
    effOpen = 0,
    effClose = 1,
    effSetProgram = 2,
    effGetProgram = 3,
    effSetProgramName = 4,
    effGetProgramName = 5,
    effGetParamLabel = 6,
    effGetParamDisplay = 7,
    effGetParamName = 8,
    effSetSampleRate = 10,
    effSetBlockSize = 11,
    effMainsChanged = 12,
    effEditGetRect = 13,
    effEditOpen = 14,
    effEditClose = 15,
    effEditIdle = 19,
    effGetChunk = 23,
    effSetChunk = 24,
    effProcessReplacing = 26,
    effCanBeAutomated = 26,
    effGetTailSize = 52,
    effGetParameterProperties = 56,
    effGetVstVersion = 58,
    effEditKeyDown = 59,
    effEditKeyUp = 60,
    effSetEditKnobMode = 61,
    effGetMidiProgramName = 62,
    effGetCurrentMidiProgram = 63,
    effGetMidiProgramCategory = 64,
    effHasMidiProgramsChanged = 65,
    effGetMidiKeyName = 66,
    effBeginSetProgram = 67,
    effEndSetProgram = 68,
    effGetSpeakerArrangement = 69,
    effSetSpeakerArrangement = 42,
    effShellGetNextPlugin = 70,
    effStartProcess = 71,
    effStopProcess = 72,
    effSetTotalSampleToProcess = 73,
    effSetPanLaw = 74,
    effBeginLoadBank = 75,
    effBeginLoadProgram = 76,
    effSetProcessPrecision = 77,
    effGetNumMidiInputChannels = 78,
    effGetNumMidiOutputChannels = 79,
    effCanDo = 51
};

enum VstHostOpcodes {
    audioMasterAutomate = 0,
    audioMasterVersion = 1,
    audioMasterCurrentId = 2,
    audioMasterIdle = 3,
    audioMasterPinConnected = 4,
    audioMasterWantMidi = 6,
    audioMasterGetTime = 7,
    audioMasterProcessEvents = 8,
    audioMasterSetTime = 9,
    audioMasterTempoAt = 10,
    audioMasterGetNumAutomatableParameters = 11,
    audioMasterGetParameterQuantization = 12,
    audioMasterIOChanged = 13,
    audioMasterNeedIdle = 14,
    audioMasterSizeWindow = 15,
    audioMasterGetSampleRate = 16,
    audioMasterGetBlockSize = 17,
    audioMasterGetInputLatency = 18,
    audioMasterGetOutputLatency = 19,
    audioMasterGetPreviousPlug = 20,
    audioMasterGetNextPlug = 21,
    audioMasterWillReplaceOrAccumulate = 22,
    audioMasterGetCurrentProcessLevel = 23,
    audioMasterGetAutomationState = 24,
    audioMasterOfflineStart = 25,
    audioMasterOfflineRead = 26,
    audioMasterOfflineWrite = 27,
    audioMasterOfflineGetCurrentPass = 28,
    audioMasterOfflineGetCurrentMetaPass = 29,
    audioMasterSetOutputSampleRate = 30,
    audioMasterGetOutputSpeakerArrangement = 31,
    audioMasterGetVendorString = 32,
    audioMasterGetProductString = 33,
    audioMasterGetVendorVersion = 34,
    audioMasterVendorSpecific = 35,
    audioMasterSetIcon = 36,
    audioMasterCanDo = 37,
    audioMasterGetLanguage = 38,
    audioMasterOpenWindow = 39,
    audioMasterCloseWindow = 40,
    audioMasterGetDirectory = 41,
    audioMasterUpdateDisplay = 42,
    audioMasterBeginEdit = 43,
    audioMasterEndEdit = 44,
    audioMasterOpenFileSelector = 45,
    audioMasterCloseFileSelector = 46,
    audioMasterEditFile = 47,
    audioMasterGetChunkFile = 48,
    audioMasterGetInputSpeakerArrangement = 49,
    
    // VST 2.1 extensions
    audioMasterGetSpeakerArrangement = 69,
    audioMasterSetSpeakerArrangement = 70,
    audioMasterSetBlockSizeAndSampleRate = 71,
    audioMasterSetBypass = 72,
    audioMasterGetEffectName = 73,
    audioMasterGetErrorText = 74,
    audioMasterGetVendorName = 75,
    audioMasterGetProductName = 76,
    audioMasterGetMasterVersion = 77,
    audioMasterSetRealTime = 78,
    audioMasterGetOutputLatencyPtr = 79,
    audioMasterGetInputLatencyPtr = 80
};

struct AEffect;

typedef VstIntPtr (*AudioMasterCallback)(AEffect* effect, VstInt32 opcode, VstInt32 index, 
                                          VstIntPtr value, void* ptr, float opt);

typedef VstIntPtr (*AEffectDispatcherProc)(AEffect* effect, VstInt32 opcode, VstInt32 index, 
                                            VstIntPtr value, void* ptr, float opt);

typedef void (*AEffectProcessProc)(AEffect* effect, float** inputs, float** outputs, VstInt32 sampleFrames);

typedef void (*AEffectSetParameterProc)(AEffect* effect, VstInt32 index, float parameter);
typedef float (*AEffectGetParameterProc)(AEffect* effect, VstInt32 index);

struct AEffect {
    VstInt32 magic;
    AEffectDispatcherProc dispatcher;
    AEffectProcessProc process;
    AEffectSetParameterProc setParameter;
    AEffectGetParameterProc getParameter;
    VstInt32 numPrograms;
    VstInt32 numParams;
    VstInt32 numInputs;
    VstInt32 numOutputs;
    VstInt32 flags;
    VstIntPtr resvd1;
    VstIntPtr resvd2;
    VstInt32 initialDelay;
    VstInt32 realQualities;
    VstInt32 offQualities;
    float ioRatio;
    void* object;
    void* user;
    VstInt32 uniqueID;
    VstInt32 version;
    AEffectProcessProc processReplacing;
    AEffectProcessProc processDoubleReplacing;
    char future[56];
};

// VST2 Speaker Arrangement structures
struct VstSpeakerProperties {
    float azimuth;
    float elevation;
    float radius;
    float reserved;
    char name[64];
    VstInt32 type;
    char future[28];
};

struct VstSpeakerArrangement {
    VstInt32 type;
    VstInt32 numChannels;
    VstSpeakerProperties speakers[8];
};

// Speaker arrangement types
enum VstSpeakerArrangementType {
    kSpeakerArrStereo = 0,
    kSpeakerArr51 = 5
};

class VST2Loader {
public:
    VST2Loader();
    ~VST2Loader();
    
    bool loadPlugin(const juce::String& path);
    void unloadPlugin();
    
    bool isLoaded() const { return effect != nullptr; }
    AEffect* getEffect() { return effect; }
    
    // Process audio
    void processReplacing(float** inputs, float** outputs, int sampleFrames);
    
    // Parameter handling
    void setParameter(int index, float value);
    float getParameter(int index);
    int getNumParameters() const;
    juce::String getParameterName(int index);
    juce::String getParameterText(int index);
    
    // Editor handling
    bool hasEditor() const;
    void* openEditor(void* parentWindow);
    void closeEditor();
    void getEditorSize(int& width, int& height);
    
    // Program/preset handling
    int getNumPrograms() const;
    int getCurrentProgram() const;
    void setCurrentProgram(int index);
    juce::String getProgramName(int index);
    
    // Speaker arrangement
    void setSpeakerArrangement(VstSpeakerArrangement* inputs, VstSpeakerArrangement* outputs);
    bool getSpeakerArrangement(VstSpeakerArrangement** inputs, VstSpeakerArrangement** outputs);
    
    // Static access to 5.1 arrangement for callback
    static const VstSpeakerArrangement& getInputArrangement() { return inputArrangement; }
    static const VstSpeakerArrangement& getOutputArrangement() { return outputArrangement; }
    
    // Initialization
    void setSampleRate(double sampleRate);
    void setBlockSize(int blockSize);
    void suspend();
    void resume();
    
    // State management
    bool getChunk(void** data, bool isPreset);
    bool setChunk(void* data, int byteSize, bool isPreset);
    
    static VstIntPtr VSTCALLBACK hostCallback(AEffect* effect, VstInt32 opcode, VstInt32 index, 
                                              VstIntPtr value, void* ptr, float opt);
    
private:
    HMODULE pluginModule = nullptr;
    AEffect* effect = nullptr;
    void* editorWindow = nullptr;
    
    // Store current speaker arrangements  
    static VstSpeakerArrangement inputArrangement;
    static VstSpeakerArrangement outputArrangement;
    
    // Store whether plugin wants surround
    bool wantsSurround = false;
    
    // Original host callback (if we need to chain)
    static AudioMasterCallback originalHostCallback;
    
    // Setup 5.1 speaker arrangement
    void setup51Arrangement(VstSpeakerArrangement& arrangement);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VST2Loader)
};