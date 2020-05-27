using System;
using System.Windows.Forms;
using System.ComponentModel;

namespace SpotifyAdBlocker
{
    public partial class SpotifyAdBlockerForm
    {
        private IContainer components = null;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {
            this.PatchButton = new System.Windows.Forms.Button();
            this.HostsFilePatchDeprecated = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // PatchButton
            // 
            this.PatchButton.Font = new System.Drawing.Font("Trebuchet MS", 36F);
            this.PatchButton.Location = new System.Drawing.Point(15, 14);
            this.PatchButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.PatchButton.Name = "PatchButton";
            this.PatchButton.Size = new System.Drawing.Size(356, 136);
            this.PatchButton.TabIndex = 0;
            this.PatchButton.Text = "Patch Spotify Ads!";
            this.PatchButton.UseVisualStyleBackColor = true;
            this.PatchButton.Click += new System.EventHandler(this.PatchButtonActionClick);
            // 
            // HostsFilePatchDeprecated
            // 
            this.HostsFilePatchDeprecated.Font = new System.Drawing.Font("Trebuchet MS", 16F);
            this.HostsFilePatchDeprecated.Location = new System.Drawing.Point(15, 155);
            this.PatchButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.HostsFilePatchDeprecated.Name = "HostsFilePatchDeprecated";
            this.HostsFilePatchDeprecated.Size = new System.Drawing.Size(356, 36);
            this.HostsFilePatchDeprecated.TabIndex = 1;
            this.HostsFilePatchDeprecated.Text = "Hosts File Patching (deprecated)";
            this.HostsFilePatchDeprecated.UseVisualStyleBackColor = true;
            this.HostsFilePatchDeprecated.Click += new System.EventHandler(this.HostsFilePatchDeprecatedActionClick);
            // 
            // SpotifyAdBlockerForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(383, 203);
            this.Controls.Add(this.HostsFilePatchDeprecated);
            this.Controls.Add(this.PatchButton);
            this.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.Name = "SpotifyAdBlockerForm";
            this.Text = "Spotify AdBlocker";
            this.Load += new System.EventHandler(this.OnFormLoad);
            this.ResumeLayout(false);

        }

        private Button PatchButton;
        private Button HostsFilePatchDeprecated;
    }
}

