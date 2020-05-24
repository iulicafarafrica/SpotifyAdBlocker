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
        private const string DowngradeBatPath = @".\downgrade.bat";
        private const string NetUtilsDllPath = @".\netutils.dll";
        private readonly string _spotifyInstallationPath = $@"C:\Users\{Environment.UserName}\AppData\Roaming\Spotify";

        // Error codes used in Environment.Exit calls are found here: https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes--0-499-
        public void Patch()
        {
            if (!Program.ProgramIsElevated)
            {
                MessageBox.Show($@"FATAL ERROR: This program has not been ran as an administrator. This program requires admin permissions, at the patching is done in System32\drivers\etc, a system directory.{Environment.NewLine}{Environment.NewLine}To run this program as an administrator, right click the program's exe file, and select 'Run as administrator'");
                Environment.Exit(0x13); // ERROR_WRITE_PROTECT
            }

            if (!Directory.Exists(PathToPatch))
            {
                MessageBox.Show(@"FATAL ERROR: drivers\etc system path doesn't seem to exist. Is this a valid Windows 10 installation?");
                Environment.Exit(0x3); // ERROR_PATH_NOT_FOUND
            } else if (!File.Exists(HostsfilePath))
            {
                MessageBox.Show(@"FATAL ERROR: hosts file not found in program directory. Make sure this program was installed from the proper source (HxxxB's GitHub), and you don't remove any files in the distribution.");
                Environment.Exit(0x2); // ERROR_FILE_NOT_FOUND
            } else if (!File.Exists(DowngradeBatPath))
            {
                MessageBox.Show(@"FATAL ERROR: downgrade.bat file not found in program directory. Make sure this program was installed from the proper source (HxxxB's GitHub), and you don't remove any files in the distribution.");
                Environment.Exit(0x2); // ERROR_FILE_NOT_FOUND
            } else if (!Directory.Exists(_spotifyInstallationPath))
            {
                MessageBox.Show(@"FATAL ERROR: Spotify installation not found. Make sure you have the desktop version found on the Spotify website, not the Microsoft Store version.");
                Environment.Exit(0x2); // ERROR_FILE_NOT_FOUND
            }
            
            try
            {
                File.Copy(HostsfilePath, Path.Combine(PathToPatch, @"hosts"), true);
                ExecuteCommand($@"start {DowngradeBatPath}");
                File.Copy(NetUtilsDllPath, Path.Combine(_spotifyInstallationPath, "netutils.dll"), true);
            }
            catch(Exception x)
            {
                MessageBox.Show(@"FATAL ERROR: Patching failed. Please tell the developer (HxxxB) if you get this error.");
                Environment.Exit(0xBC); // ERROR_INVALID_STARTING_CODESEG
            }

            MessageBox.Show(@"PATCHING SUCCEEDED: You might need to restart spotify/your computer for patches to apply. Furthermore, this patch may not only block spotify ads, but other types of advertisements as well due to intertwining ad servers, which is a bonus, because who actually likes advertisments?");
            Environment.Exit(0x0); // SUCCESS
        }

        private static void ExecuteCommand(string command)
        {
            var processInfo = new ProcessStartInfo("cmd.exe", "/c " + command)
            {
                CreateNoWindow = true,
                UseShellExecute = false,
                RedirectStandardError = true,
                RedirectStandardOutput = true
            };

            var process = Process.Start(processInfo);

            process.OutputDataReceived += (sender,  e) =>
                Console.WriteLine("output>>" + e.Data);
            process.BeginOutputReadLine();

            process.ErrorDataReceived += (object sender, DataReceivedEventArgs e) =>
                Console.WriteLine("error>>" + e.Data);
            process.BeginErrorReadLine();

            process.WaitForExit();

            Console.WriteLine("ExitCode: {0}", process.ExitCode);
            process.Close();
        }
    }
}
