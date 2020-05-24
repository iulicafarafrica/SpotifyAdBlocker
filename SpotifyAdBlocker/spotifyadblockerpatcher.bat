@echo off
if "%1"=="patch" (
	icacls "%localappdata%\Spotify\Update" /reset /T > NUL 2>&1
	del /s /q "%localappdata%\Spotify\Update" > NUL 2>&1
	mkdir "%localappdata%\Spotify\Update" > NUL 2>&1
	icacls "%localappdata%\Spotify\Update" /deny "%username%":W > NUL 2>&1
) else (
	goto ready
)
exit /b
:ready
del "spotify_installer-1.1.4.197.g92d52c4f-13.exe" > NUL 2>&1
taskkill /f /im Spotify.exe > NUL 2>&1
powershell Get-AppxPackage -Name "SpotifyAB.SpotifyMusic" | findstr "PackageFullName" > NUL
if %errorlevel% EQU 0 (
	echo.
	echo The Microsoft Store version of Spotify has been detected which is not supported.
	echo Please uninstall it first, and then run this script again.
	echo.
	echo To uninstall, search for Spotify in the start menu and right-click on the result and click Uninstall.
	echo.
	echo The full ^(offline^) setup for the desktop version can be found here:
	echo https://download.scdn.co/SpotifyFullSetup.exe
	echo.
	pause
	exit 
)
echo Patching started...
echo Downloading Spotify non-hotfixable, please wait (may take a minute or so)...
powershell.exe -ExecutionPolicy Bypass -Command (new-object System.Net.WebClient).DownloadFile('http://upgrade.spotify.com/upgrade/client/win32-x86/spotify_installer-1.1.4.197.g92d52c4f-13.exe','spotify_installer-1.1.4.197.g92d52c4f-13.exe')
echo Disabling auto-update...
if not exist "%appdata%\Spotify\" mkdir "%appdata%\Spotify" > NUL 2>&1
powershell "saps -wait -filepath '%0' -verb runas -argumentlist 'patch'" >nul 2>&1
echo Finalizing the patching process...
spotify_installer-1.1.4.197.g92d52c4f-13.exe /extract "%appdata%\Spotify"
del "%appdata%\Spotify\SpotifyMigrator.exe" > NUL
del "%appdata%\Spotify\SpotifyStartupTask.exe" > NUL
powershell -ExecutionPolicy Bypass -Command "$ws = New-Object -ComObject WScript.Shell; $s = $ws.CreateShortcut('%userprofile%\Desktop\Spotify.lnk'); $S.TargetPath = '%appdata%\Spotify\Spotify.exe'; $S.Save()" > NUL 2>&1
del "spotify_installer-1.1.4.197.g92d52c4f-13.exe"
exit