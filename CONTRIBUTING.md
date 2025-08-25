# Contributing to Altiverb Surround Wrapper

Thank you for your interest in contributing to this project! 🎵

## 🐛 Bug Reports

When filing bug reports, please include:

- **Operating System** (Windows version)
- **DAW Name and Version** (Studio One 6.x, etc.)
- **Altiverb Version** (7.x)
- **Steps to reproduce** the issue
- **Expected vs actual behavior**
- **Console logs** if available

## 💡 Feature Requests

Before submitting feature requests:

1. Check existing issues to avoid duplicates
2. Describe the use case and problem being solved
3. Consider compatibility with various DAWs
4. Provide mockups or examples if applicable

## 🔧 Code Contributions

### Development Setup

1. **Fork** the repository
2. **Install JUCE 8.0.8** and Visual Studio 2022
3. **Clone** your fork locally
4. **Create** a feature branch: `git checkout -b feature/amazing-feature`

### Code Standards

- **Follow JUCE conventions** for C++ code
- **Test thoroughly** with multiple DAWs if possible
- **Keep commits atomic** and well-described
- **Update documentation** for new features

### Build and Test

```bash
# Generate project files
Projucer.exe --resave AltiverbSurroundWrapper.jucer

# Build debug version
MSBuild "Builds\VisualStudio2022\AltiverbSurroundWrapper.sln" /p:Configuration=Debug /p:Platform=x64

# Test in your preferred DAW
# Install to: C:\Program Files\Common Files\VST3\
```

### Pull Requests

1. **Update README.md** if needed
2. **Test with multiple instances** to ensure stability  
3. **Verify no crashes** during plugin lifecycle
4. **Submit PR** with clear description

## 🎯 Priority Areas

Currently looking for contributions in:

- **macOS Support** - Porting to macOS with Audio Units
- **Additional DAW Testing** - Logic Pro, Reaper, Ableton Live compatibility
- **Alternative VST2 Plugins** - Support for other surround reverb plugins
- **GUI Improvements** - Enhanced user interface

## 📝 Documentation

Help improve documentation by:

- **Adding troubleshooting steps** for specific DAWs
- **Creating video tutorials** for common workflows  
- **Translating README** to other languages
- **Updating build instructions** for different setups

## 🤝 Community

- Be respectful and constructive
- Help other users in Issues
- Share your experience with different setups
- Report compatibility findings

## ❓ Questions

For general questions, please:

1. Check existing **Issues** and **Discussions**
2. Search the **README** and documentation
3. Ask in **GitHub Discussions** for community help
4. File an **Issue** only for bugs or specific feature requests

Thank you for contributing to the audio production community! 🎶