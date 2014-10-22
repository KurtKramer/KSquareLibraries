using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace KKControls
{
  public partial class ScrollingChart: UserControl
  {
    private  Color    backGroundColor = Color.White;
    private  Color    plotColor       = Color.Blue;
    private  float    rangeMin        = -10.0f;
    private  float    rangeMax        = 10.0f;

    private  float[]  data            = null;

    public  ScrollingChart ()
    {
      InitializeComponent ();
    }

    
    /// <summary>
    /// Specify the BAckGround Color of chart line
    /// </summary>
    [Bindable (true), Category ("ScrollingChart Options"),
    Description ("Specify BackGround Color.")]
    public Color BackGroundColor
    {
      get {return backGroundColor;}
      set {backGroundColor = value;}
    }


    /// <summary>
    /// Specify the color of chart line
    /// </summary>
    [Bindable (true), Category ("ScrollingChart Options"),
    Description ("Specify Plot Color.")]
    public Color PlotColor
    {
      get {return plotColor;}
      set {plotColor = value;}
    }




    /// <summary>
    /// Specify Minimum Range Value
    /// </summary>
    [Bindable (true), Category ("ScrollingChart Options"),
    Description ("Range Minimum.")]
    public float RangeMin
    {
      get {return rangeMin;}
      set {rangeMin = value;}
    }



    /// <summary>
    /// Specify Maximum Range Value
    /// </summary>
    [Bindable (true), Category ("ScrollingChart Options"),
    Description ("Range Maximum.")]
    public float RangeMax
    {
      get {return rangeMax;}
      set {rangeMax = value;}
    }




    /// <summary>
    /// Data to be Plotted
    /// </summary>
    [Bindable (true), Category ("ScrollingChart Options"),
    Description ("Data to be ploted.")]
    public float[] Data
    {
      get {return data;}
      set {data = value;}
    }





  }
}
