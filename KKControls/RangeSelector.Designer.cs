namespace KKControls
{
  partial class RangeSelector
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
        components.Dispose();
      }
      base.Dispose(disposing);
    }

    #region Component Designer generated code

    /// <summary> 
    /// Required method for Designer support - do not modify 
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent ()
    {
      this.LowerBar = new System.Windows.Forms.Panel();
      this.UpperBar = new System.Windows.Forms.Panel();
      this.SuspendLayout();
      // 
      // LowerBar
      // 
      this.LowerBar.BackColor = System.Drawing.Color.Black;
      this.LowerBar.Location = new System.Drawing.Point(76, 0);
      this.LowerBar.Name = "LowerBar";
      this.LowerBar.Size = new System.Drawing.Size(9, 29);
      this.LowerBar.TabIndex = 0;
      // 
      // UpperBar
      // 
      this.UpperBar.BackColor = System.Drawing.Color.Black;
      this.UpperBar.Location = new System.Drawing.Point(940, 0);
      this.UpperBar.Name = "UpperBar";
      this.UpperBar.Size = new System.Drawing.Size(9, 29);
      this.UpperBar.TabIndex = 1;
      // 
      // RangeSelector
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
      this.Controls.Add(this.UpperBar);
      this.Controls.Add(this.LowerBar);
      this.Name = "RangeSelector";
      this.Size = new System.Drawing.Size(984, 27);
      this.ResumeLayout(false);

    }

    #endregion

    private System.Windows.Forms.Panel LowerBar;
    private System.Windows.Forms.Panel UpperBar;
  }
}
