#### Block Spotify ads for free via hosts file/non-hotfixable client downgrade/netutils.dll patching. May also block other types of ads. 
#### Requires admin perms to run, and 7z to unzip the release archive. 7zip download can be found [here](https://www.7-zip.org/)
#### Download the latest version found [right here](https://github.com/HxxxB/SpotifyAdBlocker/releases)
##### Windows only

Compiling details:
----
SpotifyAdBlocker (C#, .NET)

Release | AnyCPU - embedded debug info 

.NET runtime/sdk used: .NET Framework 4.8

----

NetUtilsSpotifyInjector and SpotifyChromeElfReplacer

Release | AnyCPU/x32 - no debug info

Windows SDK (C++): 10.0.17763.0 (later versions won't compile due to bit alignment max being screwed up, in `winnt.h`)

Bit alignment set to 8

Compiler optimizations added 

Linked with the following command: `link /DLL /out:netutils.dll /LTCG *.obj MSVCRTD.lib User32.lib Gdi32.lib ole32.lib Comdlg32.lib OleAut32.lib`

Generates error `LINK : warning LNK4098: defaultlib 'LIBCMT' conflicts with use of other libs; use /NODEFAULTLIB:library`, can be safely ignored 



