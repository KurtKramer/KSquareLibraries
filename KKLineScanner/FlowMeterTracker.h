#if  !defined(_FLOWMETERTRACKER_)
#define  _FLOWMETERTRACKER_

#include "KKBaseTypes.h"
#include "DateTime.h"
#include "GoalKeeper.h"
#include "KKStrParser.h"
#include "RunLog.h"
using namespace KKB;


namespace  KKLSC
{
  /**
   *@brief Class that keeps track the Flow-Meter-Counter values by scan-lines; using these values it will compute 
   * estimated flow rate.
   *
   *@details
   *@code
   * Glossary
   * 
   *
   * FlowRate(FR) = Rate of water flow in (Meters/Sec)
   *
   * FlowRateRatio(FRR) = Number TemporalPixel's per SpatialPixel with respect to physical distance; aka FlowRateFactor.  (TP's/SP's)
   *
   * ScanRate(SR) = Number of camera scan lines captured per second.  (ScanLines/Sec)  or  (TP's/Sec)
   *
   * SpatialPixel(SP) = From the image chambers width perspective;  
   *
   * TemporalPixel(TP) = One scan line;  The physical distance a pixel covers with respect to flow is dependent
   *                 on both scan rate and flow rate.   TP = (FRR)(SP).
   *
   * ChamberWidthSpatialy (CWS)  Distance across Imaging Chamber width as measured in meters (m).
   * ChamberWidthPixels   (CWP)  Number of pixels imaged across the imaging chamber. (1 + CropLeft - CropRight).  (SP's)
   * ChamberWidthTemporal (CWT)  Time it would take for Camera to cover same distance in direction of flow.
   *
   *  FlowRateRatio = ((CWS)(SR)) / ((FR)(CWP)).
   *  FlowRate      = ((CWS)(SR)) / ((FRR)(CWP))
   *  
   *
   *@endcode
   */
  class  FlowMeterTracker
  {
  public:
    typedef  FlowMeterTracker*  FlowMeterTrackerPtr;

    FlowMeterTracker ();

    FlowMeterTracker (const FlowMeterTracker&  entry);
  
    ~FlowMeterTracker ();

    /**
     * @param[in]  _flowMeterPresent      Indicates if there is a Flow Meter or will FlowRate and FlowRateRatio be derived from configuration; see _flowRateRatioDefault.
     * @param[in]  _flowRateRatioDefault  When flow meter s not present or unable to compute FlowRate  will utilize this value to return FlowRate and FlowRateRatio.
     * @param[in]  _historyTableSize      The number of FlowMeter readings that will be tracked.
     * @param[in]  _scanRate              Scan-Lines per second that camera is operating at.
     * @param[in]  _imagingWidthMeters    Width in meters that is being imaged.
     * @param[in]  _imagingWidthPixels    Number of pixels that is being imaged;  this is the same as distance refereed to by _imagingWidthMeters.
     * @param[in]  _ticsPerMeter          The number counter ticks that occur per meter; this value will help compute FlowRate.
     */
    void  Initialize (bool     _flowMeterPresent,
                      float    _flowRateRatioDefault,
                      kkint32  _historyTableSize,
                      float    _scanRate,
                      float    _imagingWidthMeters,
                      kkint32  _imagingWidthPixels,
                      float    _ticsPerMeter
                     );

    kkMemSize  MemoryConsumedEstimated ();
  
    // Access Methods
    bool    FlowMeterPresent     ()  const  {return flowMeterPresent;}
    float   FlowRateDefault      ()  const  {return flowRateDefault;}
    float   FlowRateRatioDefault ()  const  {return flowRateRatioDefault;}
    kkint32 HistoryTableSize     ()  const  {return historyTableSize;}
    float   ImagingWidthMeters   ()  const  {return imagingWidthMeters;}
    kkint32 ImagingWidthPixels   ()  const  {return imagingWidthPixels;}
    float   TicsPerMeter         ()  const  {return ticsPerMeter;}
    
    float  ComputeFlowRateFromFlowRateRatio (float _flowRateRatio);
    float  ComputeFlowRateRatioFromFlowRate (float _flowRate);


    void  AddEntry (kkuint32 _scanLineNum,
                    kkuint32 _counterValue
                   );


    /**  Computes flow rate using the two most current count entries. */
    float  FlowRateInstantaneous ();


    /**  Computes flow rate using full range of history table. */
    float  FlowRateTrend ();

    void   FlowRateRatioDefaultChanged (float _flowRateRatioDefault);

    void   GetFlowRateInstantaneous (float&   flowRate,
                                     float&   flowRateRatio
                                    );             


    void   GetFlowRateTrend (float&   flowRate,
                             float&   flowRateRatio
                            );             


    void   ScanRateChanged (float _newScanRate);


  private:
    struct  Entry
    {
      Entry ();
      kkuint32  scanLineNum;
      kkuint32  counterValue;
    };
    typedef  Entry*  EntryPtr;

    bool      flowMeterPresent;
    float     flowRateDefault;       /**< Default value to be used if Flow-Meter not present. */
    float     flowRateRatioDefault;  /**< Default value to be used if Flow-Meter not present. */
    EntryPtr  history;
    kkint32   historyTableSize;
    kkint32   historyLastIdxAdded;
    kkint32   historyOldestIdx;
    float     imagingWidthMeters;
    kkint32   imagingWidthPixels;    /**< Should represent the width of the imaging chamber in pixels. */
    float     scanRate;
    float     ticsPerMeter;          /**< Assuming that flow-rate(m/s) is a linear function of Flow-Meter-Count. */
  };  /* FlowMeterTracker */


  typedef  FlowMeterTracker::FlowMeterTrackerPtr  FlowMeterTrackerPtr;

};  /* KKLSC */

#endif

