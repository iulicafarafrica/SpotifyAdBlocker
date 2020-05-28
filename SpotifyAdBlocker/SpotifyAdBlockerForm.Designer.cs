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
            this.SuspendLayout();
            // 
            // PatchButton
            // 
            this.PatchButton.Font = new System.Drawing.Font("Trebuchet MS", 24F);
            this.PatchButton.Location = new System.Drawing.Point(11, 13);
            this.PatchButton.Margin = new System.Windows.Forms.Padding(4);
            this.PatchButton.Name = "PatchButton";
            this.PatchButton.Size = new System.Drawing.Size(723, 95);
            this.PatchButton.TabIndex = 0;
            this.PatchButton.Text = "Patch Spotify Ads!";
            this.PatchButton.UseVisualStyleBackColor = true;
            this.PatchButton.Click += new System.EventHandler(this.PatchButtonActionClick);
            // 
            // SpotifyAdBlockerForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(12F, 25F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(744, 123);
            this.Controls.Add(this.PatchButton);
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "SpotifyAdBlockerForm";
            this.Text = "Spotify AdBlocker";
            this.Load += new System.EventHandler(this.OnFormLoad);
            this.ResumeLayout(false);

        }

        private Button PatchButton;
    }
}

