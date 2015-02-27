/* Chart.h -- Used to build Charts from data-series.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#ifndef _CHART_
#define  _CHART_

#include  "BasicTypes.h"
#include  "Str.h"
#include  "Raster.h"
#include  "Point.h"

namespace  KKU
{
  /**
   *@brief Used to Create chart's from defined series of data.
   *@details  This class is meant to allow you to define multiple series, their x and y ranges and the 
   * data for each series.
   *@todo  This class Chart has not been used as of yet so requires testing.
   */

  class  Chart
  {
  public:
    class  Series;
    class  XLabel;
    class  PlotPoint;

    typedef  Series*            SeriesPtr;
    typedef  vector<SeriesPtr>  SeriesList;
    typedef  PlotPoint*         PlotPointPtr;


    Chart (KKStr  _title);
  
    ~Chart ();

    int32      NumOfSeries ()  {return  (int32)series.size ();}
  
    void       AddAValue (uint32  _seriesIDX,
                          FFLOAT  _xVal,
                          FFLOAT  _yVal
                         );
  
    RasterPtr  CreateRaster ();
  
    void       DefineASeries (KKStr _name);
  
    void       DefineASeries (KKStr       _name,
                              PixelValue  _color
                             );
  
  
    void       DefineAXLabel (FFLOAT  _xVal,
                              KKStr  _name
                             );
  
    void       Save (KKStr  _fileName);
  
    void       SaveAsImage (KKStr  _fileName);
  
  private:
    int32  LookUpXLableIDX (double  xVal);
  
    PlotPointPtr  LookUpPoint (int32   seriesIDX,
                               double  yVal
                              );
  
    Point  RasterCoordinates (const PlotPoint&  plotPoint);
  
  
  
    KKStr              title;
    vector<SeriesPtr>  series;
    vector<XLabel>     xLabels;
  
    FFLOAT             minXValue;
    FFLOAT             maxXValue;
    
    FFLOAT             minYValue;
    FFLOAT             maxYValue;
  
    int32              maxPointsPlotedInASeries;
  
  
    int32              numOfDefaultColorsUsed;
  
    // Used when creating Raster of Chart.
    int32   chartWidth;
    int32   chartHeight;
    int32   xOffset;
    int32   yOffset;
    double  yRange;
    double  xRange;
    double  yScale;
    double  xScale;
    double  xMin;
    double  xMax;
    double  yMin;
    double  yMax;
    double  yIncrement;
  }; /* Chart */

}  /* KKU */

#endif
