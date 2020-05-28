using System;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Windows.Forms;

namespace SpotifyAdBlocker
{ 
    // Not static as it's safer to use instance methods in UI backend code
    internal class Patching
    {
        private const string PatcherBatPath = @".\spotifyadblockerpatcher.bat";
        private const string NetUtilsDllPath = @".\netutils.dll";
        private const string ChromeElfDllPath = @".\chrome_elf.dll";
        private const string ChromeElfDllConfigPath = @".\config.ini";
        private readonly string _spotifyInstallationPath = $@"C:\Users\{Environment.UserName}\AppData\Roaming\Spotify";
        private readonly string _doubleNewline = $@"{Environment.NewLine}{Environment.NewLine}";

        private readonly DataReceivedEventHandler _emptyDataReceiverHandler = (sender, e) =>
        {
            new ManualResetEvent(false).WaitOne(100);
        };

        internal static void FatalErrorMsg(string msg) => MessageBox.Show(msg, "SpotifyAdBlocker: error/warning", MessageBoxButtons.OK, MessageBoxIcon.Error);

        internal static void NormalMsg(string msg, bool isSuccessMsg = false)
        {
            if (isSuccessMsg)
                MessageBox.Show(new Form {TopMost=true}, msg, "SpotifyAdBlocker", MessageBoxButtons.OK, MessageBoxIcon.Information);
            else 
                MessageBox.Show(msg, "SpotifyAdBlocker", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        // Error codes used in Environment.Exit() calls are found here: https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes--0-499-
        public void MainPatch()
        { 
            if (!File.Exists(PatcherBatPath))
            {
                FatalErrorMsg($@"FATAL ERROR: spotifyadblockerpatcher.bat file not found in program directory{_doubleNewline}Make sure this program was installed from the proper source (HxxxB's GitHub), and you don't remove any files in the distribution");
                Environment.Exit(0x2); // ERROR_FILE_NOT_FOUND
            } else if (!Directory.Exists(_spotifyInstallationPath))
            {
                FatalErrorMsg($@"FATAL ERROR: Spotify installation not found{_doubleNewline}Make sure you have the desktop version found on the Spotify website, not the Microsoft Store version, as the Microsoft Store version is not supported");
                Environment.Exit(0x3); // ERROR_PATH_NOT_FOUND
            } else if (!File.Exists(ChromeElfDllPath))
            {
                FatalErrorMsg($@"FATAL ERROR: chrome_elf.dll file not found in program directory{_doubleNewline}Make sure this program was installed from the proper source (HxxxB's GitHub), and you don't remove any files in the distribution.");
                Environment.Exit(0x2); // ERROR_FILE_NOT_FOUND
            } else if (!File.Exists(ChromeElfDllConfigPath))
            {
                FatalErrorMsg($@"FATAL ERROR: chrome_elf.dll config file (config.ini) not found in program directory{_doubleNewline}Make sure this program was installed from the proper source (HxxxB's GitHub), and you don't remove any files in the distribution");
                Environment.Exit(0x2); // ERROR_FILE_NOT_FOUND
            } else if (!File.Exists(NetUtilsDllPath))
            {
                FatalErrorMsg($@"FATAL ERROR: netutils.dll file (config.ini) not found in program directory{_doubleNewline}Make sure this program was installed from the proper source (HxxxB's GitHub), and you don't remove any files in the distribution");
                Environment.Exit(0x2); // ERROR_FILE_NOT_FOUND
            }
            
            try
            {
                ExecuteCommand($@"start {PatcherBatPath}");

                // Delay for enough time for Spotify file handles on chrome_elf.dll to be released properly, to avoid IOException
                new ManualResetEvent(false).WaitOne(500);

                File.Copy(NetUtilsDllPath, Path.Combine(_spotifyInstallationPath, NetUtilsDllPath), true);
                File.Copy(ChromeElfDllPath, Path.Combine(_spotifyInstallationPath, ChromeElfDllPath), true);
                File.Copy(ChromeElfDllConfigPath, Path.Combine(_spotifyInstallationPath, ChromeElfDllConfigPath), true);
            }
            catch (Exception ex)
            {
                FatalErrorMsg($@"FATAL ERROR: Patching failed.{_doubleNewline}{ex}{_doubleNewline} Please tell the developer (HxxxB) if you get this error, and the type of error you got");
                Environment.Exit(0xBC); // ERROR_INVALID_STARTING_CODESEG
            }

            NormalMsg(@"PATCHING SUCCEEDED: Spotify ad-blocking patches finalized and properly applied. If you do get ads, please tell the developer (HxxxB)", true);
        }

        private void ExecuteCommand(string command)
        {
            var appProcessStartInfo = new ProcessStartInfo("cmd.exe", "/c " + command)
            {
                CreateNoWindow = true,
                UseShellExecute = false,
                RedirectStandardError = true,
                RedirectStandardOutput = true,
                WindowStyle = ProcessWindowStyle.Hidden
            };

            var appProcess = new Process
            {
                StartInfo = appProcessStartInfo,
                EnableRaisingEvents = true
            };

            appProcess.Start();

            appProcess.OutputDataReceived += _emptyDataReceiverHandler;
            appProcess.BeginOutputReadLine();

            appProcess.ErrorDataReceived += _emptyDataReceiverHandler;
            appProcess.BeginErrorReadLine();

            appProcess.WaitForExit();
            appProcess.Close();
        }
    }
}
