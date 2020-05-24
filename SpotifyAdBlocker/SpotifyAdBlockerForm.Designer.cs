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
            this.PatchButton.Font = new System.Drawing.Font("Trebuchet MS", 24F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.PatchButton.Location = new System.Drawing.Point(30, 26);
            this.PatchButton.Name = "PatchButton";
            this.PatchButton.Size = new System.Drawing.Size(737, 108);
            this.PatchButton.TabIndex = 0;
            this.PatchButton.Text = "Patch Spotify Ads!";
            this.PatchButton.UseVisualStyleBackColor = true;
            this.PatchButton.Click += new System.EventHandler(this.PatchButtonActionClick);
            // 
            // SpotifyAdBlockerForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(12F, 25F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 165);
            this.Controls.Add(this.PatchButton);
            this.Name = "SpotifyAdBlockerForm";
            this.Text = Program.IsAdmin ? "Spotify AdBlocker" : "Spotify AdBlocker: WARNING - NOT ADMIN";
            this.Load += new System.EventHandler((object e, EventArgs args) => { });
            this.ResumeLayout(false);

        }

        private Button PatchButton;
    }
}

