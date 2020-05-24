using System;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Principal;
using System.Threading;
using System.Windows.Forms;

namespace SpotifyAdBlocker
{
    internal static class Program
    {
        [STAThread]
        private static void Main()
        {
            MutexName = ((GuidAttribute)Assembly.GetExecutingAssembly().GetCustomAttributes(typeof(GuidAttribute), false)[0]).Value;
            MutexInst = new Mutex(true, MutexName, out var firstInstance);
            if (firstInstance)
            {
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);
                Application.Run(new SpotifyAdBlockerForm());
            }
            else
            {
                Patching.NormalMsg("RUNTIME WARNING: Another instance of this program is already running.");
            }
        }

        // Boolean that checks if program is running as admin, which is required due to hosts file accessibility 
        internal static readonly bool IsAdmin = WindowsIdentity.GetCurrent().Owner.IsWellKnown(WellKnownSidType.BuiltinAdministratorsSid);
        // Mutex instance/name
        internal static Mutex MutexInst { get; set; }
        internal static string MutexName { get; set; }
        internal static void CleanMutex()
        {
            if (MutexInst == null) return;

            MutexInst.Close();
            MutexInst = null;
        }

        // SpotifyAdBlocker internal metadata
        private static readonly string AppFullExePath = Application.ExecutablePath.Replace("/", "\\");

        internal static void RestartProgramAsAdmin()
        {
            try
            {
                CleanMutex();
                var pinfocopy = new ProcessStartInfo
                {
                    FileName = AppFullExePath,
                    UseShellExecute = true,
                    Verb = "runas"
                };

                using (var pcopy = new Process {StartInfo = pinfocopy})
                {
                    pcopy.Start();
                }

                Process.GetCurrentProcess().Kill();
            }
            catch
            {
                MutexInst = new Mutex(true, MutexName);
            }
        }
    }
}
