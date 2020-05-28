### Block Spotify ads for free via dll injector/replacer and Spotify client hotfix version downgrade patches
### Download the latest version found [right here](https://github.com/HxxxB/SpotifyAdBlocker/releases)
###### Windows only (maybe MacOS/Linux support later on???)

Compiling details:
----

IDE used: [Visual Studio Community 2019 Preview](https://visualstudio.microsoft.com/vs/preview/) (2017 can probably be used to also compile, not completely sure)
[ReSharper](https://www.jetbrains.com/resharper/) used for code analysis


SpotifyAdBlocker (C#, .NET)
----

Release | AnyCPU - embedded debug info 

.NET runtime/sdk used: .NET Framework 4.8

----

NetUtilsSpotifyInjector and SpotifyChromeElfReplacer (C++)
----

Release | Win32 - no debug info

Windows SDK version (C++): 10.0.17763.0 (later versions won't compile due to bit alignment max being screwed up, in `winnt.h`)

Platform Toolset: v142 (vs2019)

MSVC Toolset: 14.27.28826

C++ language standard: C++17

Multi-byte character set

Struct bit alignment set to 8-strict

Multi-threaded runtime library

C++/SEH exceptions for normal C files - `enabled`

Stack-buffer over-run runtime security check - `disabled` for performance

Floating point model - `precise`

Function-level linking and parallel code generation `enabled`

Compiler optimizations  - `enabled` 
- SDL checks - `disabled`
- Maximum optimization (favor `speed`)
- Inline function expansion set to `Any Suitable`
- Intrinsic functions - `enabled`
- Favor fast code
- Frame pointer creation on call stack `suppressed` to make function calling faster 
- Fiber-safe optimizations - `enabled`
- Whole Program optimization - `enabled`

Incremental Link-Time Code Generation (LTCG) optimization used to make inline assembler and compiled object files link via managed pre-JIT runtime-only-invoked methods

