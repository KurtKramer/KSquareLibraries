namespace KKControls
{
  partial class RadioButtonList
  {
    /// <summary> 
    /// Required designer variable.
    /// </summary>
    private System.ComponentModel.IContainer components = null;

    /// <summary> 
    /// Clean up any resources being used.
    /// </summary>
    /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
    protected override void Dispose (bool disposing)
    {
      if (disposing && (components != null))
      {
        components.Dispose ();
      }
      base.Dispose (disposing);
    }

    #region Component Designer generated code

    /// <summary> 
    /// Required method for Designer support - do not modify 
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent ()
    {
      this.optionsPanel = new System.Windows.Forms.GroupBox ();
      this.SuspendLayout ();
      // 
      // optionsPanel
      // 
      this.optionsPanel.Location = new System.Drawing.Point (0, 0);
      this.optionsPanel.Name = "optionsPanel";
      this.optionsPanel.Size = new System.Drawing.Size (150, 150);
      this.optionsPanel.TabIndex = 0;
      this.optionsPanel.TabStop = false;
      this.optionsPanel.Text = "Options";
      this.optionsPanel.SizeChanged += new System.EventHandler (this.optionsPanel_SizeChanged);
      // 
      // RadioButtonList
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF (6F, 13F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.Controls.Add (this.optionsPanel);
      this.Name = "RadioButtonList";
      this.ResumeLayout (false);

    }

    #endregion

    private System.Windows.Forms.GroupBox optionsPanel;

  }
}
