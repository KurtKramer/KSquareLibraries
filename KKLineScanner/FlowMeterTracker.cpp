#include "FirstIncludes.h"
#include <stdlib.h>
#include <memory.h>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "MemoryDebug.h"
using namespace std;


#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "OSservices.h"
#include "KKStr.h"
using namespace KKB;

#include "Variables.h"

#include "FlowMeterTracker.h"
using  namespace  KKLSC;




FlowMeterTracker::FlowMeterTracker ():
    flowMeterPresent     (false),
    flowRateDefault      (1.0f),
    flowRateRatioDefault (1.0f),
    history              (NULL),
    historyTableSize     (-1),
    historyLastIdxAdded  (-1),
    historyOldestIdx     (-1),
    imagingWidthMeters   (0.0635f),
    imagingWidthPixels   (2048),
    scanRate             (0.0f),
    ticsPerMeter         (10.0f)
{
}



FlowMeterTracker::FlowMeterTracker (const FlowMeterTracker&  entry):
    flowMeterPresent     (entry.flowMeterPresent),
    flowRateDefault      (entry.flowRateDefault),
    flowRateRatioDefault (entry.flowRateRatioDefault),
    history              (NULL),
    historyTableSize     (entry.historyTableSize),
    historyLastIdxAdded  (entry.historyLastIdxAdded),
    historyOldestIdx     (entry.historyOldestIdx),
    imagingWidthMeters   (entry.imagingWidthMeters),
    imagingWidthPixels   (entry.imagingWidthPixels),
    scanRate             (entry.scanRate),
    ticsPerMeter         (entry.ticsPerMeter)
{
  history = new Entry[historyTableSize];
  for  (int32 x = 0;  x < historyTableSize;  ++x)
    history[x] = entry.history[x];
}




FlowMeterTracker::~FlowMeterTracker ()
{
  delete  history;
  history = NULL;
}


void  FlowMeterTracker::Initialize (bool     _flowMeterPresent,
                                    float    _flowRateRatioDefault,
                                    kkint32  _historyTableSize,
                                    float    _scanRate,
                                    float    _imagingWidthMeters,
                                    kkint32  _imagingWidthPixels,
                                    float    _ticsPerMeter
                                   )
{                                   
  flowMeterPresent     = _flowMeterPresent;
  flowRateRatioDefault = _flowRateRatioDefault;
  scanRate             = _scanRate;             
  imagingWidthMeters   = _imagingWidthMeters;  
  imagingWidthPixels   = _imagingWidthPixels;
  ticsPerMeter         = _ticsPerMeter;         

  historyTableSize = _historyTableSize;
  delete  history;
  history = new Entry[historyTableSize];
  historyLastIdxAdded = -1;
  historyOldestIdx    = -1;

  flowRateDefault = ComputeFlowRateFromFlowRateRatio (flowRateRatioDefault);
}



int32  FlowMeterTracker::MemoryConsumedEstimated ()
{
  int32  mem = sizeof (*this);
  if  (historyTableSize > 0)
    mem += sizeof (Entry) * historyTableSize;
  return  mem;
}



void  FlowMeterTracker::FlowRateRatioDefaultChanged (float _flowRateRatioDefault)
{
  flowRateRatioDefault = _flowRateRatioDefault;
  flowRateDefault = ComputeFlowRateFromFlowRateRatio (flowRateRatioDefault);
}



void  FlowMeterTracker::ScanRateChanged (float _newScanRate)
{
  scanRate = _newScanRate;
  flowRateDefault = ComputeFlowRateFromFlowRateRatio (flowRateRatioDefault);
}




void  FlowMeterTracker::AddEntry (uint32 _scanLineNum,
                                  uint32 _counterValue
                                 )
{
  if  (historyLastIdxAdded < 0)
  {
    historyLastIdxAdded = 0;
    historyOldestIdx    = 0;
  }
  else
  {
    ++historyLastIdxAdded;
    if  (historyLastIdxAdded >= historyTableSize)
      historyLastIdxAdded = 0;
    if  (historyLastIdxAdded == historyOldestIdx)
    {
      ++historyOldestIdx;
      if  (historyOldestIdx >= historyTableSize)
        historyOldestIdx = 0;
    }
  }
  history[historyLastIdxAdded].scanLineNum  = _scanLineNum;
  history[historyLastIdxAdded].counterValue = _counterValue;
}  /* AddEntry */



FlowMeterTracker::Entry::Entry ():
    scanLineNum (0),
    counterValue (0)
{
}



float  FlowMeterTracker::FlowRateInstantaneous ()
{
  if  ((historyLastIdxAdded == historyOldestIdx)  ||  (ticsPerMeter == 0.0f)  ||  (scanRate == 0.0f))
    return flowRateDefault;

  int32 prevIdx = historyLastIdxAdded - 1;
  if  (prevIdx < 0)
    prevIdx = historyTableSize - 1;

  EntryPtr  lastPtr = history + historyLastIdxAdded;
  EntryPtr  prevPtr = history + prevIdx;
 
  int32 tics      = lastPtr->counterValue - prevPtr->counterValue;
  int32 scanLines = lastPtr->scanLineNum  - prevPtr->scanLineNum;

  float meters = tics / ticsPerMeter;
  float secs   = scanLines / scanRate;

  return  meters / secs;
}  /* FlowRateInstantaneous */




float  FlowMeterTracker::FlowRateTrend ()
{
  if  ((historyLastIdxAdded == historyOldestIdx)  ||  (ticsPerMeter == 0.0f)  ||  (scanRate == 0.0f))
    return flowRateDefault;

  EntryPtr  lastPtr = history + historyLastIdxAdded;
  EntryPtr  prevPtr = history + historyOldestIdx;
 
  int32 tics      = lastPtr->counterValue - prevPtr->counterValue;
  int32 scanLines = lastPtr->scanLineNum  - prevPtr->scanLineNum;

  if  ((tics == 0)  ||  (scanLines == 0))
    return 0.0f;

  float meters = tics / ticsPerMeter;
  float secs   = scanLines / scanRate;

  return  meters / secs;
}  /* FlowRateTrend */




void  FlowMeterTracker::GetFlowRateInstantaneous (float&   flowRate,
                                                  float&   flowRateRatio
                                                 )                                                 
{
  if  (!flowMeterPresent)
  {
    flowRate      = flowRateDefault;
    flowRateRatio = flowRateRatioDefault;
    return;
  }

  flowRate = FlowRateInstantaneous ();
  flowRateRatio = ComputeFlowRateRatioFromFlowRate (flowRate);

  return;
}


void  FlowMeterTracker::GetFlowRateTrend (float&   flowRate,
                                          float&   flowRateRatio
                                         )                                                 
{
  if  ((!flowMeterPresent)  ||  (this->historyLastIdxAdded < 0)  ||  (historyLastIdxAdded == this->historyOldestIdx))
  {
    flowRate      = flowRateDefault;
    flowRateRatio = flowRateRatioDefault;
    return;
  }

  if  (!flowMeterPresent)
  {
    flowRate      = flowRateDefault;
    flowRateRatio = flowRateRatioDefault;
    return;
  }

  flowRate = FlowRateTrend ();
  flowRateRatio = ComputeFlowRateRatioFromFlowRate (flowRate);

  return;
}



float  FlowMeterTracker::ComputeFlowRateFromFlowRateRatio (float _flowRateRatio)
{
  if  ((_flowRateRatio == 0.0f)  ||  (imagingWidthPixels == 0.0f))
    return  flowRateDefault;

  //float  flowRate = (imagingWidthMeters * scanRate) / (_flowRateRatio * imagingWidthPixels);
  float  flowRate = (_flowRateRatio  * scanRate * imagingWidthMeters)  / imagingWidthPixels;

  return  flowRate;
}



float  FlowMeterTracker::ComputeFlowRateRatioFromFlowRate (float _flowRate)
{
  if  ((_flowRate == 0.0f)  ||  (imagingWidthPixels == 0.0f))
    return  flowRateRatioDefault;

  //float  flowRateRatio = (imagingWidthMeters * scanRate) / (_flowRate * imagingWidthPixels);
  float  flowRateRatio = (_flowRate * imagingWidthPixels) / (imagingWidthMeters * scanRate);

  return flowRateRatio;
}
