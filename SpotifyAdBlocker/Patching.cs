using System;
using System.Diagnostics;
using System.IO;
using System.Security.Principal;
using System.Windows.Forms;

namespace SpotifyAdBlocker
{ 
    internal class Patching
    {
        private const string PathToPatch = @"C:\Windows\System32\drivers\etc";
        private const string HostsfilePath = @".\hosts";
        private const string PatcherBatPath = @".\spotifyadblockerpatcher.bat";
        private const string NetUtilsDllPath = @".\netutils.dll";
        private const string ChromeElfDllPath = @".\chrome_elf.dll";
        private const string ChromeElfDllConfigPath = @".\config.ini";
        private readonly string _spotifyInstallationPath = $@"C:\Users\{Environment.UserName}\AppData\Roaming\Spotify";
        private readonly string _doubleNewline = $@"{Environment.NewLine}{Environment.NewLine}";

        internal static void FatalErrorMsg(string msg) => MessageBox.Show(msg, "SpotifyAdBlocker: FatalError", MessageBoxButtons.OK, MessageBoxIcon.Error);

        internal static void NormalMsg(string msg, bool isSuccessMsg = false)
        {
            if (isSuccessMsg)
                MessageBox.Show(new Form {TopMost=true}, msg, "SpotifyAdBlocker", MessageBoxButtons.OK, MessageBoxIcon.Information);
            else 
                MessageBox.Show(msg, "SpotifyAdBlocker", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        // Error codes used in Environment.Exit() calls are found here: https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes--0-499-
        public void Patch()
        {
            // Check if program is running as admin, which is required due to hosts file accessibility 
            if (!Program.IsAdmin)
            {
                var result = MessageBox.Show($@"FATAL ERROR: This program has not been ran as an administrator. This program requires admin permissions, as some of the patching is done in System32\drivers\etc, a system directory.{Environment.NewLine}{Environment.NewLine}Do you want to restart this program as an administrator?",
                    "SpotifyAdBlocker: FatalError",
                    MessageBoxButtons.YesNo,
                    MessageBoxIcon.Error);
                if (result == DialogResult.Yes) 
                    Program.RestartProgramAsAdmin();
                else 
                    Environment.Exit(0x13); // ERROR_WRITE_PROTECT
            }

            if (!Directory.Exists(PathToPatch))
            {
                FatalErrorMsg(@"FATAL ERROR: drivers\etc system path doesn't seem to exist. Is this a valid Windows 10 installation?");
                Environment.Exit(0x3); // ERROR_PATH_NOT_FOUND
            } else if (!File.Exists(HostsfilePath))
            {
                FatalErrorMsg(@"FATAL ERROR: hosts file not found in program directory. Make sure this program was installed from the proper source (HxxxB's GitHub), and you don't remove any files in the distribution.");
                Environment.Exit(0x2); // ERROR_FILE_NOT_FOUND
            } else if (!File.Exists(PatcherBatPath))
            {
                FatalErrorMsg(@"FATAL ERROR: spotifyadblockerpatcher.bat file not found in program directory. Make sure this program was installed from the proper source (HxxxB's GitHub), and you don't remove any files in the distribution.");
                Environment.Exit(0x2); // ERROR_FILE_NOT_FOUND
            } else if (!Directory.Exists(_spotifyInstallationPath))
            {
                FatalErrorMsg(@"FATAL ERROR: Spotify installation not found. Make sure you have the desktop version found on the Spotify website, not the Microsoft Store version.");
                Environment.Exit(0x3); // ERROR_PATH_NOT_FOUND
            } else if (!File.Exists(ChromeElfDllPath))
            {
                FatalErrorMsg(@"FATAL ERROR: chrome_elf.dll file not found in program directory. Make sure this program was installed from the proper source (HxxxB's GitHub), and you don't remove any files in the distribution.");
                Environment.Exit(0x2); // ERROR_FILE_NOT_FOUND
            } else if (!File.Exists(ChromeElfDllConfigPath))
            {
                FatalErrorMsg(@"FATAL ERROR: chrome_elf.dll config file (config.ini) not found in program directory. Make sure this program was installed from the proper source (HxxxB's GitHub), and you don't remove any files in the distribution.");
                Environment.Exit(0x2); // ERROR_FILE_NOT_FOUND
            } else if (!File.Exists(NetUtilsDllPath))
            {
                FatalErrorMsg(@"FATAL ERROR: netutils.dll file (config.ini) not found in program directory. Make sure this program was installed from the proper source (HxxxB's GitHub), and you don't remove any files in the distribution.");
                Environment.Exit(0x2); // ERROR_FILE_NOT_FOUND
            }
            
            try
            {
                File.Copy(HostsfilePath, Path.Combine(PathToPatch, @"hosts"), true);
                ExecuteCommand($@"start {PatcherBatPath}");
                File.Copy(NetUtilsDllPath, Path.Combine(_spotifyInstallationPath, NetUtilsDllPath), true);
                File.Copy(ChromeElfDllPath, Path.Combine(_spotifyInstallationPath, ChromeElfDllPath), true);
                File.Copy(ChromeElfDllConfigPath, Path.Combine(_spotifyInstallationPath, ChromeElfDllConfigPath), true);
            }
            catch(Exception ex)
            {
                FatalErrorMsg($@"FATAL ERROR: Patching failed.{_doubleNewline}{ex}{_doubleNewline} Please tell the developer (HxxxB) if you get this error.");
                Environment.Exit(0xBC); // ERROR_INVALID_STARTING_CODESEG
            }

            NormalMsg(@"PATCHING SUCCEEDED: As an additional note, this patch may not only block spotify ads, but other types of advertisements as well due to intertwining ad servers, which is a bonus, because who actually likes advertisments?", true);
            Environment.Exit(0x0); // SUCCESS
        }

        private static void ExecuteCommand(string command)
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

            appProcess.OutputDataReceived += (sender, e) =>
                Console.WriteLine("output>>" + e.Data);
            appProcess.BeginOutputReadLine();

            appProcess.ErrorDataReceived += (sender, e) =>
                Console.WriteLine("error>>" + e.Data);
            appProcess.BeginErrorReadLine();

            appProcess.WaitForExit();

            Console.WriteLine("ExitCode: {0}", appProcess.ExitCode);
            appProcess.Close();
        }
    }
}
