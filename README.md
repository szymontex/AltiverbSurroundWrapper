# Altiverb Surround Wrapper

A VST3 wrapper that enables **5.1 surround sound** processing with **Altiverb 7 XL** in Studio One and other modern DAWs.

## 🎯 The Problem This Solves

**Altiverb 7 XL** is one of the best convolution reverbs available, with stunning 5.1 surround impulse responses. However:

- **Altiverb is only available as VST2** (no VST3 version exists)
- **Studio One doesn't support surround processing in VST2 plugins** - it loads them in stereo only
- **Other modern DAWs have similar VST2 surround limitations**
- This means you **cannot use Altiverb's beautiful 5.1 surround reverb** in Studio One

## 💡 The Solution

This wrapper:
- **Loads your existing Altiverb 7 XL VST2** internally
- **Presents itself as a VST3 plugin** to your DAW
- **Enables full 5.1 surround processing** that Studio One recognizes
- **Gives you access to Altiverb's original interface** through the wrapper

**⚠️ Important: You must own a legal copy of Altiverb 7 XL - this wrapper only provides the VST3 interface to your existing Altiverb installation.**

## ✨ Features

- **🎵 5.1 Surround Processing** - Full 6-channel audio routing (L, R, C, LFE, Ls, Rs)
- **🎛️ Native GUI Integration** - Opens the original Altiverb interface in a popup window
- **⚙️ Configurable VST2 Path** - Browse and select your Altiverb installation location
- **🔄 Multiple Instance Support** - Stable operation with multiple plugin instances
- **💾 Registry Configuration** - Automatically remembers your VST2 path selection

## 📋 Requirements

- **Windows 10/11** (64-bit)
- **Altiverb 7 XL VST2** - You must own and have installed a legal copy
- **DAW with VST3 support** (Studio One, Cubase, Reaper, etc.)
- **JUCE Framework 8.0.8** (only needed for building from source)

**🔑 Legal Note**: This wrapper does not include Altiverb - you must purchase and install Altiverb 7 XL from Audio Ease separately.

## 🚀 Quick Start

1. **Make sure Altiverb 7 XL VST2 is installed** and working in other DAWs
2. **Download** the latest VST3 from [Releases](../../releases)
3. **Copy** `AltiverbSurroundWrapper.vst3` to your VST3 folder:
   ```
   C:\Program Files\Common Files\VST3\
   ```
4. **Launch Studio One** (or your preferred DAW) and scan for new plugins
5. **Create a 5.1 surround track** and load "Altiverb 7 Surround" as an effect
6. **If Altiverb doesn't load automatically**: Click "Browse VST2 Path..." to locate your `Altiverb 7.dll` file
7. **Click "Open Altiverb Editor"** to access the full Altiverb interface with all your impulse responses

## 🔧 Configuration

### First Time Setup
1. Insert the plugin on a 5.1 surround track
2. If Altiverb doesn't load automatically, click **"Browse VST2 Path..."**
3. Navigate to your Altiverb installation (typically):
   ```
   C:\Program Files\VSTPlugins\Altiverb 7\Altiverb 7.dll
   ```
4. The path is saved automatically for future use

### Common Installation Paths
- `C:\Program Files\VSTPlugins\Altiverb 7\Altiverb 7.dll`
- `C:\Program Files (x86)\VSTPlugins\Altiverb 7\Altiverb 7.dll`  
- `C:\Program Files\Audio Ease\Altiverb 7\Altiverb 7.dll`

## 🏗️ Building from Source

### Prerequisites
1. **JUCE Framework 8.0.8** - [Download here](https://juce.com/get-juce/)
2. **Visual Studio 2022** Community or Professional
3. **Windows SDK** (latest)

### Build Steps
```bash
# 1. Clone the repository
git clone https://github.com/yourusername/AltiverbSurroundWrapper.git
cd AltiverbSurroundWrapper

# 2. Update JUCE path in AltiverbSurroundWrapper.jucer if needed

# 3. Generate Visual Studio project with Projucer
"C:\Path\To\JUCE\Projucer.exe" --resave AltiverbSurroundWrapper.jucer

# 4. Build the project
MSBuild "Builds\VisualStudio2022\AltiverbSurroundWrapper.sln" /p:Configuration=Release /p:Platform=x64

# 5. VST3 output location
Builds\VisualStudio2022\x64\Release\VST3\AltiverbSurroundWrapper.vst3
```

## 🎚️ Technical Details

### Channel Mapping
The wrapper automatically maps 5.1 channels as follows:
- **Channel 0**: Left (L)
- **Channel 1**: Right (R)  
- **Channel 2**: Center (C)
- **Channel 3**: LFE (Low Frequency Effects)
- **Channel 4**: Left Surround (Ls)
- **Channel 5**: Right Surround (Rs)

### VST2 Integration
- Uses custom VST2 hosting engine with audioMaster callbacks
- Implements proper 5.1 speaker arrangement negotiation
- Handles VST2 editor lifecycle management
- Registry-based configuration storage

## 🐛 Troubleshooting

### Plugin Not Loading
- Verify Altiverb VST2 is installed and working in other hosts
- Check the VST2 path in plugin settings
- Ensure you're using the 64-bit version of Altiverb

### No Sound Processing
- Confirm your DAW track is set to 5.1 surround mode
- Check input/output routing in your DAW
- Verify Altiverb has reverb settings loaded

### Editor Window Issues
- Close and reopen the Altiverb editor window
- Try scanning for plugins again in your DAW
- Restart your DAW if editor becomes unresponsive

## 📝 Version History

### v1.0.0 - Initial Release
- ✅ 5.1 surround processing
- ✅ VST2 to VST3 wrapping  
- ✅ GUI with Altiverb editor integration
- ✅ Configurable VST2 path selection
- ✅ Multiple instance stability
- ✅ Windows Registry configuration

## 📄 License

This project is released under the **MIT License** - see [LICENSE](LICENSE) for details.

## ❓ Why This Wrapper is Needed

**Studio One's VST2 Limitation**: Studio One (and other modern DAWs) only load VST2 plugins in stereo mode, regardless of the plugin's actual surround capabilities. This prevents access to Altiverb's incredible 5.1 surround impulse responses.

**No Official VST3**: Audio Ease has not released a VST3 version of Altiverb, leaving surround users without options in modern DAWs.

**This Wrapper's Approach**: Acts as a "translator" between your existing Altiverb VST2 and your DAW's VST3 interface, enabling the surround processing that was always possible but blocked by format limitations.

## 🔐 Legal & Ethical Notice

- **This wrapper does NOT contain any Audio Ease code or content**
- **You must own a legal license for Altiverb 7 XL** 
- **This is similar to using a VST host or ReWire** - it just provides a different interface to your existing plugin
- **All audio processing is performed by your licensed Altiverb installation**
- **Respects Audio Ease's intellectual property** while solving a technical limitation

## 🙏 Acknowledgments

- **Audio Ease** for creating Altiverb 7 XL - still the gold standard for convolution reverb
- **JUCE Framework** for excellent audio plugin development tools
- **VST SDK** by Steinberg for plugin standards

## 🤝 Contributing

Contributions welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## ⭐ Support

If this wrapper helped you achieve 5.1 surround reverb in your DAW, please give it a star! ⭐

---

**Made with ❤️ for the audio production community**