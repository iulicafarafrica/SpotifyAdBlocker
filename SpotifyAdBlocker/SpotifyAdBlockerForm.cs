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

        private void OnFormLoad(object sender, EventArgs e)
        {
        }

        private void PatchButtonActionClick(object sender, EventArgs e)
        {
            _patchclient.MainPatch();
        }

        private readonly Patching _patchclient = new Patching();
    }
}
