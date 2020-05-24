using System;
using System.Windows.Forms;

namespace SpotifyAdBlocker
{
    public partial class SpotifyAdBlockerForm : Form
    {
        public SpotifyAdBlockerForm()
        {
            InitializeComponent();
        }

        private void SpotifyAdBlockerForm_Load(object sender, EventArgs e)
        {
        }

        private void PatchButtonActionClick(object sender, EventArgs e)
        {
            new Patching().Patch();
        }
    }
}
