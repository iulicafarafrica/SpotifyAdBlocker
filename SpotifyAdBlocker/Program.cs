using System;
using System.Security.Principal;
using System.Windows.Forms;

namespace SpotifyAdBlocker
{
    internal static class Program
    {
        [STAThread]
        private static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new SpotifyAdBlockerForm());
        }

        // check if program is running as admin, which is required in order to copy the hosts file to system32
        internal static bool ProgramIsElevated => WindowsIdentity.GetCurrent().Owner.IsWellKnown(WellKnownSidType.BuiltinAdministratorsSid);
    }
}
