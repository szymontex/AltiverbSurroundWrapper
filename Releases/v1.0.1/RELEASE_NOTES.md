# Altiverb Surround Wrapper v1.0.1 - HOTFIX

**Critical Fix** - Removes problematic hardcoded paths and improves VST2 detection! 🔧

## 🚨 Critical Fixes

- **Removed hardcoded piracy-related path** - Clean, professional code only
- **Enhanced VST2 path detection** - Now searches 17+ common Altiverb installation locations
- **Improved compatibility** - Better support for various Altiverb installations

## 🔍 Enhanced VST2 Path Detection

Now automatically searches these locations:
- `C:\Program Files\VSTPlugins\Altiverb 7\`
- `C:\Program Files\Audio Ease\Altiverb 7\`
- `C:\Program Files\Audio Ease\Altiverb 7 XL\`
- `C:\Program Files\Steinberg\VSTPlugins\Altiverb 7\`
- Plus 13 more common installation paths!

## 🚀 Installation

1. Download `AltiverbSurroundWrapper.vst3` from this release
2. Copy to your VST3 folder: `C:\Program Files\Common Files\VST3\`
3. Scan for plugins in your DAW
4. Load "Altiverb 7 Surround" on a 5.1 surround track

## 💡 If Altiverb Doesn't Auto-Load

1. Click "Browse VST2 Path..." in the plugin GUI
2. Navigate to your Altiverb installation
3. Select `Altiverb 7.dll`
4. Path is saved automatically for future use

## 🔧 Technical Info

- **Build**: Debug configuration (fully functional)
- **Channels**: 6-channel 5.1 surround (L, R, C, LFE, Ls, Rs)
- **Platform**: Windows 64-bit
- **Size**: ~18MB VST3 bundle

## ✅ What's Working

- 5.1 surround processing in Studio One
- Multiple plugin instances
- GUI with configurable VST2 path
- Registry-based path storage
- Enhanced auto-detection

---

**This release ensures clean, professional code suitable for public distribution! 🎵**