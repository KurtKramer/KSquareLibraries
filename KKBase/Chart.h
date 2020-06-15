/* Chart.h -- Used to build Charts from data-series.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef _CHART_
#define _CHART_

#include <vector>

#include "KKBaseTypes.h"
#include "KKStr.h"
#include "Raster.h"
#include "Point.h"

namespace  KKB
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

    kkint32    NumOfSeries ()  {return  toint32_t (series.size ());}
  
    void       AddAValue (kkuint32  _seriesIDX,
                          float  _xVal,
                          float  _yVal
                         );
  
    RasterPtr  CreateRaster ();
  
    void       DefineASeries (KKStr _name);
  
    void       DefineASeries (KKStr       _name,
                              PixelValue  _color
                             );
  
  
    void       DefineAXLabel (float  _xVal,
                              KKStr  _name
                             );
  
    void       Save (KKStr  _fileName);
  
    void       SaveAsImage (KKStr  _fileName);
  
  private:
    kkint32  LookUpXLableIDX (double  xVal);
  
    PlotPointPtr  LookUpPoint (kkint32 seriesIDX,
                               double  yVal
                              );
  
    Point  RasterCoordinates (const PlotPoint&  plotPoint);
  
  
  
    KKStr              title;
    SeriesList         series;
    std::vector<XLabel>     xLabels;
  
    float             minXValue;
    float             maxXValue;
    
    float             minYValue;
    float             maxYValue;
  
    kkint32           maxPointsPlotedInASeries;
  
  
    kkint32           numOfDefaultColorsUsed;
  
    // Used when creating Raster of Chart.
    kkint32 chartWidth;
    kkint32 chartHeight;
    kkint32 xOffset;
    kkint32 yOffset;
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

}  /* KKB */

#endif
