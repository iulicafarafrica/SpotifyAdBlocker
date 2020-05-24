using System;
using System.IO;
using System.Windows.Forms;

namespace SpotifyAdBlocker
{ 
    internal class Patching
    {
        private const string PathToPatch = @"C:\Windows\System32\drivers\etc";
        private const string HostsfilePath = @".\hosts";

        // Error codes used in Environment.Exit calls are found here: https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes--0-499-
        public void Patch()
        {
            if (!Program.ProgramIsElevated)
            {
                MessageBox.Show($@"FATAL ERROR: This program has not been ran as an administrator. This program requires admin permissions, at the patching is done in System32\drivers\etc, a system directory.{Environment.NewLine}" +
                                "To run this program as an administrator, right click the program's exe file, and select 'Run as administrator'");
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
            }

            try
            {
                File.Copy(HostsfilePath, Path.Combine(PathToPatch, @"hosts"), true);
            }
            catch
            {
                MessageBox.Show(@"FATAL ERROR: Patching failed. Please tell the developer (HxxxB) if you get this error.");
                Environment.Exit(0xBC); // ERROR_INVALID_STARTING_CODESEG
            }

            MessageBox.Show(@"PATCHING SUCEEDED: You might need to restart spotify/your computer for patches to apply. Furthermore, this patch may not only block spotify ads, but other types of advertisements as well due to intertwining ad servers, which is a bonus, because who actually likes advertisments?");
            Environment.Exit(0x0); // SUCCESS
        }
    }
}
