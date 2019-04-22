#include  "FirstIncludes.h"
#include  <stdio.h>
#include  <iomanip>
#include  <string>
#include  <iostream>
#include  <fstream>
#include  <map>
#include  <vector>
#include  "MemoryDebug.h"
#include  "KKBaseTypes.h"
using namespace std;

#include  "OSservices.h"
#include  "RunLog.h"
using namespace KKB;


#include  "ConfusionMatrix2.h"
using namespace  KKMLL;



ConfusionMatrix2::ConfusionMatrix2 (const MLClassList&  _classes,  // Will make its own copy of '_classes'
                                    istream&            f,
                                    kkuint32            _bucketSize,
                                    kkuint32            _numOfBuckets,
                                    kkuint32            _numOfProbBuckets,
                                    kkuint32            _probBucketSize,
                                    RunLog&             _log
                                   ):
  bucketSize                  (_bucketSize),
  classCount                  (0),
  classes                     (_classes),
  correctByKnownClassByProb   (),
  correctByKnownClassBySize   (),
  correctCount                (0.0),
  countByKnownClassByProb     (),
  countByKnownClassBySize     (),
  countsByKnownClass          (),
  numInvalidClassesPredicted  (0.0),
  numOfBuckets                (_numOfBuckets),
  numOfProbBuckets            (_numOfProbBuckets),
  predictedCountsCM           (),
  probBucketSize              (_probBucketSize),
  totalCount                  (0.0),
  totalPredProb               (0.0),
  totalPredProbsByKnownClass  (),
  totalSizesByKnownClass      (),
  totPredProbCM               ()
{
  InitializeMemory ();
  Read (f, _log);
}



ConfusionMatrix2::ConfusionMatrix2 (const MLClassList&  _classes):   // Will make its own copy of list
  bucketSize                  (100),
  classCount                  (0),
  classes                     (_classes),
  correctByKnownClassByProb   (),
  correctByKnownClassBySize   (),
  correctCount                (0.0),
  countByKnownClassByProb     (),
  countByKnownClassBySize     (),
  countsByKnownClass          (),
  numInvalidClassesPredicted  (0.0),
  numOfBuckets                (40),
  numOfProbBuckets            (20),
  predictedCountsCM           (),
  probBucketSize              (5),
  totalCount                  (0.0),
  totalPredProb               (0.0),
  totalPredProbsByKnownClass  (),
  totalSizesByKnownClass      (),
  totPredProbCM               ()
{
  InitializeMemory ();
}



ConfusionMatrix2::ConfusionMatrix2 (const ConfusionMatrix2&  cm):
  bucketSize                  (cm.bucketSize),
  classCount                  (cm.classCount),
  classes                     (cm.classes),
  correctByKnownClassByProb   (),
  correctByKnownClassBySize   (),
  correctCount                (cm.correctCount),
  countByKnownClassByProb     (),
  countByKnownClassBySize     (),
  countsByKnownClass          (),
  numInvalidClassesPredicted  (cm.numInvalidClassesPredicted),
  numOfBuckets                (cm.numOfBuckets),
  numOfProbBuckets            (cm.numOfProbBuckets),
  predictedCountsCM           (),
  probBucketSize              (cm.probBucketSize),
  totalCount                  (cm.totalCount),
  totalPredProb               (cm.totalPredProb),
  totalPredProbsByKnownClass  (),
  totalSizesByKnownClass      (),
  totPredProbCM               ()
{
  CopyVector (cm.countsByKnownClass,         countsByKnownClass);
  CopyVector (cm.totalPredProbsByKnownClass, totalPredProbsByKnownClass);
  CopyVector (cm.totalSizesByKnownClass,     totalSizesByKnownClass);

  CopyVectorDoublePtr (cm.predictedCountsCM, predictedCountsCM,   classCount);
  CopyVectorDoublePtr (cm.totPredProbCM,     totPredProbCM,       classCount);

  CopyVectorDoublePtr (cm.countByKnownClassBySize,   countByKnownClassBySize,   numOfBuckets);
  CopyVectorDoublePtr (cm.correctByKnownClassBySize, correctByKnownClassBySize, numOfBuckets);
  CopyVectorDoublePtr (cm.countByKnownClassByProb,   countByKnownClassByProb,   numOfProbBuckets);
  CopyVectorDoublePtr (cm.correctByKnownClassByProb, correctByKnownClassByProb, numOfProbBuckets);
}



ConfusionMatrix2::~ConfusionMatrix2 ()
  {
  DeleteVectorDoublePtr (countByKnownClassBySize);
  DeleteVectorDoublePtr (correctByKnownClassBySize);
  DeleteVectorDoublePtr (countByKnownClassByProb);
  DeleteVectorDoublePtr (correctByKnownClassByProb);
  DeleteVectorDoublePtr (predictedCountsCM);
  DeleteVectorDoublePtr (totPredProbCM);
}



void  ConfusionMatrix2::InitializeMemory ()
{
  classes.SortByName ();

  classCount = classes.QueueSize ();

  InitializeVector (countsByKnownClass,         classCount);
  InitializeVector (totalSizesByKnownClass,     classCount);
  InitializeVector (totalPredProbsByKnownClass, classCount);

  InitializeVectorDoublePtr (predictedCountsCM,         classCount, classCount);
  InitializeVectorDoublePtr (totPredProbCM,             classCount, classCount);

  InitializeVectorDoublePtr (countByKnownClassBySize,   classCount, numOfBuckets);
  InitializeVectorDoublePtr (correctByKnownClassBySize, classCount, numOfBuckets);

  InitializeVectorDoublePtr (countByKnownClassByProb,   classCount, numOfProbBuckets);
  InitializeVectorDoublePtr (correctByKnownClassByProb, classCount, numOfProbBuckets);
}  /* InitializeMemory */



void  ConfusionMatrix2::InitializeVector (vector<double>&  v,
                                          kkuint32         x
                                         )
{
  v.clear ();
  for  (kkuint32 y = 0;  y < x;  ++y)
    v.push_back (0.0);
  }



void  ConfusionMatrix2::CopyVector (const vector<double>&  src,
                                    vector<double>&        dest
                                   )
{
  dest.clear ();

  vector<double>::const_iterator idx;
  for  (idx = src.begin ();  idx != src.end ();  ++idx)
    dest.push_back (*idx);
}  /* CopyVector */



void  ConfusionMatrix2::InitializeVectorDoublePtr (vector<double*>& v,
                                                   kkuint32         numClasses,
                                                   kkuint32         numBuckets
                                                  )
{
  for  (kkuint32 x = 0;  x < v.size ();  ++x)
  {
    delete  v[x];
    v[x] = NULL;
}

  v.clear ();
  while  (v.size () < numClasses)
  {
    double*  d = new double[numBuckets];
    v.push_back (d);
    for  (kkuint32 y = 0;  y < numBuckets;  ++y)
      d[y] = 0.0;
  }
}  /* InitializeVectorDoublePtr */



void  ConfusionMatrix2::IncreaseVectorDoublePtr (vector<double*>&  v,
                                                 kkuint32          numBucketsOld,
                                                 kkuint32          numBucketsNew
                                                )
{
  if  (numBucketsOld != numBucketsNew)
  {
    vector<double*>::iterator  idx;
    for  (idx = v.begin ();  idx != v.end ();  ++idx)
    {
      double*  oldArray = *idx;
      double*  newArray = new double[numBucketsNew];
      for  (kkuint32 x = 0;  x < numBucketsOld;  ++x)
        newArray[x]= oldArray[x];

      for  (kkuint32 x = numBucketsOld;  x < numBucketsNew;  ++x)
        newArray[x] = 0.0;

      *idx = newArray;
      delete  oldArray;
      oldArray = NULL;
    }
  }

  double*  d = new double[numBucketsNew];
  v.push_back (d);
  for  (kkuint32 x = 0;  x < numBucketsNew;  ++x)
    d[x] = 0.0;

}  /* IncreaseVectorDoublePtr */



void  ConfusionMatrix2::CopyVectorDoublePtr (const vector<double*>&  src,
                                             vector<double*>&        dest,
                                             kkuint32                numBuckets
                                            )
{
  for  (kkuint32 x = 0;  x < dest.size ();  ++x)
  {
    delete  dest[x];
    dest[x] = NULL;
  }

  kkuint32 classIdx = 0;
  dest.clear ();
  while  (dest.size () < src.size ())
  {
    double*  s = src[classIdx];
    double*  d = new double[numBuckets];
    dest.push_back (d);
    for  (kkuint32 y = 0;  y < numBuckets;  ++y)
      d[y] = s[y];

    ++classIdx;
  }
}  /* CopyVectorDoublePtr */



void  ConfusionMatrix2::DeleteVectorDoublePtr (vector<double*>&  v)
    {
  for  (kkuint32 x = 0;  x < v.size ();  ++x)
    {
    delete  v[x];
    v[x] = NULL;
    }
}  /* DeleteVectorDoublePtr */



kkuint32  ConfusionMatrix2::AddClassToConfusionMatrix (MLClassPtr  newClass,
                                                       RunLog&     log
                                                      )
{
  auto existingClassIdx = classes.PtrToIdx (newClass);
  if  (existingClassIdx)
  {
    log.Level (-1) << endl
      << "ConfusionMatrix2::AddClassToConfusionMatrix  ***ERROR***  Class[" << newClass->Name () << "]  already in class list." << endl
      << endl;
    return  existingClassIdx.value ();
  }

  classes.PushOnBack (newClass);
  classCount++;
 
  IncreaseVectorDoublePtr (correctByKnownClassByProb, numOfProbBuckets, numOfProbBuckets);
  IncreaseVectorDoublePtr (countByKnownClassByProb,   numOfProbBuckets, numOfProbBuckets);

  IncreaseVectorDoublePtr (correctByKnownClassBySize, numOfBuckets, numOfBuckets);
  IncreaseVectorDoublePtr (countByKnownClassBySize,   numOfBuckets, numOfBuckets);

  IncreaseVectorDoublePtr (predictedCountsCM,         classCount - 1, classCount);
  IncreaseVectorDoublePtr (totPredProbCM,             classCount - 1, classCount);

  countsByKnownClass.push_back         (0.0);
  totalPredProbsByKnownClass.push_back (0.0); 
  totalSizesByKnownClass.push_back     (0.0);

  return classes.PtrToIdx (newClass).value ();
}  /* AddClassToConfusionMatrix */



double  ConfusionMatrix2::PredictedCountsCM (kkuint32  knownClassIdx, 
                                             kkuint32  predClassIdx
                                            )  const
{
  if  (knownClassIdx >= classCount)
    return 0.0;
 
  if  (predClassIdx >= classCount)
    return 0.0;
 
  return  predictedCountsCM [knownClassIdx][predClassIdx];
}



VectorDouble  ConfusionMatrix2::PredictedCounts ()  const
{
  kkuint32  knownClassIdx, predClassIdx;

  VectorDouble  pc;
  for  (predClassIdx = 0;  predClassIdx < classCount;  predClassIdx++)
  {
    double  predCount = 0.0;
    for  (knownClassIdx = 0;  knownClassIdx < classCount;  knownClassIdx++)
      predCount += predictedCountsCM[knownClassIdx][predClassIdx];
    pc.push_back (predCount);
  }
 
  return  pc;
}  /* PredictedCounts */



double  ConfusionMatrix2::CountsByKnownClass (kkuint32 knownClassIdx)  const
{
  if  (knownClassIdx >= classCount)
    return 0.0;
  
  return  countsByKnownClass [knownClassIdx];
}



const VectorDouble&  ConfusionMatrix2::CountsByKnownClass ()  const
{
  return  countsByKnownClass;
}  /* CountsByKnownClass */



void  ConfusionMatrix2::Increment (MLClassPtr  _knownClass,
                                   MLClassPtr  _predClass,
                                   kkint32     _size,
                                   double      _probability,
                                   RunLog&     _log
                                  )
{
  OptionUInt32  knownClassNum = {};
  OptionUInt32  predClassNum  = {};

  if  (_probability < 0)
    _probability = 0;

  if  (!_knownClass)
  {
    numInvalidClassesPredicted += 1.0;
    _log.Level (-1) << endl
                   << "ConfusionMatrix2::Increment  **** _knownClass = NULL ****"
                   << endl
                   << endl;
    return;
  }

  if  (!_predClass)
  {
    numInvalidClassesPredicted += 1.0;
    _log.Level (-1) << endl
                   << "ConfusionMatrix2::Increment  **** _predClass = NULL ****"
                   << endl
                   << endl;
    return;
  }

  knownClassNum = classes.PtrToIdx (_knownClass);
  if  (!knownClassNum)
    knownClassNum = AddClassToConfusionMatrix (_knownClass, _log);

  predClassNum  = classes.PtrToIdx (_predClass);
  if  (!predClassNum)
    predClassNum = AddClassToConfusionMatrix (_predClass, _log);

  auto knownClassIdx = knownClassNum.value ();
  auto predClassIdx  = predClassNum.value ();

  if  (knownClassIdx == predClassIdx)
     correctCount += 1.0;

  totalCount += 1.0;
   
  totalSizesByKnownClass[knownClassIdx] += _size;

  totalPredProbsByKnownClass [knownClassIdx] += _probability;
  totalPredProb                              += _probability;

  countsByKnownClass [knownClassIdx]++;

  (predictedCountsCM [knownClassIdx] [predClassIdx])++;
  totPredProbCM      [knownClassIdx] [predClassIdx] += _probability;
  
  if  (_size > 0)
  {
    kkuint32  bucket = (_size - 1) / bucketSize;
    if  (bucket >= numOfBuckets)
      bucket = numOfBuckets - 1;

    countByKnownClassBySize[knownClassIdx][bucket]++;
    if  (knownClassIdx == predClassIdx)
      correctByKnownClassBySize [knownClassIdx][bucket]++;
  }
  else
  {
    _size = -1;
  }

  {
     kkuint32  bucket = 0;
     
     if  ((_probability >= 0.0)  &&  (_probability <= 1.0))
       bucket = ((kkuint32)(_probability * 100) / probBucketSize);
     else
       bucket = 0;

     if  (bucket >= numOfProbBuckets)
       bucket = numOfProbBuckets - 1;

    countByKnownClassByProb [knownClassIdx][bucket]++;
    if  (knownClassIdx == predClassNum)
      correctByKnownClassByProb [knownClassIdx][bucket]++;
  }
}  /* Increment */



KKStr  StripOutInvalidLatexCaracters (const KKStr&  src)
{
  kkuint32  newLen = (kkint32)(src.Len () * 1.3);

  KKStr  result (newLen);

  for  (kkuint32 x = 0;  x < src.Len ();  x++)
  {
    char  ch = src[x];

    switch  (ch)
    {
      case  '#': result << "\\#";
                  break;

      case  '$':  result << "\\$";
                  break;

      case  '&':  result << "\\&";
                  break;

      case  '_':  result << "\\_";
                  break;

      case  '%':  result << "\\%";
                  break;

      case  '{':  result << "\\{";
                  break;

      case  '}':  result << "\\}";
                  break;


        default:  result.Append (ch);
                  break;
    }

  }

  return  result;
}  /* StripOutInvalidLatexCaracters */



void  ConfusionMatrix2::PrintSingleLine (ostream& _outFile,
                                         KKStr    _name,
                                         double   _lineTotal,
                                         double   _splits[]
                                        )  const
{
  if  (_name.Len () > 25)
    _name = _name.SubStrPart (_name.Len () - 25);

  _outFile << setw (25) << _name
           << setw (16) << _lineTotal;

  for  (kkuint32 predClassNum = 0; predClassNum < classCount; predClassNum++)
  {
    _outFile << setw (16) << _splits [predClassNum];
  }

  _outFile << endl;
}  /* PrintSingleLine */



void  ConfusionMatrix2::PrintSingleLineTabDelimited (ostream&      _outFile,
                                                     const KKStr&  _name,
                                                     double        _lineTotal,
                                                     double        _splits[]
                                                    )  const
{

  KKStr  name (_name);
  name << "(" << _lineTotal << ")";

  _outFile << _name << "\t" << _lineTotal;

  for  (kkuint32 predClassNum = 0; predClassNum < classCount; predClassNum++)
  {
    _outFile << "\t" << _splits [predClassNum];
  }

  _outFile << endl;
}  /* PrintSingleLineTabDelimited */


                                                     
void  ConfusionMatrix2::PrintSingleLineHTML (ostream&     o,
                                             const KKStr& _name,
                                             double       _lineTotal,
                                             kkuint32     _knownClassNum,
                                             double       _splits[]
                                            )  const
{
  kkuint32  predClassNum;

  o << "    <tr><td style=\"text-align:left; font-family:Arial\">" << _name << "</td>" << "<td>" << _lineTotal << "</td>";

  for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
  {
    if  (predClassNum == _knownClassNum)
    {
      o << "<td style=\"font-weight:bold\">"
        << _splits [predClassNum] 
        << "</td>";
    }
    else
    {
      o << "<td>";
      if  (_splits [predClassNum] != 0.0)
        o << _splits [predClassNum];
      o << "</td>";
    }
  }
  o << "</tr>" << endl;
}  /* PrintSingleLineHTML */



void  ConfusionMatrix2::PrintSingleLineLatexTable (ostream&       _outFile,
                                                   kkuint32       _knownClassNum, 
                                                   const KKStr&   _name,
                                                   double         _lineTotal,
                                                   double         _splits[]
                                                  )  const
{
  KKStr  name (_name);
  name << "(" << _lineTotal << ")";

  _outFile << StripOutInvalidLatexCaracters (_name) << " & " << _lineTotal;

  for  (kkuint32 predClassNum = 0;  predClassNum < classCount;  ++predClassNum)
  {
    _outFile << " & ";
    if  (_knownClassNum == predClassNum)
      _outFile << "\\textbf{";

    _outFile << _splits [predClassNum];

    if  (_knownClassNum == predClassNum)
      _outFile << "}";
  }

  _outFile << "\\\\" << endl;
}  /* PrintSingleLineLatexTable */



void  ConfusionMatrix2::PrintSingleLineShort (ostream&      _outFile,
                                              const KKStr&  _name,
                                              double        _lineTotal,
                                              double        _splits[])    const
{
  KKStr  name (_name);
  name << "(" << _lineTotal << ")";

  _outFile << setw (25) << name;

  for  (kkuint32 predClassNum = 0; predClassNum < classCount; predClassNum++)
  {
    _outFile << setw (6) << _splits [predClassNum];
  }

  _outFile << endl;
}



void  ConfusionMatrix2::PrintPercentLine (ostream&  _outFile,
                                          KKStr     _name,
                                          double    _lineTotal,
                                          double    _splits[]
                                         )  const
{
  char    buff[40];
  double  perc;

  if  (totalCount == 0.0)
    perc = 0.0;
  else
    perc = (double)_lineTotal / totalCount;

# ifdef  USE_SECURE_FUNCS
    sprintf_s (buff, sizeof (buff), "%.1f%%", (100.0 * perc));
# else
  sprintf (buff, "%.1f%%", (100.0 * perc));
# endif

  if  (_name.Len () > 25)
    _name = _name.SubStrPart (_name.Len () - 25);

  _outFile << setw (25) << _name 
           << setw (16) << buff;

  for  (kkuint32 predClassNum = 0; predClassNum < classCount; predClassNum++)
  {
    if  (_lineTotal <= 0)
       perc = 0.0;
    else
       perc = 100.0 * (double)_splits[predClassNum] / (double)_lineTotal;


# ifdef  USE_SECURE_FUNCS
    sprintf_s (buff, sizeof (buff), "%.3f%%", perc);
# else
    sprintf (buff, "%.3f%%", perc);
#endif

    _outFile << setw (16) << buff;
  }
     
  _outFile << endl;
}  /* PrintPercentLine */



void  ConfusionMatrix2::PrintPercentLineTabDelimited (ostream&      _outFile,
                                                      const KKStr&  _name,
                                                      double        _lineTotal,
                                                      double        _splits[]
                                                     )  const
{
  double  perc;

  if  (totalCount <= 0.0)
    perc = 0.0;
  else
    perc = (double)_lineTotal / totalCount;

  _outFile << _name << "\t" 
           << StrFormatDouble ((100.0 * perc), "zz0.00") << "%";

  for  (kkuint32 predClassNum = 0;  predClassNum < classCount;  ++predClassNum)
  {
    if  (_lineTotal <= 0)
       perc = 0.0;
    else
       perc = 100.0 * (double)_splits[predClassNum] / (double)_lineTotal;

    _outFile << "\t" << StrFormatDouble (perc, "ZZ0.000") << "%";
  }
     
  _outFile << endl;
}  /* PrintPercentLineTabDelimited */



void  ConfusionMatrix2::PrintAvgPredProbLineHTML (ostream&       o,
                                                  const KKStr&  _name,
                                                  double        _totalAvgPredProbThisLine,
                                                  double        _totalCountThisLine,
                                                  kkuint32      _knownClassNum,
                                                  double        _avgPredProbs[],
                                                  double        _numPredByClass[]
                                                 )  const
{
  double  avgPredProb;

  if  (_totalCountThisLine <= 0.0)
    avgPredProb = 0.0;
  else
    avgPredProb = _totalAvgPredProbThisLine / _totalCountThisLine;

  KKStr avgPredProbStr = StrFormatDouble ((100.0 * avgPredProb), "zz0.00") + "%";
  o << "    <tr>" << "<td style=\"text-align:left; font-family:Arial\">" << _name << "</td>" << "<td>" << avgPredProbStr << "</td>";

  for  (kkuint32 predClassNum = 0;  predClassNum < classCount;  predClassNum++)
  {
    if  (_numPredByClass[predClassNum] <= 0.0)
       avgPredProb = 0.0;
    else
       avgPredProb = 100.0 * _avgPredProbs[predClassNum] / _numPredByClass[predClassNum];

    if  (predClassNum == _knownClassNum)
      o << "<td style=\"font-weight:bold\">";
    else
      o << "<td>";

    o << StrFormatDouble (avgPredProb, "ZZ0.000") << "%" << "</td>";
  }
     
  o << "</tr>" << endl;
}  /* PrintAvgPredProbLineHTML */



void  ConfusionMatrix2::PrintPercentLineHTML (ostream&      o,
                                              const KKStr&  _name,
                                              double        _lineTotal,
                                              kkuint32      _knownClassNum,
                                              double        _splits[]
                                             )  const
{
  double  perc;

  if  (totalCount <= 0.0)
    perc = 0.0;
  else
    perc = (double)_lineTotal / totalCount;

  KKStr percentStr = StrFormatDouble ((100.0 * perc), "zz0.00") + "%";
  o << "    <tr>" << "<td style=\"text-align:left; font-family:Arial\">" << _name << "</td>" << "<td>" << percentStr << "</td>";

  for  (kkuint32 predClassNum = 0;  predClassNum < classCount;  predClassNum++)
  {
    if  (_lineTotal <= 0)
       perc = 0.0;
    else
       perc = 100.0 * (double)_splits[predClassNum] / (double)_lineTotal;

    if  (predClassNum == _knownClassNum)
    {
      o << "<td style=\"font-weight:bold\">"
        << StrFormatDouble (perc, "ZZ0.000") << "%" 
        << "</td>";
    }
    else
    {
      o << "<td>";
      if  (perc != 0.0)
        o << StrFormatDouble (perc, "ZZ0.000") << "%";
      o << "</td>";
    }
  }
  o << "</tr>" << endl;
}  /* PrintPercentLineHTML */



void  ConfusionMatrix2::PrintPercentLineShort (ostream&     _outFile,
                                               const KKStr& _name,
                                               double       _lineTotal,
                                               double       _splits[]
                                              )  const
{
  double  perc;

  if  (totalCount == 0.0)
    perc = 0.0;
  else
    perc = 100.0 * (double)_lineTotal / totalCount;

  KKStr name (_name);
  name << "(" << StrFormatDouble (perc, "ZZ0.0") << ")";

  _outFile << setw (25) << name;

  for  (kkuint32 predClassNum = 0;  predClassNum < classCount;  ++predClassNum)
  {
    if  (_lineTotal <= 0)
       perc = 0.0;
    else
       perc = 100.0 * (double)_splits[predClassNum] / (double)_lineTotal;

    _outFile << setw (6) << StrFormatDouble (perc, "zz0.0");
  }
     
  _outFile << endl;
}  /* PrintPercentLineShort */



void  ConfusionMatrix2::PrintPercentLineLatexTable (ostream&      _outFile,
                                                    kkuint32      _rowNum,
                                                    const KKStr&  _name,
                                                    double        _lineTotal,
                                                    double        _splits[]
                                                    )  const
{   
  double  perc;

  if  (totalCount <= 0.0)
    perc = 0.0;
  else
    perc = (double)_lineTotal / totalCount;

  _outFile << StripOutInvalidLatexCaracters (_name) << " & " 
           << StrFormatDouble ((100.0 * perc), "zz0.0") << "\\%";

  for  (kkuint32 predClassNum = 0;  predClassNum < classCount;  ++predClassNum)
  {
    if  (_lineTotal <= 0)
       perc = 0.0;
    else
       perc = 100.0 * (double)_splits[predClassNum] / (double)_lineTotal;

    _outFile << " & ";
    if  (_rowNum == predClassNum)
      _outFile << "\\textbf{";

    _outFile << StrFormatDouble (perc, "ZZ0.00") << "\\%";
    if  (_rowNum == predClassNum)
      _outFile << "}";
  }

  _outFile << "\\\\" << endl;
}  /* PrintPercentLineLatexTable */



void   ConfusionMatrix2::PrintConfusionMatrix (ostream&  outFile)  const
{
  // Lets generate Titles first
  outFile << endl;

  double  perc = 0.0;
  if  (totalCount > 0.0)
    perc = correctCount / totalCount;

  outFile  << "Overall Accuracy is " 
           << setprecision (5)
           << (100.0 * perc) << "%"
           << endl;

  outFile  << endl;

  KKStr  titleLine1, titleLine2, titleLine3;
  classes.ExtractThreeTitleLines (titleLine1, titleLine2, titleLine3, 16);

  outFile << setw (25) << ""          << setw(16) << ""      << setw (0) << titleLine1 << endl;
  outFile << setw (25) << ""          << setw(16) << ""      << setw (0) << titleLine2 << endl;
  outFile << setw (25) << "ClassName" << setw(16) << "Count" << setw (0) << titleLine3 << endl;

  outFile << setw (25) << "==========="  << setw(16) << "====";
  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    outFile << setw (16) << "============";
  }
  outFile << endl;

  double*  totals = new double[classCount];
  for  (kkuint32 x = 0; x < classCount; x++)
    totals[x] = 0;


  double  totalNonNoise = 0;
  double  totalNonNoiseRight = 0;

  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    bool  noiseClass = classes[knownClassNum].UnDefined ();

    for  (kkuint32 predClassNum = 0; predClassNum < classCount; predClassNum++)
    {
      totals[predClassNum] += predictedCountsCM[knownClassNum] [predClassNum];
    }
     
    PrintSingleLine (outFile, 
                     classes            [knownClassNum].Name (),
                     countsByKnownClass [knownClassNum],
                     predictedCountsCM  [knownClassNum]
                    );

    if  (!noiseClass)
    {
      totalNonNoise = totalNonNoise + countsByKnownClass [knownClassNum];
      totalNonNoiseRight += predictedCountsCM [knownClassNum] [knownClassNum];
    }
  }

  PrintSingleLine (outFile, 
                   KKStr ("Totals"),
                   totalCount,
                   totals
                  );

  outFile << endl << endl;

  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    PrintPercentLine (outFile, 
                      classes            [knownClassNum].Name (),
                      countsByKnownClass [knownClassNum],
                      predictedCountsCM  [knownClassNum]
                     );
  }

  outFile << endl
          << endl;

  perc = 0.0;
  if  (totalNonNoise != 0)
    perc = (double)totalNonNoiseRight / (double)totalNonNoise;

  outFile << "Accuracy for Non Noise "  
          << setprecision (5)  
          << (perc * 100.0)
          << "%"
          << endl;


  outFile << endl << endl;

  delete[]  totals;
}  /* PrintConfusionMatrix */



void  ConfusionMatrix2::PrintConfusionMatrixHTML (ostream&  o)  const
{
  double  overallAccuracy = 0.0;
  if  (totalCount != 0.0)
    overallAccuracy = 100.0 * correctCount / totalCount;

  if  (numInvalidClassesPredicted > 0.0)
  {
    o << "<p style=\"font-weight:bold\">" << endl 
      << "*********************************************************************************************<br />"  << endl
      << "*******************        WARNING    WARNING   WARNING  WARNING        *********************<br />"  << endl
      << "*******************                                                     *********************<br />"  << endl
      << "*******************   There were invalid classes specified that were    *********************<br />"  << endl
      << "*******************   not counted.    numInvalidClassesPredicted[" << numInvalidClassesPredicted << "] *********************<br />" << endl
      << "*********************************************************************************************<br />"  << endl
      << "</p>"   << endl
      << "<br />" << endl;
  }

  o << "Overall Accuracy: " 
    << StrFormatDouble (overallAccuracy, "ZZZ0.000") << "%"
    << endl;


  o << "<table align=\"center\" border=\"2\" cellpadding=\"3\" cellspacing=\"0\" frame=\"box\"  summary=\"Confusion \" >" << endl
    << "  <thead style=\"font-weight:bold; text-align:center; vertical-align:bottom\">"          << endl
    << "    <tr>"                                                                                << endl
    << "        <th>Class<br />Names</th><th>Count</th>" << classes.ExtractHTMLTableHeader ()    << endl
    << "    </tr>"                                                                               << endl
    << "  </thead>"                                                                              << endl
    << "  <tbody style=\"font-weight:normal; text-align:right; font-family:Courier\">" << endl;

  double*  totals = new double[classCount];
  for  (kkuint32 x = 0; x < classCount; x++)
    totals[x] = 0;

  double  totalNonNoise = 0;
  double  totalNonNoiseRight = 0;

  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    bool  noiseClass = classes[knownClassNum].UnDefined ();

    for  (kkuint32 predClassNum = 0; predClassNum < classCount; predClassNum++)
    {
      totals[predClassNum] += predictedCountsCM [knownClassNum] [predClassNum];
    }

    PrintSingleLineHTML (o, 
                         classes            [knownClassNum].Name (),
                         countsByKnownClass [knownClassNum],
                         knownClassNum,
                         predictedCountsCM  [knownClassNum]
                        );
    if  (!noiseClass)
    {
      totalNonNoise      = totalNonNoise      + countsByKnownClass [knownClassNum];
      totalNonNoiseRight = totalNonNoiseRight + predictedCountsCM  [knownClassNum] [knownClassNum];
    }

  }

  PrintSingleLineHTML (o, 
                       KKStr ("Totals"),
                       totalCount,
                       int32_max,
                       totals
                      );

  o << "<tr><td colspan=\"" << (classCount + 2) << "\">&nbsp</td></tr>" << endl;

  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    PrintPercentLineHTML (o, 
                          classes            [knownClassNum].Name (),
                          countsByKnownClass [knownClassNum],
                          knownClassNum,
                          predictedCountsCM  [knownClassNum]
                         );
  }

  o << "</tbody>" << endl
    << "</table>" << endl;

  double  nonNoiseAccuracy = 0.0;
  if  (totalNonNoise != 0)
    nonNoiseAccuracy = 100.0 * (double)totalNonNoiseRight / (double)totalNonNoise;

  o << "Non Noise Accuracy: " << StrFormatDouble (nonNoiseAccuracy, "ZZ0.000") << "%" << "<br />" << endl;

  delete  [] totals;
}  /* PrintConfusionMatrixHTML */



void  ConfusionMatrix2::PrintConfusionMatrixAvgPredProbHTML (ostream&   o)  const

{
  double  overallAvgPredProb = 0.0;
  if  (totalCount != 0.0)
    overallAvgPredProb = 100.0 * totalPredProb / totalCount;

  if  (numInvalidClassesPredicted > 0.0)
  {
    o << "<p style=\"font-weight:bold\">" << endl 
      << "*********************************************************************************************<br />"  << endl
      << "*******************        WARNING    WARNING   WARNING   WARNING       *********************<br />"  << endl
      << "*******************                                                     *********************<br />"  << endl
      << "*******************   There were invalid classes specified that were    *********************<br />"  << endl
      << "*******************   not counted.    numInvalidClassesPredicted[" << numInvalidClassesPredicted << "] *********************<br />" << endl
      << "*********************************************************************************************<br />"  << endl
      << "</p>"   << endl
      << "<br />" << endl;
  }
  
  o << "Overall AvgPredProb: " 
    << StrFormatDouble (overallAvgPredProb, "ZZZ0.000") << "%"
    << endl;
  
  o << "<table align=\"center\" border=\"2\" cellpadding=\"3\" cellspacing=\"0\" frame=\"box\"  summary=\"Confusion \" >" << endl
    << "  <thead style=\"font-weight:bold; text-align:center; vertical-align:bottom\">"          << endl
    << "    <tr>"                                                                                << endl
    << "        <th>Class<br />Names</th><th>Count</th>" << classes.ExtractHTMLTableHeader ()    << endl
    << "    </tr>"                                                                               << endl
    << "  </thead>"                                                                              << endl
    << "  <tbody style=\"font-weight:normal; text-align:right; font-family:Courier\">" << endl;

  double*  totalPredProbByPredClass = new double[classCount];
  double*  totalCountsByPredClass   = new double[classCount];
  for  (kkuint32 predClassNum = 0;  predClassNum < classCount;  predClassNum++)
  {
    totalPredProbByPredClass [predClassNum] = 0.0;
    totalCountsByPredClass   [predClassNum] = 0.0;
  }

  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    PrintAvgPredProbLineHTML (o, 
                              classes                    [knownClassNum].Name (),
                              totalPredProbsByKnownClass [knownClassNum],
                              countsByKnownClass         [knownClassNum],
                              knownClassNum,
                              totPredProbCM              [knownClassNum],
                              predictedCountsCM          [knownClassNum]
                             );
    for  (kkuint32 predClassNum = 0;  predClassNum < classCount;  predClassNum++)
    {
      totalPredProbByPredClass [predClassNum] += totPredProbCM     [knownClassNum][predClassNum];
      totalCountsByPredClass   [predClassNum] += predictedCountsCM [knownClassNum][predClassNum];
    }
  }

  o << "<tr><td colspan=\"" << (classCount + 2) << "\">&nbsp</td></tr>" << endl;

  kkuint32 knownClassNum = uint32_max;
  PrintAvgPredProbLineHTML (o, 
                            "AllClasses",
                            totalPredProb,
                            totalCount,
                            knownClassNum,
                            totalPredProbByPredClass,
                            totalCountsByPredClass
                           );

  o << "</tbody>" << endl
    << "</table>" << endl;

  delete[]  totalCountsByPredClass;
  delete[]  totalPredProbByPredClass;
}  /* PrintConfusionMatrixHTML */



void  ConfusionMatrix2::PrintAccuracyByProbByClassHTML (ostream&   o)  const

{
  VectorDouble countByProb   (numOfProbBuckets, 0.0);
  VectorDouble correctByProb (numOfProbBuckets, 0.0);

  double  totalRunningCount   = 0.0;
  double  totalRunningCorrect = 0.0;

  double  acc = 0.0;

  o << "<table align=\"center\" border=\"2\" cellpadding=\"3\" cellspacing=\"0\" frame=\"box\"  summary=\"Confusion \" >" << endl
    << "  <thead style=\"font-weight:bold; text-align:center; vertical-align:bottom\">"          << endl
    << "    <tr>"                                                                                << endl
    << "        <th>Class<br />Names</th><th>All<br />Classes</th>";

  for  (kkuint32 bucket = 0;  bucket < numOfProbBuckets;  bucket++)
  {
    o << "<th>" << ((bucket + 1) * probBucketSize) << "</th>";
  }
  o << "    </tr>"                                                                               << endl
    << "  </thead>"                                                                              << endl
    << "  <tbody style=\"font-weight:normal; text-align:right; font-family:Courier\">" << endl;

  KKStr  ln (1024);
  KKStr  accStr;

  for  (kkuint32 classNum = 0;  classNum < classCount;  classNum++)
  {
    double  countThisClass   = 0.0;
    double  correctThisClass = 0.0;  

    ln = "";

    for  (kkuint32 bucket = 0;  bucket < numOfProbBuckets;  bucket++)
    {
      double  count   = countByKnownClassByProb   [classNum][bucket];
      double  correct = correctByKnownClassByProb [classNum][bucket];

      countThisClass   += count;
      correctThisClass += correct;

      countByProb   [bucket] += countByKnownClassByProb   [classNum][bucket];
      correctByProb [bucket] += correctByKnownClassByProb [classNum][bucket];

      acc =0.0;
      accStr = "";
      if  (count != 0.0)
      {
        acc = 100.0 * correct / count;
        accStr = StrFormatDouble (acc, "ZZ0.000") + "%";
      }

      ln << "<td>" << accStr << "</td>";
    }

    totalRunningCount   += countThisClass;
    totalRunningCorrect += correctThisClass;

    accStr = "";
    acc = 0.0;
    if  (countThisClass != 0.0)
    {
      acc = 100.0 * correctThisClass / countThisClass;
      accStr = StrFormatDouble (acc, "ZZ0.000") + "%";
    }

    o << "    <tr>" 
      << "<td style=\"text-align:left; font-family:Arial\">" + classes[classNum].Name () + "</td>" 
      << "<td>" << accStr  << "</td>"
      << ln
      << "</tr>"
      << endl;
  }

  {
    acc = 0.0;
    if  (totalRunningCount != 0.0)
      acc = 100.0 * totalRunningCorrect / totalRunningCount;

    o << "    <tr>" 
      << "<td style=\"text-align:left; font-family:Arial\">" << "Total<br />All Classes"  << "</td>" 
      << "<td>" << StrFormatDouble (acc, "ZZ0.000") << "%" << "</td>";

    for  (kkuint32 bucket = 0;  bucket < numOfProbBuckets;  bucket++)
    {
      acc = 0.0;
      accStr = "";
      if  (countByProb [bucket] != 0.0)
      {
        acc = 100.0 * correctByProb [bucket] / countByProb [bucket];
        accStr = StrFormatDouble (acc, "ZZ0.000") + "%";
      }

      o << "<td>" << accStr << "</td>";
    }
    o << "</tr>" << endl;
  }

  o << "</tbody>" << endl
    << "</table>" << endl;
}  /* PrintAccuracyByProbByClassHTML */



void   ConfusionMatrix2::PrintConfusionMatrixTabDelimited (ostream&  outFile)  const
{
  // Lets generate Titles first
  outFile << endl;

  double  overallAccuracy = 0.0;
  if  (totalCount != 0.0)
   overallAccuracy = 100.0 * correctCount / totalCount;

  if  (numInvalidClassesPredicted > 0.0)
  {
    outFile << endl 
            << "*********************************************************************************************"  << endl
            << "*******************        WARNING    WARNING   WARNING  WARNING        *********************"  << endl
            << "*******************                                                     *********************"  << endl
            << "*******************   There were invalid classes specified that were    *********************"  << endl
            << "*******************   not counted.    numInvalidClassesPredicted[" << numInvalidClassesPredicted << "] *********************" << endl
            << "*********************************************************************************************"  << endl
            << endl;
  }

  outFile  << "Overall Accuracy:\t" 
           << StrFormatDouble (overallAccuracy, "ZZZ0.000") << "%"
           << endl;

  outFile  << endl;

  KKStr  titleLine1, titleLine2, titleLine3;
  classes.ExtractThreeTitleLines (titleLine1, titleLine2, titleLine3);

  outFile << ""            << "\t" << ""      << "\t" << titleLine1 << endl;
  outFile << ""            << "\t" << ""      << "\t" << titleLine2 << endl;
  outFile << "Class_Names" << "\t" << "Count" << "\t" << titleLine3 << endl;

  double*  totals = new double[classCount];
  for  (kkuint32 x = 0; x < classCount; x++)
    totals[x] = 0;

  double  totalNonNoise = 0;
  double  totalNonNoiseRight = 0;

  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    bool  noiseClass = classes[knownClassNum].UnDefined ();

    for  (kkuint32 predClassNum = 0; predClassNum < classCount; predClassNum++)
    {
      totals[predClassNum] += predictedCountsCM[knownClassNum] [predClassNum];
    }
     
    PrintSingleLineTabDelimited (outFile, 
                                 classes            [knownClassNum].Name (),
                                 countsByKnownClass [knownClassNum],
                                 predictedCountsCM  [knownClassNum]
                                );
    if  (!noiseClass)
    {
      totalNonNoise      = totalNonNoise      + countsByKnownClass [knownClassNum];
      totalNonNoiseRight = totalNonNoiseRight + predictedCountsCM  [knownClassNum] [knownClassNum];
    }
  }

  PrintSingleLineTabDelimited (outFile, 
                               KKStr ("Totals"),
                               totalCount,
                               totals
                              );

  outFile << endl << endl;

  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    PrintPercentLineTabDelimited (outFile, 
                                  classes            [knownClassNum].Name (),
                                  countsByKnownClass [knownClassNum],
                                  predictedCountsCM  [knownClassNum]
                                 );
  }

  double  nonNoiseAccuracy = 0.0;
  if  (totalNonNoise != 0)
     nonNoiseAccuracy = 100.0 * (double)totalNonNoiseRight / (double)totalNonNoise;

  outFile << endl
          << endl;

  outFile << "Non Noise Accuracy:" << "\t"  << StrFormatDouble (nonNoiseAccuracy, "ZZ0.000") << "%" << endl;


  outFile << endl << endl;

  delete  [] totals;
}  /* PrintConfusionMatrixTabDelimited */



void   ConfusionMatrix2::PrintLatexTableColumnHeaders (ostream&  outFile)  const
{
  outFile << "\\begin{tabular}{|";
  for  (kkuint32 x = 0;  x < (classCount + 2);  x++)
    outFile << "c|";
  outFile << "}" << endl;

  outFile << "\\hline" << endl;
  
  outFile << "Class Names" << " & " << "Count";

  for  (kkuint32 x = 0;  x < classCount;  x++)
  {
    outFile << " & " << StripOutInvalidLatexCaracters (classes[x].Name ());
  }
  outFile << "\\\\" << endl;


  outFile << "\\hline" << endl;
}  /* PrintConfusionMatrixLatexTableColumnHeaders */



void   ConfusionMatrix2::PrintConfusionMatrixLatexTable (ostream&  outFile)
{
  double  overallAccuracy = 0.0;
  if  (totalCount != 0.0)
    overallAccuracy = 100.0 * correctCount / totalCount;

  outFile  << "Overall Accuracy:\t" 
           << StrFormatDouble (overallAccuracy, "ZZZ0.000") << "\\%\\\\"
           << endl;

  PrintLatexTableColumnHeaders (outFile);
  

  double*  totals = new double[classCount];
  for  (kkuint32 x = 0; x < classCount; x++)
    totals[x] = 0;


  double  totalNonNoise = 0;
  double  totalNonNoiseRight = 0;

  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    bool  noiseClass = classes[knownClassNum].UnDefined ();

    for  (kkuint32 predClassNum = 0; predClassNum < classCount; predClassNum++)
    {
      totals[predClassNum] += predictedCountsCM[knownClassNum] [predClassNum];
    }
     
    PrintSingleLineLatexTable (outFile, 
                               knownClassNum,
                               classes            [knownClassNum].Name (),
                               countsByKnownClass [knownClassNum],
                               predictedCountsCM  [knownClassNum]
                              );

    outFile << "\\hline" << endl;

    if  (!noiseClass)
    {
      totalNonNoise      = totalNonNoise      + countsByKnownClass [knownClassNum];
      totalNonNoiseRight = totalNonNoiseRight + predictedCountsCM  [knownClassNum] [knownClassNum];
    }

  }

  outFile << "\\hline" << endl;

  PrintSingleLineLatexTable (outFile, 
                             int32_max,
                             KKStr ("Totals"),
                             totalCount,
                             totals
                            );

  outFile << "\\hline" << endl;

  outFile << "\\end{tabular}" << endl;


  outFile << endl
          << "\\vspace{16pt}" << endl
          << endl;


  PrintLatexTableColumnHeaders (outFile);
  
  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    PrintPercentLineLatexTable (outFile, 
                                knownClassNum,
                                classes            [knownClassNum].Name (),
                                countsByKnownClass [knownClassNum],
                                predictedCountsCM  [knownClassNum]
                               );
    outFile << "\\hline" << endl;
  }

  outFile << "\\end{tabular}" << endl;

  outFile << endl;

  delete[]  totals;
}  /* PrintConfusionMatrixLatexTable */



void   ConfusionMatrix2::PrintConfusionMatrixNarrow (ostream&  outFile)
{
  // Lets generate Titles first
  outFile << endl;

  double  perc = 0.0;
  if  (totalCount != 0)
    perc = correctCount / totalCount;

  outFile  << "Overall Accuracy is " 
           << setprecision (5)
           << (100.0 * perc) << "%"
           << endl;

  outFile  << endl;
   

  outFile << setw (25) << "Class Names";

  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    KKStr  colDesc ("Col");
    colDesc << (knownClassNum + 1);

    colDesc.LeftPad (6);

    outFile << colDesc;
  }
  outFile << endl;

  outFile << setw (25) << "===========";
  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    outFile << setw (6) << "====";
  }
  outFile << endl;

  double*  totals = new double[classCount];
  for  (kkuint32 x = 0; x < classCount; x++)
    totals[x] = 0;


  double  totalNonNoise = 0;
  double  totalNonNoiseRight = 0;

  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    bool  noiseClass = classes[knownClassNum].UnDefined ();

    for  (kkuint32 predClassNum = 0; predClassNum < classCount; predClassNum++)
    {
      totals[predClassNum] += predictedCountsCM [knownClassNum] [predClassNum];
    }
     
    PrintSingleLineShort (outFile, 
                          classes            [knownClassNum].Name (),
                          countsByKnownClass [knownClassNum],
                          predictedCountsCM  [knownClassNum]
                         );
    if  (!noiseClass)
    {
      totalNonNoise      += countsByKnownClass [knownClassNum];
      totalNonNoiseRight += predictedCountsCM  [knownClassNum] [knownClassNum];
    }
  }

  PrintSingleLineShort (outFile, 
                        KKStr ("Totals"),
                        totalCount,
                        totals
                       );

  outFile << endl << endl;

  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    PrintPercentLineShort (outFile, 
                           classes            [knownClassNum].Name (),
                           countsByKnownClass [knownClassNum],
                           predictedCountsCM  [knownClassNum]
                         );
  }

  outFile << endl
          << endl;


  perc = 0.0;
  if  (totalNonNoise != 0)
    perc = (double)totalNonNoiseRight / (double)totalNonNoise;

  outFile << "Accuracy for Non Noise "
          << setprecision (5)
          << (perc * 100.0)
          << "%"
          << endl;

  outFile << endl << endl;

  delete[]  totals;
}  /* PrintCrossValidationNarrow */



double  PercentOf (double x,  double y)
{
  double  total = x + y;
  if  (total == 0.0)
    return 0.0;
  else 
    return 100.0 * x / total;
}



void   ConfusionMatrix2::PrintTrueFalsePositivesTabDelimited (ostream&  r)  const
{
  kkuint32  numOfClasses = classes.QueueSize ();

  //  Refer to http://www.medcalc.be/manual/mpage06-13a.php for definitions.
  // First we calc TruePositives, FasePositives, TrueNegatives, FalseNegatives
  double*  falsePositives = new double[numOfClasses];
  double*  falseNegatives = new double[numOfClasses];
  double*  truePositives  = new double[numOfClasses];
  double*  trueNegatives  = new double[numOfClasses];

  double  totalTP = 0.0;
  double  totalFP = 0.0;
  double  totalTN = 0.0;
  double  totalFN = 0.0;

  for  (kkuint32 x = 0;  x < numOfClasses;  x++)
  {
    truePositives [x] = predictedCountsCM [x][x];
    totalTP += predictedCountsCM[x][x];

    trueNegatives [x] = 0.0;
    falsePositives[x] = 0.0;
    falseNegatives[x] = 0.0;

    for  (kkuint32 y = 0;  y < numOfClasses;  y++)
    {
      if  (y != x)
      {
        falsePositives[x] += predictedCountsCM [y][x];  // Was classified as x but was classed as x.
        totalFP           += predictedCountsCM [y][x];

        falseNegatives[x] += predictedCountsCM [x][y];  // Should have been classed as x not y.
        totalFN           += predictedCountsCM [x][y];

        trueNegatives [x] += (countsByKnownClass [y] - predictedCountsCM [y][x]);
        totalTN += (countsByKnownClass [y] - predictedCountsCM [y][x]);
      }
    }
  }

  KKStr  titleLine1, titleLine2;
  classes.ExtractTwoTitleLines (titleLine1, titleLine2);

  r << "\t" << ""         "\t" << titleLine1 << endl;
  r << "\t" << "Total" << "\t" << titleLine2 << endl;

  r << "TruePositives" << "\t" << totalTP;
  for  (kkuint32 x = 0;  x < numOfClasses;  x++)
  {
    r << "\t" << truePositives[x];
  }
  r << endl;

  r << "FalsePositives" << "\t" << totalFP;
  for  (kkuint32 x = 0;  x < numOfClasses;  x++)
  {
    r << "\t" << falsePositives[x];
  }
  r << endl;

  r << "TrueNegatives" << "\t" << totalTN;
  for  (kkuint32 x = 0;  x < numOfClasses;  x++)
  {
    r << "\t" << trueNegatives[x];
  }
  r << endl;

  r << "FalseNegatives"  << "\t" << totalFN;
  for  (kkuint32 x = 0;  x < numOfClasses;  x++)
  {
    r << "\t" << falseNegatives[x];
  }
  r << endl;

  r << endl;
  r << "Sensitivity(TP/(TP+FN))" << "\t" << StrFormatDouble(PercentOf (totalTP, totalFN), "zzz,zz0.00") << "%";
  for  (kkuint32 x = 0;  x < numOfClasses;  x++)
  {
    r << "\t" << StrFormatDouble(PercentOf (truePositives[x], falseNegatives[x]), "zzz,zz0.00") << "%";
  }
  r << endl;

  r << "Specificity(TN/(TN+FP))" << "\t" << StrFormatDouble(PercentOf (totalTN, totalFP), "zzz,zz0.00") << "%";
  for  (kkuint32 x = 0;  x < numOfClasses;  x++)
  {
    r << "\t" << StrFormatDouble(PercentOf (trueNegatives[x], falsePositives[x]), "zzz,zz0.00") << "%";
  }
  r << endl;

  r << "PositivePredictiveValue(TP/(TP+FP))" << "\t" << StrFormatDouble(PercentOf (totalTP, totalFP), "zzz,zz0.00") << "%";
  for  (kkuint32 x = 0;  x < numOfClasses;  x++)
  {
    r << "\t" << StrFormatDouble(PercentOf (truePositives[x], falsePositives[x]), "zzz,zz0.00") << "%";
  }
  r << endl;

  {
    double  fMeasure = 0.0;
    double  divisor = 2.0 * (double)totalTP + (double)totalFP + (double)totalFN;
    if  (divisor != 0.0)
      fMeasure = 100.0 * (2.0 * (double)totalTP / divisor);

    r << "F-Measure(2*TP/(2*TP + FP + FN))" << "\t" << StrFormatDouble(fMeasure, "zzz,zz0.00") << "%";
    for  (kkuint32 x = 0;  x < numOfClasses;  x++)
    {
      fMeasure = 0.0;
      divisor = 2.0 * (double)truePositives[x] + (double)falsePositives[x] + (double)falseNegatives[x];
      if  (divisor != 0.0)
        fMeasure = 100.0 * (2.0 * (double)truePositives[x] / divisor);

      r << "\t" << StrFormatDouble(fMeasure, "zzz,zz0.00") << "%";
    }
    r << endl;
  }

  delete[]  falseNegatives;
  delete[]  falsePositives;
  delete[]  trueNegatives;
  delete[]  truePositives;
}  /* PrintTrueFalsePositivesTabDelimited */



void   ConfusionMatrix2::ComputeFundamentalStats (MLClassPtr mlClass,
                                                  double&    truePositives,
                                                  double&    trueNegatives,
                                                  double&    falsePositives,
                                                  double&    falseNegatives
                                                 )
                                                 const
{
  truePositives  = 0.0;
  trueNegatives  = 0.0;
  falsePositives = 0.0;
  falseNegatives = 0.0;

  auto mlClassNum = classes.PtrToIdx (mlClass);
  if  (!mlClassNum)
    return;

  auto mlClassIdx = mlClassNum.value ();

  kkuint32 numOfClasses = classes.QueueSize ();

  truePositives  = predictedCountsCM [mlClassIdx][mlClassIdx];

  for  (kkuint32 y = 0;  y < numOfClasses;  y++)
  {
    if  (y != mlClassIdx)
    {
      falsePositives += predictedCountsCM [y][mlClassIdx];  // Was classified as x but was classed as x.
      falseNegatives += predictedCountsCM [mlClassIdx][y];  // Should have been classed as x not y.
      trueNegatives  += (countsByKnownClass [y] - predictedCountsCM [y][mlClassIdx]);
    }
  }
  return;
}  /* ComputeFundamentalStats */



float  ConfusionMatrix2::FMeasure (MLClassPtr  positiveClass,
                                   RunLog&     log
                                  )  const
{
  auto positiveNum = classes.PtrToIdx (positiveClass);
  if  (!positiveNum)
  {
    KKStr invalidClassName = "";
    if  (positiveClass)
       invalidClassName = positiveClass->Name ();

    log.Level (-1) << "ConfusionMatrix2::FMeasure         ***ERROR***      Invalid Positive Class Specified[" << invalidClassName << "]" << endl;
    return 0.0f;
  }

  auto positiveIDX = positiveNum.value ();

  kkuint32  numOfClasses = classes.QueueSize ();

  double  totalTP = 0.0;
  double  totalFP = 0.0;
  double  totalTN = 0.0;
  double  totalFN = 0.0;

  totalTP = predictedCountsCM[positiveIDX][positiveIDX];

  for  (kkuint32 y = 0;  y < numOfClasses;  y++)
  {
    if  (y != positiveIDX)
    {
      totalFP += predictedCountsCM[y][positiveIDX];
      totalTN += predictedCountsCM[y][y];
      totalFN += predictedCountsCM[positiveIDX][y];
    }
  }

  double  fMeasure = 0.0;
  double  divisor = 2.0 * (double)totalTP + (double)totalFP + (double)totalFN;
  if  (divisor != 0.0)
     fMeasure = 100.0 * (2.0 * (double)totalTP / divisor);

  return  (float)fMeasure;
}  /* FMeasure */



void   ConfusionMatrix2::PrintErrorBySize (ostream&  outFile)  const
{
  outFile << endl;

  outFile << "Size" << "\t";

  // Lets first Print Titles.
  for  (kkuint32 classNum = 0; classNum < classCount; classNum++)
  {
    outFile << "\t\t";
    outFile << classes[classNum].Name ();
  }
  outFile << endl;

  for  (kkuint32 bucket = 0;  bucket < numOfBuckets;  bucket++)
  {
    outFile << ((bucket + 1) * bucketSize) << "\t";

    for  (kkuint32 classNum = 0; classNum < classCount; classNum++)
    {
      outFile << countByKnownClassBySize   [classNum][bucket] << "\t"
              << correctByKnownClassBySize [classNum][bucket] << "\t";
    }

    outFile << endl;
  }

  outFile << endl;
}  /* PrintErrorBySize */



void   ConfusionMatrix2::PrintErrorByProb (ostream&  outFile)  const
{
  outFile << endl;

  outFile << "Prob" << "\t";

  // Lets first Print Titles.
  for  (kkuint32  classNum = 0; classNum < classCount; ++classNum)
  {
    outFile << "\t\t";
    outFile << classes[classNum].Name ();
  }

  outFile << endl;

  outFile << setiosflags (ios::fixed);

  for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    outFile << ((bucket + 1) * probBucketSize) << "%\t";

    for  (kkuint32 classNum = 0;  classNum < classCount;  ++classNum)
    {
      double  perc;

      double count   = countByKnownClassByProb   [classNum][bucket];
      double correct = correctByKnownClassByProb [classNum][bucket];
      
      if  (count > 0)
         perc = 100.0 * (double)correct / (double)count;
      else
         perc = 0.0;

      outFile << count << "\t" << correct << "\t" << setprecision (1) << perc << "% \t";
    }

    outFile << endl;
  }

  outFile << endl;
}  /* PrintErrorByProb */



void   ConfusionMatrix2::PrintErrorBySizeByRows (ostream&  outFile)  const
{
  outFile  << endl
           << "Error by size" << endl
           << endl;

  outFile  << "ClassName\tAvg Size\t";
  for  (kkuint32 bucket = 0; bucket < numOfBuckets; bucket++)
  {
    outFile << ((bucket + 1) * bucketSize) << "\t";
  }

  outFile << endl
          << endl;

  for  (kkuint32 classNum = 0; classNum < classCount; classNum++)
  {
    double  avg;
    if  (countsByKnownClass [classNum] != 0)
      avg = totalSizesByKnownClass[classNum] / countsByKnownClass [classNum];
    else
      avg = 0;

    outFile << classes[classNum].Name ()
            << "\t"
            <<  avg;


    for  (kkuint32 bucket = 0; bucket < numOfBuckets; bucket++)
    {
      outFile << "\t" << countByKnownClassBySize[classNum][bucket];
    }
    outFile << endl;

    
    outFile << classes[classNum].Name () << " Correct" << "\t";
    for  (kkuint32 bucket = 0; bucket < numOfBuckets; bucket++)
    {
      outFile << "\t" << correctByKnownClassBySize [classNum][bucket];
    }

    outFile << endl;
    outFile << endl;
  }
  outFile << endl
          << endl
          << endl;

  outFile  << "ClassName\tAvg Size\t";
  for  (kkuint32 bucket = 0; bucket < numOfBuckets; bucket++)
  {
    outFile << ((bucket + 1) * bucketSize) << "\t";
  }

  outFile << endl
          << endl;

  for  (kkuint32 classNum = 0; classNum < classCount; classNum++)
  {
    double  avg;
    if  (countsByKnownClass [classNum] != 0)
      avg = totalSizesByKnownClass[classNum] / countsByKnownClass [classNum];
    else
      avg = 0;

    outFile << classes[classNum].Name ()
            << "\t"
            <<  avg;


    for  (kkuint32 bucket = 0; bucket < numOfBuckets; bucket++)
    {
      float  a = 0.0f;
      if  (countByKnownClassBySize[classNum][bucket] != 0)
        a = (float)(correctByKnownClassBySize [classNum][bucket]) / (float)(countByKnownClassBySize[classNum][bucket]);

      outFile << "\t" << a;
    }
    outFile << endl;

    outFile << endl;
    outFile << endl;
  }

  outFile << endl;
}  /* PrintErrorBySizeByRows */



void   ConfusionMatrix2::PrintErrorByProbByRows (ostream&  outFile)  const
{
  double*  totalCountByProb   = new double[numOfProbBuckets];
  double*  totalCorrectByProb = new double[numOfProbBuckets];

  for  (kkuint32 x = 0; x < numOfProbBuckets; x++)
  {
    totalCountByProb  [x] = 0;
    totalCorrectByProb[x] = 0;
  }

  outFile  << "ClassName\tAvg Prob\t";
  for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    outFile << ((bucket + 1) * probBucketSize) << "%\t";
  }

  outFile << endl
          << endl;

  for  (kkuint32 classNum = 0; classNum < classCount; classNum++)
  {
    double  avg;
    if  (countsByKnownClass [classNum] != 0)
      avg = 100.0 * totalPredProbsByKnownClass [classNum] / countsByKnownClass [classNum];
    else
      avg = 0;

    outFile << classes[classNum].Name ()
            << "\t"
            <<  avg << "%";


    for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
    {
      outFile << "\t" << countByKnownClassByProb [classNum][bucket];
      totalCountByProb[bucket] = totalCountByProb[bucket] + countByKnownClassByProb [classNum][bucket];
    }
    outFile << endl;

    outFile << classes[classNum].Name () << " Correct" << "\t";
    for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
    {
      outFile << "\t" << correctByKnownClassByProb [classNum][bucket];
      totalCorrectByProb[bucket] = totalCorrectByProb[bucket] + correctByKnownClassByProb [classNum][bucket];
    }
    outFile << endl;


    outFile << "Accuracy"  << "\t";
    for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
    {
      double  perc;

      if  (countByKnownClassByProb [classNum][bucket] <= 0.0)
        perc = 0.0;
      else
        perc = 100.0 * correctByKnownClassByProb [classNum][bucket] / countByKnownClassByProb [classNum][bucket];

      outFile << "\t" << perc << "%";
    }
    outFile << endl;

    outFile << endl;
  }

  outFile << endl;

  outFile << "Total" << "\t";
  for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    outFile << "\t" << totalCountByProb[bucket];
  }

  outFile << endl;

  outFile << "Correct" << "\t";
  for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    outFile << "\t" << totalCorrectByProb[bucket];
  }
  outFile << endl;

  outFile << "Accuracy" << "\t";
  for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    double  perc;

    if  (totalCountByProb[bucket] <= 0)
      perc = 0.0;
    else
      perc = 100.0 * totalCorrectByProb[bucket] / totalCountByProb[bucket];

    outFile << "\t" << perc << "%";
  }

  delete[]  totalCountByProb;   totalCountByProb   = NULL;
  delete[]  totalCorrectByProb; totalCorrectByProb = NULL;

  outFile << endl;

  return;
}  /* PrintErrorByProbByRows */



void  ConfusionMatrix2::PrintProbDistributionTitle (ostream&  outFile)
{
  outFile << "Total";
  for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    outFile << "\t" << ((bucket + 1) * probBucketSize) << "%";
  }

  outFile << endl;
}  /* PrintPronDistributionTitle */



void   ConfusionMatrix2::PrintProbDistributionTotalCount (ostream&  outFile)
{
  double*  count   = new double[numOfProbBuckets];

  for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    count [bucket] = 0;
  }


  double  total = 0;

  for  (kkuint32 classNum = 0; classNum < classCount; classNum++)
  {
    for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
    {
      count[bucket] = count[bucket] + countByKnownClassByProb [classNum][bucket];
    }
  }


  for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    total = total + count[bucket];
  }


  outFile << total;
  for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    outFile << "\t" << count[bucket];
  }

  outFile << endl;

  delete[]  count;

  return;
}  /* PrintProbDistributionTotalCount */



void   ConfusionMatrix2::PrintProbDistributionTotalError (ostream&  outFile)
{
  double*  count   = new double[numOfProbBuckets];
  double*  correct = new double[numOfProbBuckets];

  for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    count   [bucket] = 0;
    correct [bucket] = 0;
  }
  
  for  (kkuint32 classNum = 0; classNum < classCount; classNum++)
  {
    for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
    {
      count  [bucket] = count  [bucket] + countByKnownClassByProb   [classNum][bucket];
      correct[bucket] = correct[bucket] + correctByKnownClassByProb [classNum][bucket];
    }
  }

  double  overallAccuracy = 0.0;
  
  {
    double  totalBucketCount = 0;
    double  totalCorrect = 0;

    for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
    {
      totalBucketCount = totalBucketCount + count[bucket];
      totalCorrect     = totalCorrect     + correct[bucket];
    }

    if  (totalBucketCount == 0.0)
    {
      overallAccuracy = 0.0;
    }
    else
    {
      overallAccuracy = 100.0 * totalCorrect / totalBucketCount;
    }
  }

  outFile << setprecision (2);

  outFile << overallAccuracy << "%";

  for  (kkuint32 bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    double  accuracy = 0.0;
    
    if  (count[bucket] > 0)
       accuracy = 100.0 * (double)correct[bucket] / (double)count[bucket];

    outFile << "\t" << accuracy << "%";
  }

  outFile << endl;

  delete[]  count;    count   = NULL;
  delete[]  correct;  correct = NULL;
}  /* PrintProbDistributionTotalError */



void   ConfusionMatrix2::PrintErrorBySizeReduced (ostream&  outFile)  const
{
  outFile << endl;
  outFile << endl;
  outFile << endl;

  for  (kkuint32 classNum = 0; classNum < classCount; classNum++)
     PrintErrorBySizeRowReduced (outFile, classNum);

}  /* PrintErrorBySizeReduced */



void   ConfusionMatrix2::PrintErrorBySizeRowReduced (ostream&  outFile,
                                                     kkuint32  classNum
                                                    )  const
{
  kkint32*   bucketHeadings = new kkint32 [numOfBuckets];
  double*  bucketCount    = new double[numOfBuckets];
  double*  bucketCorrect  = new double[numOfBuckets];

  kkuint32 bucket = 0;

  kkuint32 numNewBuckets = 0;

  while  (bucket < numOfBuckets)
  {
    double   countInCol        = 0.0;
    double   countCorrectInCol = 0.0;

    while  ((bucket < numOfBuckets)  &&  (countInCol < 20))
    {
      countInCol        = countInCol        + countByKnownClassBySize   [classNum][bucket];
      countCorrectInCol = countCorrectInCol + correctByKnownClassBySize [classNum][bucket];
      bucket++;
    }

    bucketHeadings[numNewBuckets] = bucket * bucketSize;
    bucketCount[numNewBuckets]    = countInCol;
    bucketCorrect[numNewBuckets]  = countCorrectInCol;
    numNewBuckets++;
  }

  outFile << "Class_Name\tAvg_Size";
  for  (bucket = 0;  bucket < numNewBuckets; bucket++)
  {
    outFile << "\t" << bucketHeadings[bucket];
  }
  outFile << endl;

  double  avg;
  if  (countsByKnownClass [classNum] != 0)
    avg = totalSizesByKnownClass [classNum] / countsByKnownClass [classNum];
  else
    avg = 0;
  
  outFile << classes[classNum].Name () << "\t" <<  avg;

  for  (bucket = 0;  bucket < numNewBuckets; bucket++)
  {
    outFile << "\t" << bucketCount[bucket];
  }
  outFile << endl;
  
  outFile << "\t";
  for  (bucket = 0;  bucket < numNewBuckets; bucket++)
  {
    outFile << "\t" << bucketCorrect[bucket];
  }
  outFile << endl;

  outFile << "\t";
  for  (bucket = 0;  bucket < numNewBuckets; bucket++)
  {
    double  accuracy = 0;
    if  (bucketCount[bucket] <= 0)
    {
      accuracy = 0.0;
    }
    else
    {
      accuracy = 100.0 * (double)bucketCorrect[bucket] / (double)bucketCount[bucket];
    }

    outFile << "\t" << accuracy << "%";
  }

  outFile << endl;
  outFile << endl;
  outFile << endl;

  delete[]  bucketHeadings;
  delete[]  bucketCount;
  delete[]  bucketCorrect;
}  /* PrintErrorBySizeRowReduced */



KKStr   ConfusionMatrix2::AccuracyStr ()
{
  double*  accuracys = new double[classCount];

  for  (kkuint32 x = 0;  x < classCount;  x++)
  {
    if  (countsByKnownClass [x] == 0)
      accuracys[x] = 0;
    else
      accuracys[x] = (100.0 * (double) predictedCountsCM [x] [x]) / ((double) (countsByKnownClass [x]));
  }

  KKStr  accuracyStr;

  for  (kkuint32 x = 0; x < classCount; x++)
  {
    if  (x > 0)
      accuracyStr << "  ";

    KKStr  className;
    MLClassPtr  mlClass = classes.IdxToPtr (x);
    if  (mlClass)
       className = mlClass->Name ();
    else
       className = "***UnDefined***";

    accuracyStr << className << " " << StrFormatDouble (accuracys[x], "##0.000") << "%";
  }

  delete[]  accuracys;  accuracys = NULL;

  return  accuracyStr;
}  /* AccuracyStr */



double  ConfusionMatrix2::Accuracy ()  const
{
  if  (totalCount == 0)
    return 0.0;
  else
    return  100.0 * correctCount / totalCount;
}



double  ConfusionMatrix2::AvgPredProb ()  const
{
  if  (totalCount == 0)
    return 0.0;

  return  totalPredProb / totalCount;
}



double  ConfusionMatrix2::Accuracy (MLClassPtr  mlClass)  const
{
  auto classNum = classes.PtrToIdx (mlClass);
  if  (!classNum)
    return 0.0f;

  auto classIdx = classNum.value ();

  float  accuracy = (float)(100.0 * (predictedCountsCM[classIdx] [classIdx])  /  (countsByKnownClass [classIdx]));

  return  accuracy;
}  /* Accuracy */



VectorFloat ConfusionMatrix2::AccuracyByClass  ()  const
{
  VectorFloat  accuracies;
  for  (kkuint32  classNum = 0;  classNum < classCount;  classNum++)
  {
    if  (countsByKnownClass [classNum] == 0)
    {
      accuracies.push_back (0.0f);
    }
    else
    {
      float  classAccuracy = (float)(100.0f * (predictedCountsCM[classNum] [classNum])  / (countsByKnownClass [classNum]));
      accuracies.push_back (classAccuracy);
    }
  }

  return  accuracies;
}  /* AccuracyByClass */



float  ConfusionMatrix2::AccuracyClassWeightedEqually ()  const
{
  kkuint32  classSize = classes.QueueSize ();
  float  totalAccuracy = 0.0f;

  for  (kkuint32  classNum = 0;  classNum < classSize;  classNum++)
  {
    if  (countsByKnownClass [classNum] != 0)
    {
      float  classAccuracy = (float)(100.0f * (predictedCountsCM[classNum] [classNum])  / (countsByKnownClass [classNum]));
      totalAccuracy += classAccuracy;
    }
  }

  float  weightedAccuracy = (totalAccuracy / (float)classSize);

  return  weightedAccuracy;
}  /* AccuracyClassWeightedEqually */



double  ConfusionMatrix2::Count (MLClassPtr  mlClass)
{
  kkuint32  classNum   = 0;
  bool   found  = false;
  kkuint32  numClasses = classes.QueueSize ();

  while  ((classNum < numClasses)  &&  (!found))
  {
    if  (classes[classNum].UpperName () == mlClass->UpperName ())
      found = true;
    else
      classNum++;
  }


  if  (found)
  {
    return  countsByKnownClass [classNum];
  }

  return  0.0;
}  /* Count */



void  ConfusionMatrix2::FactorCounts (double  factor)
{
  correctCount               *= factor;
  totalCount                 *= factor;
  totalPredProb              *= factor;
  numInvalidClassesPredicted *= factor;

  for  (kkuint32 x = 0; x < classCount;  x++)
  {
    countsByKnownClass         [x] = countsByKnownClass         [x] * factor;
    totalSizesByKnownClass     [x] = totalSizesByKnownClass     [x] * factor;
    totalPredProbsByKnownClass [x] = totalPredProbsByKnownClass [x] * factor;

    for  (kkuint32 y = 0; y < classCount; y++)
    {
      predictedCountsCM[x][y] = predictedCountsCM[x][y] * factor;
      totPredProbCM    [x][y] = totPredProbCM    [x][y] * factor;
    }

    for  (kkuint32 y = 0; y < numOfBuckets; y++)
    {
      countByKnownClassBySize   [x][y] = countByKnownClassBySize   [x][y] * factor;
      correctByKnownClassBySize [x][y] = correctByKnownClassBySize [x][y] * factor; 
    }

    for  (kkuint32 y = 0; y < numOfProbBuckets; y++)
    {
      countByKnownClassByProb   [x][y] = countByKnownClassByProb   [x][y] * factor;
      correctByKnownClassByProb [x][y] = correctByKnownClassByProb [x][y] * factor;
    }
  }
}  /* FactorCounts */



/******************************************************************************
 * PrintConfusionMatrixHTML
 ******************************************************************************/
void ConfusionMatrix2::PrintConfusionMatrixHTML (const char *title, 
                                                 ostream&   file
                                                )
{
  // generate html preamble
  file << "<html>" << endl;
  file << "<head>" << endl;
  file << "<title>" << title << "</title>" << endl;
  file << "<body bgcolor=\"white\">" << endl;

  // generate the title
  file  << "<h1>" << title << "</h1>" << endl;

  // generate the accuracy statement
  file  << "<p><b>Overall Accuracy</b> is " 
      << setprecision (5)
      << (100.0 * correctCount / totalCount) << "%"
      << "</p>" << endl;

  /***************************************************************************
   * generate the table with the counts
   ***************************************************************************/
  file << "<table cellpadding=\"2\" cellspacing=\"0\" border=\"2\">" << endl;
  file << "<tr>" << endl;

  // output the first row (which is class names)
  file << "<th align=\"center\" bgcolor=\"#CCCCCC\"><b>Class Names</b></th>" << endl;
  file << "<th align=\"center\" bgcolor=\"#CCCCCC\"><b>Totals</b></th>" << endl;
  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  ++knownClassNum)
  {
    file << "<th align=\"center\" bgcolor=\"#CCCCCC\"><b>" << classes[knownClassNum].Name() << "</b></th>" << endl;
  }
  file  << "</tr>" << endl;

  double *totals = new double[classCount];
  for  (kkuint32 x = 0; x < classCount;  ++x)
  {
    totals[x] = 0;
  }

  double  totalNonNoise = 0;
  double  totalNonNoiseRight = 0;

  // output the data rows
  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  ++knownClassNum)
  {
    bool  noiseClass = classes[knownClassNum].UnDefined();
    for  (kkuint32 predClassNum = 0;  predClassNum < classCount;  ++predClassNum)
    {
      totals[predClassNum] += predictedCountsCM[knownClassNum] [predClassNum];
    }
     
    file << "<tr>" << endl;
    file << "<th align=\"center\" bgcolor=\"#CCCCCC\"><b>" << classes            [knownClassNum].Name() << "</b></th>" << endl;
    file << "<td align=\"center\" bgcolor=\"#EFEFEF\">"    << countsByKnownClass [knownClassNum]        << "</td>"     << endl;

    for  (kkuint32 predClassNum = 0;  predClassNum < classCount;  ++predClassNum)
    {
      if (predClassNum == knownClassNum)
        file << "<td align=\"center\" bgcolor=\"#EEEEEE\">";
      else 
        file << "<td align=\"center\">";
      file << predictedCountsCM[knownClassNum][predClassNum]; 
      file << "</td>" << endl;
    }
    file << "</tr>" << endl;
    if  (!noiseClass)
    {
      totalNonNoise = totalNonNoise + countsByKnownClass [knownClassNum];
      totalNonNoiseRight += predictedCountsCM [knownClassNum] [knownClassNum];
    }
  }

  // output the totals line for the first table
  file << "<tr>" << endl;
  file << "<th align=\"center\" bgcolor=\"#CCCCCC\"><b>Totals</b></th>" << endl;
  file << "<td align=\"center\" bgcolor=\"#EEEEEE\">" << totalCount << "</b></th>" << endl;
  
  for  (kkuint32 predClassNum = 0;  predClassNum < classCount;  ++predClassNum)
  {
    file << "<td align=\"center\">";
    file << totals[predClassNum]; 
    file << "</td>" << endl;
  }
  file << "</tr>" << endl;
  file << "</table>" << endl;

  /***************************************************************************
   * generate the table with the percents
   ***************************************************************************/
  file  << "<br/>" << endl;
  file  << "<p><b>Accuracy for Non Noise</b> "
          << setprecision (5)
          << (((double)totalNonNoiseRight / (double)totalNonNoise) * 100.0)
          << "%</p>"
          << endl;
  file  << "<table cellpadding=\"2\" cellspacing=\"0\" border=\"2\">" << endl;
  file  << "<tr>" << endl;

  // output the first row (which is class names)
  file << "<th align=\"center\" bgcolor=\"#CCCCCC\"><b>Class Names</b></th>" << endl;
  file << "<th align=\"center\" bgcolor=\"#CCCCCC\"><b>Totals</b></th>" << endl;
  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  ++knownClassNum)
  {
    file << "<th align=\"center\" bgcolor=\"#CCCCCC\"><b>" << classes[knownClassNum].Name() << "</b></th>" << endl;
  }
  file  << "</tr>" << endl;

  // output the data rows
  double perc=0.0;
  for  (kkuint32 knownClassNum = 0;  knownClassNum < classCount;  ++knownClassNum)
  {
    file << "<tr>" << endl;
    file << "<th align=\"center\" bgcolor=\"#CCCCCC\"><b>" << classes[knownClassNum].Name() << "</b></th>" << endl;
    file << "<td align=\"center\" bgcolor=\"#EFEFEF\">" << setprecision (4) << (countsByKnownClass [knownClassNum] / totalCount * 100) << "</td>" << endl;

    for  (kkuint32 predClassNum = 0;  predClassNum < classCount;  ++predClassNum)
    {
      if (predClassNum == knownClassNum)
        file << "<td align=\"center\" bgcolor=\"#EEEEEE\">";
      else 
        file << "<td align=\"center\">";
      if  (countsByKnownClass [knownClassNum] <= 0)
        perc = 0.0;
      else
        perc = predictedCountsCM [knownClassNum][predClassNum] / countsByKnownClass [knownClassNum] * 100.0;

      file << setprecision (4) << perc; 
      file << "</td>" << endl;
    }
    file << "</tr>" << endl;
  }
  file << "</table>" << endl;

  file << "</body>" << endl;
  file << "</html>" << endl;

  delete[]  totals;
}   /* PrintConfusionMatrixHTML */



void  ConfusionMatrix2::MakeSureWeHaveTheseClasses (const MLClassList&  classList,
                                                    RunLog&                     log
                                                   )
{
  MLClassList::const_iterator  idx;
  for  (idx = classList.begin ();  idx != classList.end ();  ++idx)
  {
    MLClassPtr ic = *idx;
    if  (!classes.PtrToIdx (ic))
      AddClassToConfusionMatrix (ic, log);
  }
}  /* MakeSureWeHaveTheseClasses */



void   ConfusionMatrix2::AddIn (const ConfusionMatrix2&  cm,
                                RunLog&                  log
                               )
{
  MakeSureWeHaveTheseClasses (cm.classes, log);

  kkuint32  numOfClasses = classes.QueueSize ();
  kkuint32  classIDX = 0;
  
  //  Create indirection array to handle the situation where the mlClass list's of the two 
  // confusion matrixes '*this'  and  'cm'  are not in the same order.

  VectorOptionUInt32  ind (numOfClasses, 0);
  for  (classIDX = 0;  classIDX < numOfClasses;  ++classIDX)
  {
    MLClassPtr mlClass = classes.IdxToPtr (classIDX);
    ind[classIDX]  = cm.classes.PtrToIdx (mlClass);
  }

  for  (classIDX = 0;  classIDX < numOfClasses;  ++classIDX)
  {
    auto cmsIdxValue = ind[classIDX];
    if  (!cmsIdxValue)
    {
      // !cmsIDX  indicates that the confusion matrix being added in does not include the class indicatd by 'classIDX'.
    }
    else
    {
      auto cmsIdx = cmsIdxValue.value ();
      countsByKnownClass         [classIDX] += cm.countsByKnownClass         [cmsIdx];
      totalSizesByKnownClass     [classIDX] += cm.totalSizesByKnownClass     [cmsIdx];
      totalPredProbsByKnownClass [classIDX] += cm.totalPredProbsByKnownClass [cmsIdx];

      for  (kkuint32 predictedClassIDX = 0;   predictedClassIDX  < numOfClasses;  ++predictedClassIDX)
      {
        auto  cmsPredictedValue = ind[predictedClassIDX];
        if  (cmsPredictedValue)
        {
           auto cmsPredictedIdx = cmsPredictedValue.value ();  
           predictedCountsCM[classIDX][cmsPredictedIdx] += cm.predictedCountsCM[cmsIdx][cmsPredictedIdx];
           totPredProbCM    [classIDX][cmsPredictedIdx] += cm.totPredProbCM    [cmsIdx][cmsPredictedIdx];
        }
      }
      
      for  (kkuint32 bucketIDX = 0;  bucketIDX < numOfBuckets;  ++bucketIDX)
      {
        countByKnownClassBySize   [classIDX][bucketIDX] += cm.countByKnownClassBySize   [cmsIdx][bucketIDX];
        correctByKnownClassBySize [classIDX][bucketIDX] += cm.correctByKnownClassBySize [cmsIdx][bucketIDX];
      }

      for  (kkuint32 probIDX = 0;  probIDX < numOfProbBuckets;  probIDX++)
      {
        countByKnownClassByProb   [classIDX][probIDX] += cm.countByKnownClassByProb   [cmsIdx][probIDX];
        correctByKnownClassByProb [classIDX][probIDX] += cm.correctByKnownClassByProb [cmsIdx][probIDX];
      }
    }
  }

  correctCount  += cm.correctCount;
  totalCount    += cm.totalCount;
  totalPredProb += cm.totalPredProb;
}  /* AddIn */



template<typename T>
KKStr  ArrayToDelimitedDelimitedStr (T*        _array,  
                                     kkuint32  _count,
                                     char      _delimiter
                                    )
{
  KKStr s (_count * 10);
  for  (kkuint32 x = 0;  x < _count;  x++)
  {
    if  (x > 0)
      s.Append (_delimiter);
    s << _array[x];
  }
  return s;
}  /* ArrayToDelimitedDelimitedStr */



template<typename T>
KKStr  ArrayToDelimitedDelimitedStr (const vector<T>&   v,
                                     char               delimiter
                                    )
{
  KKStr s ((kkint32)v.size () * 10);

  for  (kkuint32 x = 0;  x < v.size ();  ++x)
  {
    if  (x > 0)  s.Append (delimiter);
    s << v[x];
  }
  return s;
}  /* ArrayToDelimitedDelimitedStr */



void   DelimitedStrToArray (vector<kkint32>&  v,
                            kkuint32          minSize,
                            const KKStr&      l,
                            char              delimiter
                           )
{
  v.clear ();
  VectorKKStr  fields = l.Split (delimiter);
  kkuint32 lastField = (kkuint32)fields.size ();
  for  (kkuint32 idx = 0;  idx < lastField;  ++idx)
    v.push_back (fields[idx].ToInt32 ());
  while  (v.size () < minSize)
    v.push_back ((kkint32)0);
}  /* DelimitedStrToArray */



void   DelimitedStrToArray (vector<double>&  v,
                            kkuint32         minSize,
                            const KKStr&     l,
                            char             delimiter
                           )
{
  v.clear ();
  VectorKKStr  fields = l.Split (delimiter);
  kkuint32 lastField = (kkuint32)fields.size ();
  for  (kkuint32 idx = 0;  idx < lastField;  idx++)
    v.push_back (fields[idx].ToDouble ());
  while  (v.size () < minSize)
    v.push_back ((double)0);
}  /* DelimitedStrToArray */



void   DelimitedStrToArray (kkint32*      _array,
                            kkuint32      _count, 
                            const KKStr&  _l,
                            char          _delimiter
                           )
{
  VectorKKStr  fields = _l.Split (_delimiter);
  kkuint32  lastField = Min ((kkuint32)fields.size (), _count);
  for  (kkuint32 idx = 0;  idx < lastField;  ++idx)
    _array[idx] = fields[idx].ToInt ();
}  /* DelimitedStrToArray */



void   DelimitedStrToArray (double*        _array,
                            kkuint32       _count, 
                            const KKStr&   _l,
                            char           _delimiter
                           )
{
  VectorKKStr  fields = _l.Split (_delimiter);
  kkuint32  lastField = Min ((kkuint32)fields.size (), _count);
  for  (kkuint32 idx = 0;  idx < lastField;  ++idx)
    _array[idx] = fields[idx].ToDouble ();
}  /* DelimitedStrToArray */



void  ConfusionMatrix2::WriteXML (ostream&  f)  const
{
  f << "<ConfusionMatrix2>" << endl;

  f << "Classes"                     << "\t" << classes.ToCommaDelimitedStr () << endl;

  f << "ClassCount"                  << "\t" << classCount       << endl
    << "BucketSize"                  << "\t" << bucketSize       << endl
    << "probBucketSize"              << "\t" << probBucketSize   << endl
    << "NumOfBuckets"                << "\t" << numOfBuckets     << endl
    << "NumOfProbBuckets"            << "\t" << numOfProbBuckets << endl
    << endl
    << "TotalCount"                  << "\t" << totalCount                  << endl
    << "CorrectCount"                << "\t" << correctCount                << endl
    << "TotalPredProb"               << "\t" << totalPredProb               << endl
    << "NumInvalidClassesPredicted"  << "\t" << numInvalidClassesPredicted  << endl
    << endl;

  f << "CountsByKnownClass"          << "\t" << ArrayToDelimitedDelimitedStr (countsByKnownClass,          ',') << endl;
  f << "TotalSizesByKnownClass"      << "\t" << ArrayToDelimitedDelimitedStr (totalSizesByKnownClass,      ',') << endl;
  f << "TotalPredProbsByKnownClass"  << "\t" << ArrayToDelimitedDelimitedStr (totalPredProbsByKnownClass,  ',') << endl;


  kkint32  classIndex = 0;
  MLClassList::const_iterator  idx;
  for  (idx = classes.begin ();  idx != classes.end ();  idx++)
  {
    MLClassPtr  mlClass = *idx;
    f << "ClassTotals"  << "\t"  << "ClassName" << "\t" << mlClass->Name ().QuotedStr  () << "\t" << "ClassIndex" << "\t" << classIndex << endl;

    f << "CountByKnownClassBySize"    << "\t" << ArrayToDelimitedDelimitedStr (countByKnownClassBySize    [classIndex], numOfBuckets,     ',') << endl;
    f << "CorrectByKnownClassBySize"  << "\t" << ArrayToDelimitedDelimitedStr (correctByKnownClassBySize  [classIndex], numOfBuckets,     ',') << endl;
    
    f << "CountByKnownClassByProb"    << "\t" << ArrayToDelimitedDelimitedStr (countByKnownClassByProb    [classIndex], numOfProbBuckets, ',') << endl;
    f << "CorrectByKnownClassByProb"  << "\t" << ArrayToDelimitedDelimitedStr (correctByKnownClassByProb  [classIndex], numOfProbBuckets, ',') << endl;

    f << "PredictedCountsCM"          << "\t" << ArrayToDelimitedDelimitedStr (predictedCountsCM          [classIndex], classCount,       ',') << endl;

    f << "TotPredProbCM"              << "\t" << ArrayToDelimitedDelimitedStr (totPredProbCM              [classIndex], classCount,       ',') << endl;

    classIndex++;
  }

  f << "</ConfusionMatrix2>" << endl;
}  /* Write */



ConfusionMatrix2Ptr  ConfusionMatrix2::BuildFromIstreamXML (istream&  f,
                                                            RunLog&   log
                                                           )
{
  if  (f.eof ())
  {
    log.Level (-1) << endl << "ConfusionMatrix2::BuildFromIstreamXML  ***ERROR***   File already at EOF." << endl << endl;
    return  NULL;
  }

  char  buff[10240];
  buff[0] = 0;

  kkint64   startPos = f.tellg ();
  MLClassListPtr  classes = NULL;

  kkuint32  bucketSize        = 0;
  kkuint32  classCount        = 0;
  kkuint32  numOfBuckets      = 0;
  kkuint32  numOfProbBuckets  = 0;
  kkuint32  probBucketSize    = 0;

  f.getline (buff, sizeof (buff));
  while  ((!f.eof ())  &&  ((!classes)              ||  (bucketSize < 1)      ||  (numOfBuckets < 1)  ||  
                            (numOfProbBuckets < 1)  ||  (probBucketSize < 1)  ||  (classCount < 1)))
  {
    KKStr  l (buff);
    l.TrimLeft ();

    if  (l.CompareIgnoreCase ("</ConfusionMatrix2>") == 0)
      break;

    KKStr  lineName = l.ExtractToken2 ("\t");
    if  (lineName.CompareIgnoreCase ("Classes") == 0)
      classes = MLClassList::BuildListFromDelimtedStr (l, '\t');

    else if  (lineName.CompareIgnoreCase ("bucketSize") == 0)
      bucketSize = l.ExtractTokenUint ("\t\n\r");

    else if  (lineName.CompareIgnoreCase ("classCount") == 0)
      classCount = l.ExtractTokenUint ("\t\n\r");

    else if  (lineName.CompareIgnoreCase ("numOfBuckets") == 0)
      numOfBuckets = l.ExtractTokenUint ("\t\n\r");

    else if  (lineName.CompareIgnoreCase ("numOfProbBuckets") == 0)
      numOfProbBuckets = l.ExtractTokenUint ("\t\n\r");

    else if  (lineName.CompareIgnoreCase ("probBucketSize") == 0)
      probBucketSize = l.ExtractTokenUint ("\t\n\r");

    f.getline (buff, sizeof (buff));
  }

  if  (classes == NULL)
  {
    log.Level (-1) << endl 
                   << "ConfusionMatrix2::BuildFromIstreamXML     ***ERROR***    No Class List Was Provided."  << endl
                   << endl;
    // Failed to locate ClassList ('classes')  we an not build a ConfusionMatrixc2 object.
    return NULL;
  }

  if  ((bucketSize < 1)  ||  (numOfBuckets < 1)  ||  (numOfProbBuckets < 1)  ||  (probBucketSize < 1))
  {
    delete  classes;  classes = NULL;
    log.Level (-1) << endl 
                   << "ConfusionMatrix2::BuildFromIstreamXML     ***ERROR***    Not all needed header fields were defined."  << endl
                   << "  bucketSize[" << bucketSize << "]  ClassCount[" <<  classCount  <<  "]  numOfBuckets[" << numOfBuckets << "]  numOfProbBuckets[" << numOfProbBuckets << "]  probBucketSize[" << probBucketSize << "]" << endl
                   << endl;
    // Failed to locate ClassList ('classes')  we an not build a ConfusionMatrixc2 object.
    return NULL;
  }
  
  if  (classCount != classes->QueueSize ())
  {
    log.Level (-1) << endl 
                   << "ConfusionMatrix2::BuildFromIstreamXML    ***ERROR***    Disagreement between ClassCount[" << classCount << "]  and  Classes.QueueSize[" << classes->QueueSize () << "]" << endl
                   << endl;
    delete  classes;  classes = NULL;
    return NULL;
  }

  f.seekg (startPos);

  ConfusionMatrix2Ptr  cm =   new ConfusionMatrix2 (*classes, f, bucketSize, numOfBuckets, numOfProbBuckets, probBucketSize, log);
  delete  classes;  classes = NULL;
  return  cm;
}   /* BuildFromIstreamXML */



void   ConfusionMatrix2::Read (istream&  f,
                               RunLog&   log
                              )
{
  if  (f.eof ())
  {
    log.Level (-1) << "ConfusionMatrix2::Read   ***ERROR***   File at EOF   can not read any data." << endl;
    return;
  }

  char  buff[10240];
  buff[0] = 0;
  KKStr l  (512);

  classes.DeleteContents ();

  bucketSize        = 0;
  numOfBuckets      = 0;
  numOfProbBuckets  = 0;
  probBucketSize    = 0;

  kkuint32  classIndex = 0;
  KKStr  className = "";

  f.getline (buff, sizeof (buff));

  while  (!f.eof ())
  {
    l = buff;
    l.TrimLeft ();
    l.TrimRight ();

    if  (l.CompareIgnoreCase ("</ConfusionMatrix2>") == 0)
      break;

    KKStr  lineName = l.ExtractToken ("\t\n\r");

    if  (lineName.CompareIgnoreCase ("ClassCount") == 0)
      classCount = l.ExtractTokenInt ("\t\n\r");

    else if (lineName.CompareIgnoreCase ("Classes"))
    {
      auto  classesInXmlFile = MLClassList::BuildListFromDelimtedStr (l, '\t');
      if (classesInXmlFile)
      {
        classes.AddQueue (*classesInXmlFile);
        delete classesInXmlFile;
        classesInXmlFile = NULL;
      }
    }
  
    else if  (lineName.CompareIgnoreCase ("BucketSize") == 0)
      bucketSize = l.ExtractTokenInt ("\t");
      
    else if  (lineName.CompareIgnoreCase ("probBucketSize") == 0)
      probBucketSize = l.ExtractTokenInt ("\t");
    
    else if  (lineName.CompareIgnoreCase ("NumOfBuckets") == 0)
      numOfBuckets = l.ExtractTokenInt ("\t");
      
    else if  (lineName.CompareIgnoreCase ("NumOfProbBuckets") == 0)
      numOfProbBuckets = l.ExtractTokenInt ("\t");
      
    else if  (lineName.CompareIgnoreCase ("TotalCount") == 0)
      totalCount = l.ExtractTokenDouble ("\t");
      
    else if  (lineName.CompareIgnoreCase ("CorrectCount") == 0)
      correctCount = l.ExtractTokenDouble ("\t");
      
    else if  (lineName.CompareIgnoreCase ("TotalPredProb") == 0)
      totalPredProb = l.ExtractTokenDouble ("\t");
      
    else if  (lineName.CompareIgnoreCase ("NumInvalidClassesPredicted") == 0)
      numInvalidClassesPredicted = l.ExtractTokenDouble ("\t");
     
    else if  (lineName.CompareIgnoreCase ("CountsByKnownClass") == 0)
      DelimitedStrToArray (countsByKnownClass, classCount, l, ',');
      
    else if  (lineName.CompareIgnoreCase ("TotalSizesByKnownClass") == 0)
      DelimitedStrToArray (totalSizesByKnownClass, classCount, l, ',');
      
    else if  (lineName.CompareIgnoreCase ("TotalPredProbsByKnownClass") == 0)
      DelimitedStrToArray (totalPredProbsByKnownClass, classCount, l, ',');

    else if  (lineName.CompareIgnoreCase ("ClassTotals") == 0)
    {
      KKStr  classNameLabel = l.ExtractToken ("\t");
      className = l.ExtractToken ("\t");

      KKStr  classIndexLabel = l.ExtractToken ("\t");
      classIndex = l.ExtractTokenUint ("\t");

      if  (classIndex >= classCount)
      {
        log.Level (-1) << endl 
                       << "ConfusionMatrix2::Read    ***ERROR***    ClassIndex[" << classIndex << "]  out of range." << endl
                       << endl;
        classIndex = 0;
        break;
      }
    }

    else if  (lineName.CompareIgnoreCase ("CountByKnownClassBySize") == 0)
      DelimitedStrToArray (countByKnownClassBySize   [classIndex], numOfBuckets, l, ',');

    else if  (lineName.CompareIgnoreCase ("CorrectByKnownClassBySize") == 0)
      DelimitedStrToArray (correctByKnownClassBySize [classIndex], numOfBuckets, l, ',');

    else if  (lineName.CompareIgnoreCase ("CountByKnownClassByProb") == 0)
      DelimitedStrToArray (countByKnownClassByProb   [classIndex], numOfProbBuckets, l, ',');

    else if  (lineName.CompareIgnoreCase ("CorrectByKnownClassByProb") == 0)
      DelimitedStrToArray (correctByKnownClassByProb [classIndex], numOfProbBuckets, l, ',');

    else if  (lineName.CompareIgnoreCase ("PredictedCountsCM") == 0)
      DelimitedStrToArray (predictedCountsCM [classIndex], classCount, l, ',');

    else if  (lineName.CompareIgnoreCase ("TotPredProbCM") == 0)
      DelimitedStrToArray (totPredProbCM [classIndex], classCount, l, ',');

    if  (!f.eof ())
      f.getline (buff, sizeof (buff));
  }
}  /* Read */



void  ConfusionMatrix2::WriteSimpleConfusionMatrix (ostream&  f)  const
{
  // "Estimating the Taxonomic composition of a sample when individule are classified with error"
  // by Andrew Solow, Cabll Davis, Qiao Hu
  // Woods Hole Ocanographic Institution, Woods Hole Massachusetts
  // Marine Ecology Progresss Series
  // published 2006-july-06
  // vol 216:309-311
  
  // This data is meant to work with "ClassificationStatus.cs" to provide the data necessary to adjust for bias.

  f << "<SimpleConfusionMatrix>" << endl;
  f << "Classes"      << "\t" << classes.ToCommaDelimitedStr () << endl;
  kkuint32  row = 0;
  kkuint32  col = 0;
  MLClassList::const_iterator  idx;
  MLClassList::const_iterator  idx2;
  for  (idx = classes.begin ();  idx != classes.end ();  idx++)
  {
    MLClassPtr  mlClass = *idx;
    f << "DataRow" << "\t" << mlClass->Name () << "\t";
    col = 0;
    for  (col = 0;  col < classCount;  col++)
    {
      double p = 0.0;
      if  (countsByKnownClass[row] != 0.0)
        p = predictedCountsCM[row][col] / countsByKnownClass[row];

      if  (col > 0)
        f << ",";

      f << StrFormatDouble (predictedCountsCM[row][col], "ZZZZZ0.00") << ":" << StrFormatDouble (p, "ZZ0.0000000");
    }
    f << std::endl;

    row++;
  }

  f << "</SimpleConfusionMatrix>" << std::endl;
}  /* WriteProbabilityMatrix */



ConfussionMatrix2List::ConfussionMatrix2List (bool _owner):
       KKQueue<ConfusionMatrix2> (_owner)
{

}



ConfussionMatrix2List::~ConfussionMatrix2List ()
{
}



ConfusionMatrix2Ptr  ConfussionMatrix2List::DeriveAverageConfusionMatrix (RunLog&  log)  const
{
  if  (QueueSize () == 0)
  {
    return NULL;
  }

  const_iterator  cmIDX = begin ();
  const ConfusionMatrix2Ptr  firstCM = *cmIDX;

  ConfusionMatrix2Ptr  meanCM = new ConfusionMatrix2 (firstCM->MLClasses ());

  for  (cmIDX = begin ();  cmIDX != end ();  cmIDX++)
  {
    const ConfusionMatrix2Ptr  cm = *cmIDX;
    meanCM->AddIn (*cm, log); 
  }

  double  factor = 1.0 / (double)QueueSize ();

  meanCM->FactorCounts (factor);

  return  meanCM;
}  /* DeriveAverageConfusionMatrix */

