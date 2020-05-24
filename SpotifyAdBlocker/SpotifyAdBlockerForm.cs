using System;
using System.Windows.Forms;

namespace SpotifyAdBlocker
{
    public partial class SpotifyAdBlockerForm : Form
    {
        public SpotifyAdBlockerForm()
        {
            InitializeComponent();
            var resxicon = Properties.Resources.AppIcon;
            Icon = resxicon;
        }

        private void PatchButtonActionClick(object sender, EventArgs e)
        {
            new Patching().Patch();
        }
    }
}
