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
using namespace  KKMachineLearning;




ConfusionMatrix2::ConfusionMatrix2 (const MLClassList&  _classes,  // Will make its own copy of '_classes'
                                    istream&            f,
                                    kkint32             _bucketSize,
                                    kkint32             _numOfBuckets,
                                    kkint32             _numOfProbBuckets,
                                    kkint32             _probBucketSize,
                                    RunLog&             _log
                                   ):
  bucketSize                  (_bucketSize),
  classCount                  (0),
  correctByKnownClassByProb   (NULL),
  correctByKnownClassBySize   (NULL),
  correctCount                (0.0),
  countByKnownClassByProb     (NULL),
  countByKnownClassBySize     (NULL),
  countsByKnownClass          (NULL),
  classes                     (_classes),
  log                         (_log),
  numInvalidClassesPredicted  (0.0),
  numOfBuckets                (_numOfBuckets),
  numOfProbBuckets            (_numOfProbBuckets),
  predictedCountsCM           (NULL),
  probBucketSize              (_probBucketSize),
  totalCount                  (0.0),
  totalPredProb               (0.0),
  totalPredProbsByKnownClass  (NULL),
  totalSizesByKnownClass      (NULL),
  totPredProbCM               (NULL)
{
  InitializeMemory ();
  Read (f);
}



ConfusionMatrix2::ConfusionMatrix2 (const MLClassList&  _classes,   // Will make its own copy of list
                                    RunLog&             _log
                                   ):

  bucketSize                  (100),
  classCount                  (0),
  correctByKnownClassByProb   (NULL),
  correctByKnownClassBySize   (NULL),
  correctCount                (0.0),
  countByKnownClassByProb     (NULL),
  countByKnownClassBySize     (NULL),
  countsByKnownClass          (NULL),
  classes                     (_classes),
  log                         (_log),
  numInvalidClassesPredicted  (0.0),
  numOfBuckets                (40),
  numOfProbBuckets            (20),
  predictedCountsCM           (NULL),
  probBucketSize              (5),
  totalCount                  (0.0),
  totalPredProb               (0.0),
  totalPredProbsByKnownClass  (NULL),
  totalSizesByKnownClass      (NULL),
  totPredProbCM               (NULL)

{
  InitializeMemory ();
}




ConfusionMatrix2::ConfusionMatrix2 (const ConfusionMatrix2&  cm):

  bucketSize                  (cm.bucketSize),
  classCount                  (cm.classCount),
  correctByKnownClassByProb   (NULL),
  correctByKnownClassBySize   (NULL),
  correctCount                (cm.correctCount),
  countByKnownClassByProb     (NULL),
  countByKnownClassBySize     (NULL),
  countsByKnownClass          (NULL),
  classes                     (cm.classes),
  log                         (cm.log),
  numInvalidClassesPredicted  (cm.numInvalidClassesPredicted),
  numOfBuckets                (cm.numOfBuckets),
  numOfProbBuckets            (cm.numOfProbBuckets),
  predictedCountsCM           (NULL),
  probBucketSize              (cm.probBucketSize),
  totalCount                  (cm.totalCount),
  totalPredProb               (cm.totalPredProb),
  totalPredProbsByKnownClass  (NULL),
  totalSizesByKnownClass      (NULL),
  totPredProbCM               (NULL)

{
  kkint32  x;
  kkint32  y;

  countsByKnownClass         = new double [classCount];
  totalPredProbsByKnownClass = new double [classCount];
  totalSizesByKnownClass     = new double [classCount];

  predictedCountsCM = new double* [classCount];
  totPredProbCM     = new double* [classCount];

  countByKnownClassBySize   = new double* [classCount];
  correctByKnownClassBySize = new double* [classCount];

  countByKnownClassByProb   = new double* [classCount];
  correctByKnownClassByProb = new double* [classCount];


  for  (x = 0; x < classCount; x++)
  {
    countsByKnownClass         [x] = cm.countsByKnownClass         [x];
    totalSizesByKnownClass     [x] = cm.totalSizesByKnownClass     [x];
    totalPredProbsByKnownClass [x] = cm.totalPredProbsByKnownClass [x];

    predictedCountsCM [x] = new double [classCount];
    totPredProbCM     [x] = new double [classCount];

    countByKnownClassBySize   [x] = new double [numOfBuckets];
    correctByKnownClassBySize [x] = new double [numOfBuckets];

    countByKnownClassByProb   [x] = new double [numOfProbBuckets];
    correctByKnownClassByProb [x] = new double [numOfProbBuckets];

    for  (y = 0; y < classCount; y++)
    {
      predictedCountsCM [x][y] = cm.predictedCountsCM [x][y];
      totPredProbCM     [x][y] = cm.totPredProbCM        [x][y];
    }

    for  (y = 0; y < numOfBuckets; y++)
    {
      countByKnownClassBySize   [x][y] = cm.countByKnownClassBySize   [x][y];
      correctByKnownClassBySize [x][y] = cm.correctByKnownClassBySize [x][y];
    }

    for  (y = 0; y < numOfProbBuckets; y++)
    {
      countByKnownClassByProb   [x][y] = cm.countByKnownClassByProb   [x][y];
      correctByKnownClassByProb [x][y] = cm.correctByKnownClassByProb [x][y];
    }
  }
}




ConfusionMatrix2::~ConfusionMatrix2 ()
{
  kkint32  x;
  for  (x = 0; x < classCount; x++)
  {
    delete[] predictedCountsCM [x];
    delete[] totPredProbCM     [x];

    delete[] countByKnownClassBySize   [x];
    delete[] correctByKnownClassBySize [x];

    delete[] countByKnownClassByProb   [x];
    delete[] correctByKnownClassByProb [x];

  }


  delete[] predictedCountsCM;
  delete[] totPredProbCM;
  delete[] countsByKnownClass;
  delete[] totalSizesByKnownClass;
  delete[] totalPredProbsByKnownClass;

  delete[] countByKnownClassBySize;
  delete[] correctByKnownClassBySize;

  delete[] countByKnownClassByProb;
  delete[] correctByKnownClassByProb;
}




void  ConfusionMatrix2::InitializeMemory ()
{
  kkint32  x;
  kkint32  y;

  classes.SortByName ();

  classCount = classes.QueueSize ();

  countsByKnownClass         = new double [classCount];
  totalSizesByKnownClass     = new double [classCount];
  totalPredProbsByKnownClass = new double [classCount];

  predictedCountsCM          = new double* [classCount];
  totPredProbCM              = new double* [classCount];

  countByKnownClassBySize    = new double* [classCount];
  correctByKnownClassBySize  = new double* [classCount];

  countByKnownClassByProb    = new double* [classCount];
  correctByKnownClassByProb  = new double* [classCount];


  for  (x = 0; x < classCount; x++)
  {
    countsByKnownClass         [x] = 0.0;
    totalSizesByKnownClass     [x] = 0.0;
    totalPredProbsByKnownClass [x] = 0.0;

    predictedCountsCM [x] = new double [classCount];
    totPredProbCM     [x] = new double [classCount];

    countByKnownClassBySize   [x] = new double [numOfBuckets];
    correctByKnownClassBySize [x] = new double [numOfBuckets];

    countByKnownClassByProb   [x] = new double [numOfProbBuckets];
    correctByKnownClassByProb [x] = new double [numOfProbBuckets];

    for  (y = 0; y < classCount; y++)
    {
      predictedCountsCM [x][y] = 0.0;
      totPredProbCM     [x][y] = 0.0;
    }

    for  (y = 0; y < numOfBuckets; y++)
    {
      countByKnownClassBySize   [x][y] = 0;
      correctByKnownClassBySize [x][y] = 0;
    }

    for  (y = 0; y < numOfProbBuckets; y++)
    {
      countByKnownClassByProb   [x][y] = 0;
      correctByKnownClassByProb [x][y] = 0;
    }
  }
}  /* InitializeMemory */




double  ConfusionMatrix2::PredictedCountsCM (kkint32  knownClassIdx, 
                                             kkint32  predClassIdx
                                            )  const
{
  if  ((knownClassIdx < 0)  ||  (knownClassIdx >= classCount))
    return 0.0;
 
  if  ((predClassIdx < 0)   ||  (predClassIdx >= classCount))
    return 0.0;
 
  return  predictedCountsCM [knownClassIdx][predClassIdx];
}



VectorDouble  ConfusionMatrix2::PredictedCounts ()  const
{
  kkint32  knownClassIdx, predClassIdx;

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




double  ConfusionMatrix2::CountsByKnownClass (kkint32 knownClassIdx)  const
{
  if  ((knownClassIdx < 0)  ||  (knownClassIdx >= classCount))
    return 0.0;
  
  return  countsByKnownClass [knownClassIdx];
}



VectorDouble  ConfusionMatrix2::CountsByKnownClass ()  const
{
  VectorDouble  cbnc;
  for  (kkint32 x = 0;  x < classCount;  x++)
    cbnc.push_back (countsByKnownClass[x]);
  return  cbnc;
}  /* CountsByKnownClass */




void  ConfusionMatrix2::Increment (MLClassPtr  _knownClass,
                                   MLClassPtr  _predClass,
                                   kkint32     _size,
                                   double      _probability
                                  )
{
  kkint32  knownClassNum = -1;
  kkint32  predClassNum  = -1;

  if  (_probability < 0)
    _probability = 0;

  if  (!_knownClass)
  {
    numInvalidClassesPredicted += 1.0;
    log.Level (-1) << endl
                   << "ConfusionMatrix2::Increment  **** _knownClass = NULL ****"
                   << endl
                   << endl;
    return;
  }

  if  (!_predClass)
  {
    numInvalidClassesPredicted += 1.0;
    log.Level (-1) << endl
                   << "ConfusionMatrix2::Increment  **** _predClass = NULL ****"
                   << endl
                   << endl;
    return;
  }

  knownClassNum = classes.PtrToIdx (_knownClass);
  predClassNum  = classes.PtrToIdx (_predClass);

  if  ((knownClassNum < 0)  ||  (knownClassNum >= classCount))
  {
    numInvalidClassesPredicted += 1.0;
    log.Level (-1) << "ConfusionMatrix2::IncrementPredHits    knownClassNum[" << knownClassNum << "] out of bounds." << endl;
    return;
  }

  if  ((predClassNum < 0)  ||  (predClassNum >= classCount))
  {
    numInvalidClassesPredicted += 1.0;
    log.Level (-1) << "ConfusionMatrix2::IncrementPredHits    predClassNum[" << predClassNum << "] out of bounds." << endl;
    return;
  }

  if  (knownClassNum == predClassNum)
     correctCount += 1.0;

  totalCount += 1.0;

  totalSizesByKnownClass[knownClassNum] += _size;

  totalPredProbsByKnownClass [knownClassNum] += _probability;
  totalPredProb                              += _probability;

  countsByKnownClass [knownClassNum]++;

  (predictedCountsCM [knownClassNum] [predClassNum])++;
  totPredProbCM      [knownClassNum] [predClassNum] += _probability;


  if  (_size > 0)
  {
    kkint32  bucket = (_size - 1) / bucketSize;
    if  (bucket >= numOfBuckets)
      bucket = numOfBuckets - 1;

    countByKnownClassBySize[knownClassNum][bucket]++;
    if  (knownClassNum == predClassNum)
      correctByKnownClassBySize [knownClassNum][bucket]++;
  }
  else
  {
    _size = -1;
  }


  {
     kkint32  bucket = 0;
     
     if  ((_probability >= 0.0)  &&  (_probability <= 1.0))
       bucket = ((kkint32)(_probability * 100) / probBucketSize);
     else
       bucket = 0;

     if  (bucket >= numOfProbBuckets)
       bucket = numOfProbBuckets - 1;

     countByKnownClassByProb [knownClassNum][bucket]++;
    if  (knownClassNum == predClassNum)
      correctByKnownClassByProb [knownClassNum][bucket]++;
  }
}  /* Increment */





KKStr  StripOutInvalidLatexCaracters (const KKStr&  src)
{
  kkint32  newLen = (kkint32)(src.Len () * 1.3);

  KKStr  result (newLen);


  for  (kkint32 x = 0;  x < src.Len ();  x++)
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
                                        )
{
  kkint32  predClassNum;

  if  (_name.Len () > 25)
    _name = _name.SubStrPart (_name.Len () - 25);

  _outFile << setw (25) << _name
           << setw (16) << _lineTotal;

  for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
  {
    _outFile << setw (16) << _splits [predClassNum];
  }

  _outFile << endl;
}  /* PrintSingleLine */






void  ConfusionMatrix2::PrintSingleLineTabDelimited (ostream&      _outFile,
                                                     const KKStr&  _name,
                                                     double        _lineTotal,
                                                     double        _splits[]
                                                    )
{
  kkint32  predClassNum;

  KKStr  name (_name);
  name << "(" << _lineTotal << ")";

  _outFile << _name << "\t" << _lineTotal;

  for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
  {
    _outFile << "\t" << _splits [predClassNum];
  }

  _outFile << endl;
}  /* PrintSingleLineTabDelimited */



                                                     
void  ConfusionMatrix2::PrintSingleLineHTML (ostream&     o,
                                             const KKStr& _name,
                                             double       _lineTotal,
                                             kkint32      _knownClassNum,
                                             double       _splits[]
                                            )
{
  kkint32  predClassNum;

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
                                                   kkint32        _knownClassNum, 
                                                   const KKStr&   _name,
                                                   double         _lineTotal,
                                                   double         _splits[]
                                                  )
{
  kkint32  predClassNum;

  KKStr  name (_name);
  name << "(" << _lineTotal << ")";

  _outFile << StripOutInvalidLatexCaracters (_name) << " & " << _lineTotal;

  for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
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
                                              double        _splits[])
{
  kkint32  predClassNum;

  KKStr  name (_name);
  name << "(" << _lineTotal << ")";

  _outFile << setw (25) << name;

  for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
  {
    _outFile << setw (6) << _splits [predClassNum];
  }

  _outFile << endl;
}





void  ConfusionMatrix2::PrintPercentLine (ostream&  _outFile,
                                          KKStr     _name,
                                          double    _lineTotal,
                                          double    _splits[]
                                         )
{
  char    buff[40];
  double  perc;
  kkint32   predClassNum;

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

  for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
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
                                                     )
{
  double  perc;
  kkint32 predClassNum;

  if  (totalCount <= 0.0)
    perc = 0.0;
  else
    perc = (double)_lineTotal / totalCount;

  _outFile << _name << "\t" 
           << StrFormatDouble ((100.0 * perc), "zz0.00") << "%";
           

  for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
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
                                                  kkint32       _knownClassNum,
                                                  double        _avgPredProbs[],
                                                  double        _numPredByClass[]
                                                 )
{
  double  avgPredProb;
  kkint32 predClassNum;

  if  (_totalCountThisLine <= 0.0)
    avgPredProb = 0.0;
  else
    avgPredProb = _totalAvgPredProbThisLine / _totalCountThisLine;

  KKStr avgPredProbStr = StrFormatDouble ((100.0 * avgPredProb), "zz0.00") + "%";
  o << "    <tr>" << "<td style=\"text-align:left; font-family:Arial\">" << _name << "</td>" << "<td>" << avgPredProbStr << "</td>";

  for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
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
                                              kkint32       _knownClassNum,
                                              double        _splits[]
                                             )
{
  double  perc;
  kkint32   predClassNum;

  if  (totalCount <= 0.0)
    perc = 0.0;
  else
    perc = (double)_lineTotal / totalCount;

  KKStr percentStr = StrFormatDouble ((100.0 * perc), "zz0.00") + "%";
  o << "    <tr>" << "<td style=\"text-align:left; font-family:Arial\">" << _name << "</td>" << "<td>" << percentStr << "</td>";

  for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
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
                                              )
{
  double  perc;
  kkint32 predClassNum;

  if  (totalCount == 0.0)
    perc = 0.0;
  else
    perc = 100.0 * (double)_lineTotal / totalCount;

  KKStr name (_name);
  name << "(" << StrFormatDouble (perc, "ZZ0.0") << ")";

  _outFile << setw (25) << name;

  for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
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
                                                    kkint32       _rowNum,
                                                    const KKStr&  _name,
                                                    double        _lineTotal,
                                                    double        _splits[]
                                                    )
{   
  double  perc;
  kkint32 predClassNum;

  if  (totalCount <= 0.0)
    perc = 0.0;
  else
    perc = (double)_lineTotal / totalCount;

  _outFile << StripOutInvalidLatexCaracters (_name) << " & " 
           << StrFormatDouble ((100.0 * perc), "zz0.0") << "\\%";

  for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
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






void   ConfusionMatrix2::PrintConfusionMatrix (ostream&  outFile)
{
  kkint32 knownClassNum;
  kkint32 predClassNum;
  kkint32 x;

 
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
  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    outFile << setw (16) << "============";
  }
  outFile << endl;

  double*  totals = new double[classCount];
  for  (x = 0; x < classCount; x++)
    totals[x] = 0;


  double  totalNonNoise = 0;
  double  totalNonNoiseRight = 0;

  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    bool  noiseClass = classes[knownClassNum].UnDefined ();

    for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
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

  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
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

  delete  totals;
}  /* PrintConfusionMatrix */



void  ConfusionMatrix2::PrintConfusionMatrixHTML (ostream&  o)
{
  kkint32 knownClassNum;
  kkint32 predClassNum;
  kkint32 x;


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
  for  (x = 0; x < classCount; x++)
    totals[x] = 0;

  double  totalNonNoise = 0;
  double  totalNonNoiseRight = 0;

  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    bool  noiseClass = classes[knownClassNum].UnDefined ();

    for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
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
                       -1,
                       totals
                      );

  o << "<tr><td colspan=\"" << (classCount + 2) << "\">&nbsp</td></tr>" << endl;

  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
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






void  ConfusionMatrix2::PrintConfusionMatrixAvgPredProbHTML (ostream&   o)

{
  kkint32 knownClassNum;
  kkint32 predClassNum;

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
  for  (predClassNum = 0;  predClassNum < classCount;  predClassNum++)
  {
    totalPredProbByPredClass [predClassNum] = 0.0;
    totalCountsByPredClass   [predClassNum] = 0.0;
  }

  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    PrintAvgPredProbLineHTML (o, 
                              classes                    [knownClassNum].Name (),
                              totalPredProbsByKnownClass [knownClassNum],
                              countsByKnownClass         [knownClassNum],
                              knownClassNum,
                              totPredProbCM              [knownClassNum],
                              predictedCountsCM          [knownClassNum]
                             );
    for  (predClassNum = 0;  predClassNum < classCount;  predClassNum++)
    {
      totalPredProbByPredClass [predClassNum] += totPredProbCM     [knownClassNum][predClassNum];
      totalCountsByPredClass   [predClassNum] += predictedCountsCM [knownClassNum][predClassNum];
    }
  }

  o << "<tr><td colspan=\"" << (classCount + 2) << "\">&nbsp</td></tr>" << endl;

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

  delete  totalCountsByPredClass;
  delete  totalPredProbByPredClass;
}  /* PrintConfusionMatrixHTML */








void  ConfusionMatrix2::PrintAccuracyByProbByClassHTML (ostream&   o)

{
  kkint32 bucket;
  kkint32 classNum;

  VectorDouble countByProb   (numOfProbBuckets, 0.0);
  VectorDouble correctByProb (numOfProbBuckets, 0.0);

  double  totalCount   = 0.0;
  double  totalCorrect = 0.0;

  double  acc = 0.0;

  o << "<table align=\"center\" border=\"2\" cellpadding=\"3\" cellspacing=\"0\" frame=\"box\"  summary=\"Confusion \" >" << endl
    << "  <thead style=\"font-weight:bold; text-align:center; vertical-align:bottom\">"          << endl
    << "    <tr>"                                                                                << endl
    << "        <th>Class<br />Names</th><th>All<br />Classes</th>";

  for  (bucket = 0;  bucket < numOfProbBuckets;  bucket++)
  {
    o << "<th>" << ((bucket + 1) * probBucketSize) << "</th>";
  }
  o << "    </tr>"                                                                               << endl
    << "  </thead>"                                                                              << endl
    << "  <tbody style=\"font-weight:normal; text-align:right; font-family:Courier\">" << endl;

  KKStr  ln (1024);
  KKStr  accStr;

  for  (classNum = 0;  classNum < classCount;  classNum++)
  {
    double  countThisClass   = 0.0;
    double  correctThisClass = 0.0;  

    ln = "";

    for  (bucket = 0;  bucket < numOfProbBuckets;  bucket++)
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

    totalCount   += countThisClass;
    totalCorrect += correctThisClass;

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
    if  (totalCount != 0.0)
      acc = 100.0 * totalCorrect / totalCount;

    o << "    <tr>" 
      << "<td style=\"text-align:left; font-family:Arial\">" << "Total<br />All Classes"  << "</td>" 
      << "<td>" << StrFormatDouble (acc, "ZZ0.000") << "%" << "</td>";

    for  (bucket = 0;  bucket < numOfProbBuckets;  bucket++)
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







void   ConfusionMatrix2::PrintConfusionMatrixTabDelimited (ostream&  outFile)
{
  kkint32 knownClassNum;
  kkint32 predClassNum;
  kkint32 x;

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
  for  (x = 0; x < classCount; x++)
    totals[x] = 0;

  double  totalNonNoise = 0;
  double  totalNonNoiseRight = 0;

  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    bool  noiseClass = classes[knownClassNum].UnDefined ();

    for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
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

  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
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







void   ConfusionMatrix2::PrintLatexTableColumnHeaders (ostream&  outFile)
{
  kkint32  x = 0;

  outFile << "\\begin{tabular}{|";
  for  (x = 0;  x < (classCount + 2);  x++)
    outFile << "c|";
  outFile << "}" << endl;

  outFile << "\\hline" << endl;
  
  outFile << "Class Names" << " & " << "Count";

  for  (x = 0;  x < classCount;  x++)
  {
    outFile << " & " << StripOutInvalidLatexCaracters (classes[x].Name ());
  }
  outFile << "\\\\" << endl;


  outFile << "\\hline" << endl;
}  /* PrintConfusionMatrixLatexTableColumnHeaders */






void   ConfusionMatrix2::PrintConfusionMatrixLatexTable (ostream&  outFile)
{
  kkint32 knownClassNum;
  kkint32 predClassNum;
  kkint32 x;

  double  overallAccuracy = 0.0;
  if  (totalCount != 0.0)
    overallAccuracy = 100.0 * correctCount / totalCount;

  outFile  << "Overall Accuracy:\t" 
           << StrFormatDouble (overallAccuracy, "ZZZ0.000") << "\\%\\\\"
           << endl;

  PrintLatexTableColumnHeaders (outFile);
  

  double*  totals = new double[classCount];
  for  (x = 0; x < classCount; x++)
    totals[x] = 0;


  double  totalNonNoise = 0;
  double  totalNonNoiseRight = 0;

  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    bool  noiseClass = classes[knownClassNum].UnDefined ();

    for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
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
                             -1,
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
  
  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
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

  delete  totals;
}  /* PrintConfusionMatrixLatexTable */







void   ConfusionMatrix2::PrintConfusionMatrixNarrow (ostream&  outFile)
{
  kkint32 knownClassNum;
  kkint32 predClassNum;
  kkint32 x;

 
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

  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    KKStr  colDesc ("Col");
    colDesc << (knownClassNum + 1);

    colDesc.LeftPad (6);

    outFile << colDesc;
  }
  outFile << endl;

  outFile << setw (25) << "===========";
  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    outFile << setw (6) << "====";
  }
  outFile << endl;

  double*  totals = new double[classCount];
  for  (x = 0; x < classCount; x++)
    totals[x] = 0;


  double  totalNonNoise = 0;
  double  totalNonNoiseRight = 0;

  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    bool  noiseClass = classes[knownClassNum].UnDefined ();

    for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
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

  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
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

  delete  totals;
}  /* PrintCrossValidationNarrow */



double  PercentOf (double x,  double y)
{
  double  total = x + y;
  if  (total == 0.0)
    return 0.0;
  else 
    return 100.0 * x / total;
}




void   ConfusionMatrix2::PrintTrueFalsePositivesTabDelimited (ostream&  r)
{
  kkint32  numOfClasses = classes.QueueSize ();
  kkint32  x = 0;
  kkint32  y = 0;

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


  for  (x = 0;  x < numOfClasses;  x++)
  {
    truePositives [x] = predictedCountsCM [x][x];
    totalTP += predictedCountsCM[x][x];

    trueNegatives [x] = 0.0;
    falsePositives[x] = 0.0;
    falseNegatives[x] = 0.0;

    for  (y = 0;  y < numOfClasses;  y++)
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
  for  (x = 0;  x < numOfClasses;  x++)
  {
    r << "\t" << truePositives[x];
  }
  r << endl;

  r << "FalsePositives" << "\t" << totalFP;
  for  (x = 0;  x < numOfClasses;  x++)
  {
    r << "\t" << falsePositives[x];
  }
  r << endl;

  r << "TrueNegatives" << "\t" << totalTN;
  for  (x = 0;  x < numOfClasses;  x++)
  {
    r << "\t" << trueNegatives[x];
  }
  r << endl;

  r << "FalseNegatives"  << "\t" << totalFN;
  for  (x = 0;  x < numOfClasses;  x++)
  {
    r << "\t" << falseNegatives[x];
  }
  r << endl;

  r << endl;
  r << "Sensitivity(TP/(TP+FN))" << "\t" << StrFormatDouble(PercentOf (totalTP, totalFN), "zzz,zz0.00") << "%";
  for  (x = 0;  x < numOfClasses;  x++)
  {
    r << "\t" << StrFormatDouble(PercentOf (truePositives[x], falseNegatives[x]), "zzz,zz0.00") << "%";
  }
  r << endl;

  r << "Specificity(TN/(TN+FP))" << "\t" << StrFormatDouble(PercentOf (totalTN, totalFP), "zzz,zz0.00") << "%";
  for  (x = 0;  x < numOfClasses;  x++)
  {
    r << "\t" << StrFormatDouble(PercentOf (trueNegatives[x], falsePositives[x]), "zzz,zz0.00") << "%";
  }
  r << endl;

  r << "PositivePredictiveValue(TP/(TP+FP))" << "\t" << StrFormatDouble(PercentOf (totalTP, totalFP), "zzz,zz0.00") << "%";
  for  (x = 0;  x < numOfClasses;  x++)
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
    for  (x = 0;  x < numOfClasses;  x++)
    {
      fMeasure = 0.0;
      divisor = 2.0 * (double)truePositives[x] + (double)falsePositives[x] + (double)falseNegatives[x];
      if  (divisor != 0.0)
        fMeasure = 100.0 * (2.0 * (double)truePositives[x] / divisor);

      r << "\t" << StrFormatDouble(fMeasure, "zzz,zz0.00") << "%";
    }
    r << endl;
  }

  delete  falseNegatives;
  delete  falsePositives;
  delete  trueNegatives;
  delete  truePositives;
}  /* PrintTrueFalsePositivesTabDelimited */




float  ConfusionMatrix2::FMeasure (MLClassPtr  positiveClass)  const
{
  kkint32 positiveIDX = classes.PtrToIdx (positiveClass);
  if  (positiveIDX < 0)
  {
    KKStr invalidClassName = "";
    if  (positiveClass)
       invalidClassName = positiveClass->Name ();

    log.Level (-1) << "ConfusionMatrix2::FMeasure         ***ERROR***      Invalid Positive Class Specified[" << invalidClassName << "]" << endl;
    return 0.0f;
  }

  kkint32  numOfClasses = classes.QueueSize ();

  kkint32  y;

  double  totalTP = 0.0;
  double  totalFP = 0.0;
  double  totalTN = 0.0;
  double  totalFN = 0.0;

  totalTP = predictedCountsCM[positiveIDX][positiveIDX];

  for  (y = 0;  y < numOfClasses;  y++)
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





void   ConfusionMatrix2::PrintErrorBySize (ostream&  outFile)
{
  kkint32  bucket;
  kkint32  classNum;

  outFile << endl;

  outFile << "Size" << "\t";

  // Lets first Print Titles.
  for  (classNum = 0; classNum < classCount; classNum++)
  {
    outFile << "\t\t";
    outFile << classes[classNum].Name ();
  }
  outFile << endl;



  for  (bucket = 0; bucket < numOfBuckets; bucket++)
  {
    outFile << ((bucket + 1) * bucketSize) << "\t";

    kkint32  classNum;

    for  (classNum = 0; classNum < classCount; classNum++)
    {
      outFile << countByKnownClassBySize   [classNum][bucket] << "\t"
              << correctByKnownClassBySize [classNum][bucket] << "\t";
    }

    outFile << endl;
  }

  outFile << endl;
}  /* PrintErrorBySize */




void   ConfusionMatrix2::PrintErrorByProb (ostream&  outFile)
{
  kkint32  bucket;
  kkint32  classNum;

  outFile << endl;

  outFile << "Prob" << "\t";

  // Lets first Print Titles.
  for  (classNum = 0; classNum < classCount; classNum++)
  {
    outFile << "\t\t";
    outFile << classes[classNum].Name ();
  }

  outFile << endl;


  outFile << setiosflags (ios::fixed);

  for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    outFile << ((bucket + 1) * probBucketSize) << "%\t";

    kkint32  classNum;

    for  (classNum = 0; classNum < classCount; classNum++)
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





void   ConfusionMatrix2::PrintErrorBySizeByRows (ostream&  outFile)
{
  kkint32  bucket;
  kkint32  classNum;


  outFile  << endl
           << "Error by size" << endl
           << endl;

  outFile  << "ClassName\tAvg Size\t";
  for  (bucket = 0; bucket < numOfBuckets; bucket++)
  {
    outFile << ((bucket + 1) * bucketSize) << "\t";
  }

  outFile << endl
          << endl;

  for  (classNum = 0; classNum < classCount; classNum++)
  {
    double  avg;
    if  (countsByKnownClass [classNum] != 0)
      avg = totalSizesByKnownClass[classNum] / countsByKnownClass [classNum];
    else
      avg = 0;

    outFile << classes[classNum].Name ()
            << "\t"
            <<  avg;


    for  (bucket = 0; bucket < numOfBuckets; bucket++)
    {
      outFile << "\t" << countByKnownClassBySize[classNum][bucket];
    }
    outFile << endl;

    
    outFile << classes[classNum].Name () << " Correct" << "\t";
    for  (bucket = 0; bucket < numOfBuckets; bucket++)
    {
      outFile << "\t" << correctByKnownClassBySize [classNum][bucket];
    }

    outFile << endl;
    outFile << endl;
  }
  outFile << endl;


  outFile << endl
          << endl;

  outFile  << "ClassName\tAvg Size\t";
  for  (bucket = 0; bucket < numOfBuckets; bucket++)
  {
    outFile << ((bucket + 1) * bucketSize) << "\t";
  }

  outFile << endl
          << endl;

  for  (classNum = 0; classNum < classCount; classNum++)
  {
    double  avg;
    if  (countsByKnownClass [classNum] != 0)
      avg = totalSizesByKnownClass[classNum] / countsByKnownClass [classNum];
    else
      avg = 0;

    outFile << classes[classNum].Name ()
            << "\t"
            <<  avg;


    for  (bucket = 0; bucket < numOfBuckets; bucket++)
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





void   ConfusionMatrix2::PrintErrorByProbByRows (ostream&  outFile)
{
  kkint32  bucket;
  kkint32  classNum;

  kkint32  x;

  double*  totalCount   = new double[numOfProbBuckets];
  double*  totalCorrect = new double[numOfProbBuckets];

  for  (x = 0; x < numOfProbBuckets; x++)
  {
    totalCount   [x] = 0;
    totalCorrect [x] = 0;
  }

  outFile  << "ClassName\tAvg Prob\t";
  for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    outFile << ((bucket + 1) * probBucketSize) << "%\t";
  }

  outFile << endl
          << endl;

  for  (classNum = 0; classNum < classCount; classNum++)
  {
    double  avg;
    if  (countsByKnownClass [classNum] != 0)
      avg = 100.0 * totalPredProbsByKnownClass [classNum] / countsByKnownClass [classNum];
    else
      avg = 0;

    outFile << classes[classNum].Name ()
            << "\t"
            <<  avg << "%";


    for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
    {
      outFile << "\t" << countByKnownClassByProb [classNum][bucket];
      totalCount[bucket] = totalCount[bucket] + countByKnownClassByProb [classNum][bucket];
    }
    outFile << endl;
    


    outFile << classes[classNum].Name () << " Correct" << "\t";
    for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
    {
      outFile << "\t" << correctByKnownClassByProb [classNum][bucket];
      totalCorrect [bucket] = totalCorrect [bucket] + correctByKnownClassByProb [classNum][bucket];
    }
    outFile << endl;


    outFile << "Accuracy"  << "\t";
    for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
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
  for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    outFile << "\t" << totalCount[bucket];
  }

  outFile << endl;

  outFile << "Correct" << "\t";
  for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    outFile << "\t" << totalCorrect[bucket];
  }
  outFile << endl;

  outFile << "Accuracy" << "\t";
  for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    double  perc;

    if  (totalCount[bucket] <= 0)
      perc = 0.0;
    else
      perc = 100.0 * totalCorrect[bucket] / totalCount[bucket];

    outFile << "\t" << perc << "%";
  }

  delete  totalCount;
  delete  totalCorrect;

  outFile << endl;

  return;
}  /* PrintErrorByProbByRows */





void  ConfusionMatrix2::PrintProbDistributionTitle (ostream&  outFile)
{
  kkint32  bucket;

  outFile << "Total";
  for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    outFile << "\t" << ((bucket + 1) * probBucketSize) << "%";
  }

  outFile << endl;
}  /* PrintPronDistributionTitle */






void   ConfusionMatrix2::PrintProbDistributionTotalCount (ostream&  outFile)
{
  kkint32  bucket;
  kkint32  classNum;

  double*  count   = new double[numOfProbBuckets];

  for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    count [bucket] = 0;
  }


  double  total = 0;

  for  (classNum = 0; classNum < classCount; classNum++)
  {
    for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
    {
      count[bucket] = count[bucket] + countByKnownClassByProb [classNum][bucket];
    }
  }


  for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    total = total + count[bucket];
  }


  outFile << total;
  for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    outFile << "\t" << count[bucket];
  }

  outFile << endl;

  delete  count;

  return;
}  /* PrintProbDistributionTotalCount */





void   ConfusionMatrix2::PrintProbDistributionTotalError (ostream&  outFile)
{
  kkint32  bucket;
  kkint32  classNum;

  double*  count   = new double[numOfProbBuckets];
  double*  correct = new double[numOfProbBuckets];

  for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    count   [bucket] = 0;
    correct [bucket] = 0;
  }


  for  (classNum = 0; classNum < classCount; classNum++)
  {
    for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
    {
      count  [bucket] = count  [bucket] + countByKnownClassByProb   [classNum][bucket];
      correct[bucket] = correct[bucket] + correctByKnownClassByProb [classNum][bucket];
    }
  }


  double  overallAccuracy = 0.0;
  
  {
    double  totalCount   = 0;
    double  totalCorrect = 0;

    for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
    {
      totalCount   = totalCount   + count[bucket];
      totalCorrect = totalCorrect + correct[bucket];
    }

    if  (totalCount == 0)
    {
      overallAccuracy = 0.0;
    }
    else
    {
      overallAccuracy = 100.0 * (double)totalCorrect  /  (double)totalCount;
    }
  }

  outFile << setprecision (2);

  outFile << overallAccuracy << "%";

  for  (bucket = 0; bucket < numOfProbBuckets; bucket++)
  {
    double  accuracy = 0.0;
    
    if  (count[bucket] > 0)
       accuracy = 100.0 * (double)correct[bucket] / (double)count[bucket];

    outFile << "\t" << accuracy << "%";
  }

  outFile << endl;

  delete  count;
  delete  correct;
}  /* PrintProbDistributionTotalError */
 





void   ConfusionMatrix2::PrintErrorBySizeReduced (ostream&  outFile)
{
  kkint32  classNum;

  outFile << endl;
  outFile << endl;
  outFile << endl;

  for  (classNum = 0; classNum < classCount; classNum++)
     PrintErrorBySizeRowReduced (outFile, classNum);

}  /* PrintErrorBySizeReduced */






void   ConfusionMatrix2::PrintErrorBySizeRowReduced (ostream&  outFile,
                                                     kkint32     classNum
                                                    )
{
  kkint32*   bucketHeadings = new kkint32 [numOfBuckets];
  double*  bucketCount    = new double[numOfBuckets];
  double*  bucketCorrect  = new double[numOfBuckets];

  kkint32 bucket = 0;

  kkint32 numNewBuckets = 0;

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

  delete  bucketHeadings;
  delete  bucketCount;
  delete  bucketCorrect;
}  /* PrintErrorBySizeRowReduced */




KKStr   ConfusionMatrix2::AccuracyStr ()
{

  kkint32 x;
 
  double*  accuracys = new double[classCount];

  for  (x = 0; x < classCount; x++)
  {
    if  (countsByKnownClass [x] == 0)
      accuracys[x] = 0;
    else
      accuracys[x] = (100.0 * (double) predictedCountsCM [x] [x]) / ((double) (countsByKnownClass [x]));
  }


  KKStr  accuracyStr;

  for  (x = 0; x < classCount; x++)
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

  delete  accuracys;

  return  accuracyStr;
}  /* AccuracyStr */





double  ConfusionMatrix2::Accuracy ()
{
  if  (totalCount == 0)
    return 0.0;

  return  100.0 * correctCount / totalCount;
}




double  ConfusionMatrix2::AvgPredProb ()  const
{
  if  (totalCount == 0)
    return 0.0;

  return  totalPredProb / (double)totalCount;
}





double  ConfusionMatrix2::Accuracy (MLClassPtr  mlClass)
{
  kkint32  classNum   = 0;

  classNum = classes.PtrToIdx (mlClass);
  if  (classNum < 0)
    return 0.0f;

  if  (countsByKnownClass [classNum] == 0)
    return 0.0f;

  float  accuracy = (float)(100.0 * (predictedCountsCM[classNum] [classNum])  /  (countsByKnownClass [classNum]));

  return  accuracy;
}  /* Accuracy */




VectorFloat ConfusionMatrix2::AccuracyByClass  ()  const
{
  VectorFloat  accuracies;
  for  (kkint32  classNum = 0;  classNum < classCount;  classNum++)
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



float  ConfusionMatrix2::AccuracyClassWeightedEqually ()
{
  kkint32  classCount    = classes.QueueSize ();
  float  totalAccuracy = 0.0f;

  for  (kkint32  classNum = 0;  classNum < classCount;  classNum++)
  {
    if  (countsByKnownClass [classNum] != 0)
    {
      float  classAccuracy = (float)(100.0f * (predictedCountsCM[classNum] [classNum])  / (countsByKnownClass [classNum]));
      totalAccuracy += classAccuracy;
    }
  }

  float  weightedAccuracy = (float)(totalAccuracy / classCount);

  return  weightedAccuracy;
}  /* AccuracyClassWeightedEqually */





double  ConfusionMatrix2::Count (MLClassPtr  mlClass)
{
  kkint32  classNum   = 0;
  bool   found      = false;
  kkint32  numClasses = classes.QueueSize ();

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
  kkint32  x;

  correctCount               *= factor;
  totalCount                 *= factor;
  totalPredProb              *= factor;
  numInvalidClassesPredicted *= factor;

  for  (x = 0; x < classCount;  x++)
  {
    countsByKnownClass         [x] = countsByKnownClass         [x] * factor;
    totalSizesByKnownClass     [x] = totalSizesByKnownClass     [x] * factor;
    totalPredProbsByKnownClass [x] = totalPredProbsByKnownClass [x] * factor;

    kkint32 y;

    for  (y = 0; y < classCount; y++)
    {
      predictedCountsCM[x][y] = predictedCountsCM[x][y] * factor;
      totPredProbCM    [x][y] = totPredProbCM    [x][y] * factor;
    }

    for  (y = 0; y < numOfBuckets; y++)
    {
      countByKnownClassBySize   [x][y] = countByKnownClassBySize   [x][y] * factor;
      correctByKnownClassBySize [x][y] = correctByKnownClassBySize [x][y] * factor; 
    }

    for  (y = 0; y < numOfProbBuckets; y++)
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
  kkint32 knownClassNum;
  kkint32 predClassNum;
  kkint32 x;

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
  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    file << "<th align=\"center\" bgcolor=\"#CCCCCC\"><b>" << classes[knownClassNum].Name() << "</b></th>" << endl;
  }
  file  << "</tr>" << endl;

  double *totals = new double[classCount];
  for  (x = 0; x < classCount; x++)
  {
    totals[x] = 0;
  }

  double  totalNonNoise = 0;
  double  totalNonNoiseRight = 0;

  // output the data rows
  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    bool  noiseClass = classes[knownClassNum].UnDefined();
    for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
    {
      totals[predClassNum] += predictedCountsCM[knownClassNum] [predClassNum];
    }
     
    file << "<tr>" << endl;
    file << "<th align=\"center\" bgcolor=\"#CCCCCC\"><b>" << classes            [knownClassNum].Name() << "</b></th>" << endl;
    file << "<td align=\"center\" bgcolor=\"#EFEFEF\">"    << countsByKnownClass [knownClassNum]        << "</td>"     << endl;

    for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
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
  
  for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
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
  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    file << "<th align=\"center\" bgcolor=\"#CCCCCC\"><b>" << classes[knownClassNum].Name() << "</b></th>" << endl;
  }
  file  << "</tr>" << endl;

  // output the data rows
  double perc=0.0;
  for  (knownClassNum = 0;  knownClassNum < classCount;  knownClassNum++)
  {
    file << "<tr>" << endl;
    file << "<th align=\"center\" bgcolor=\"#CCCCCC\"><b>" << classes[knownClassNum].Name() << "</b></th>" << endl;
    file << "<td align=\"center\" bgcolor=\"#EFEFEF\">" << setprecision (4) << (countsByKnownClass [knownClassNum]/totalCount*100) << "</td>" << endl;

    for  (predClassNum = 0; predClassNum < classCount; predClassNum++)
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

  delete  totals;
}   /* PrintConfusionMatrixHTML */



void   ConfusionMatrix2::AddIn (const ConfusionMatrix2&  cm)
{
  if  (classes != cm.classes)
  {
    // 'cm' is not for the same classes as thos confusion matrix.
    log.Level (-1) << endl << endl 
                   << "ConfusionMatrix2::AddIn      *** ERROR ***   Mismatched ImageClasses." << endl
                   << endl;
    osWaitForEnter ();
    exit (-1);
  }

  kkint32  numOfClasses = classes.QueueSize ();
  kkint32  classIDX = 0;


  //  Create indirection array to handle the situation where the mlClass list's of the two 
  // confusion matrixes '*this'  and  'cm'  are not in the same order.

  vector<kkint32>  ind (numOfClasses, 0);
  for  (classIDX = 0;  classIDX < numOfClasses;  classIDX++)
  {
    MLClassPtr mlClass = classes.IdxToPtr (classIDX);
    kkint32  cmsIDX = cm.classes.PtrToIdx (mlClass);
    ind[classIDX] = cmsIDX;
  }

  for  (classIDX = 0;  classIDX < numOfClasses;  classIDX++)
  {
    kkint32 cmsIDX = ind[classIDX];

    countsByKnownClass         [classIDX] += cm.countsByKnownClass         [cmsIDX];
    totalSizesByKnownClass     [classIDX] += cm.totalSizesByKnownClass     [cmsIDX];
    totalPredProbsByKnownClass [classIDX] += cm.totalPredProbsByKnownClass [cmsIDX];

    kkint32 predictedClassIDX = 0;
    for  (predictedClassIDX = 0;   predictedClassIDX  < numOfClasses;  predictedClassIDX++)
    {
      kkint32  cmsPredictedClassIDX = ind[predictedClassIDX];
      predictedCountsCM[classIDX][predictedClassIDX] += cm.predictedCountsCM[cmsIDX][cmsPredictedClassIDX];
      totPredProbCM    [classIDX][predictedClassIDX] += cm.totPredProbCM    [cmsIDX][cmsPredictedClassIDX];
    }

    kkint32  bucketIDX = 0;
    for  (bucketIDX = 0;  bucketIDX < numOfBuckets;  bucketIDX++)
    {
      countByKnownClassBySize   [classIDX][bucketIDX] += cm.countByKnownClassBySize   [cmsIDX][bucketIDX];
      correctByKnownClassBySize [classIDX][bucketIDX] += cm.correctByKnownClassBySize [cmsIDX][bucketIDX];
    }

    kkint32  probIDX = 0;
    for  (probIDX = 0;  probIDX < numOfProbBuckets;  probIDX++)
    {
      countByKnownClassByProb   [classIDX][probIDX] += cm.countByKnownClassByProb   [cmsIDX][probIDX];
      correctByKnownClassByProb [classIDX][probIDX] += cm.correctByKnownClassByProb [cmsIDX][probIDX];
    }
  }

  correctCount  += cm.correctCount;
  totalCount    += cm.totalCount;
  totalPredProb += cm.totalPredProb;
}  /* AddIn */



template<typename T>
KKStr  ArrayToDelimitedDelimitedStr (T*     _array,  
                                     kkint32  _count,
                                     char   _delimiter
                                    )
{
  kkint32  x = _count * 10;
  KKStr s (x);

  for  (kkint32 x = 0;  x < _count;  x++)
  {
    if  (x > 0)
      s.Append (_delimiter);
    s << _array[x];
  }
  return s;
}  /* ArrayToDelimitedDelimitedStr */




void   DelimitedStrToArray (kkint32*      _array,
                            kkint32       _count, 
                            const KKStr&  _l,
                            char          _delimiter
                           )
{
  VectorKKStr  fields = _l.Split (_delimiter);
  kkint32  lastField = Min ((kkint32)fields.size (), _count);
  for  (kkint32 idx = 0;  idx < lastField;  idx++)
    _array[idx] = fields[idx].ToInt ();
}  /* DelimitedStrToArray */




void   DelimitedStrToArray (double*       _array,
                            kkint32       _count, 
                            const KKStr&  _l,
                            char          _delimiter
                           )
{
  VectorKKStr  fields = _l.Split (_delimiter);
  kkint32  lastField = Min ((kkint32)fields.size (), _count);
  for  (kkint32 idx = 0;  idx < lastField;  idx++)
    _array[idx] = fields[idx].ToDouble ();
}  /* DelimitedStrToArray */





void  ConfusionMatrix2::WriteXML (ostream&  f)  const
{
  f << "<ConfusionMatrix2>" << endl;

  f << "Classes"      << "\t" << classes.ToCommaDelimitedStr () << endl;

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

  f << "CountsByKnownClass"          << "\t" << ArrayToDelimitedDelimitedStr (countsByKnownClass,          classCount, ',') << endl;
  f << "TotalSizesByKnownClass"      << "\t" << ArrayToDelimitedDelimitedStr (totalSizesByKnownClass,      classCount, ',') << endl;
  f << "TotalPredProbsByKnownClass"  << "\t" << ArrayToDelimitedDelimitedStr (totalPredProbsByKnownClass,  classCount, ',') << endl;


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

  kkint32  startPos = (kkint32)f.tellg ();
  MLClassListPtr  classes = NULL;

  kkint32  bucketSize        = -1;
  kkint32  classCount        = -1;
  kkint32  numOfBuckets      = -1;
  kkint32  numOfProbBuckets  = -1;
  kkint32  probBucketSize    = -1;

  //bucketSize = -1;

  f.getline (buff, sizeof (buff));
  while  ((!f.eof ())  &&  ((!classes)  ||  (bucketSize < 1)  ||  (numOfBuckets < 1)  ||  (numOfProbBuckets < 1)  ||  (probBucketSize < 1)  ||  (classCount < 1)))
  {
    KKStr  l (buff);
    l.TrimLeft ();

    if  (l.CompareIgnoreCase ("</ConfusionMatrix2>") == 0)
      break;

    KKStr  lineName = l.ExtractToken2 ("\t");
    if  (lineName.CompareIgnoreCase ("Classes") == 0)
      classes = MLClassList::BuildListFromDelimtedStr (l, '\t');

    else if  (lineName.CompareIgnoreCase ("bucketSize") == 0)
      bucketSize = l.ExtractTokenInt ("\t\n\r");

    else if  (lineName.CompareIgnoreCase ("classCount") == 0)
      classCount = l.ExtractTokenInt ("\t\n\r");

    else if  (lineName.CompareIgnoreCase ("numOfBuckets") == 0)
      numOfBuckets = l.ExtractTokenInt ("\t\n\r");

    else if  (lineName.CompareIgnoreCase ("numOfProbBuckets") == 0)
      numOfProbBuckets = l.ExtractTokenInt ("\t\n\r");

    else if  (lineName.CompareIgnoreCase ("probBucketSize") == 0)
      probBucketSize = l.ExtractTokenInt ("\t\n\r");

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




void   ConfusionMatrix2::Read (istream& f)
{
  if  (f.eof ())
  {
    log.Level (-1) << "ConfusionMatrix2::Read   ***ERROR***   File at EOF   can not read any data." << endl;
    return;
  }

  char  buff[10240];
  buff[0] = 0;
  KKStr l  (512);

  kkint32  numOfBuckets      = -1;
  kkint32  numOfProbBuckets  = -1;

  kkint32  classIndex = 0;
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
      
    else if  (lineName.CompareIgnoreCase ("BucketSize") == 0)
      bucketSize = l.ExtractTokenInt ("\t");
      
    else if  (lineName.CompareIgnoreCase ("probBucketSize") == 0)
      bucketSize = l.ExtractTokenInt ("\t");
    
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
      classIndex = l.ExtractTokenInt ("\t");

      if  ((classIndex < 0)  ||  (classIndex >= classCount))
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
  // "Estimating the Taxonomic composition of a sample when individules are classified with error"
  // by Andrew Solow, Cabll Davis, Qiao Hu
  // Woods Hole Ocanographic Institution, Woods Hole Massachusetts
  // Marine Ecology Progresss Series
  // published 2006-july-06
  // vol 216:309-311
  
  // This data is ment to work with "ClassificationStatus.cs" to prvide the data nessasary to 
  // adjust for bias.

  f << "<SimpleConfusionMatrix>" << endl;
  f << "Classes"      << "\t" << classes.ToCommaDelimitedStr () << endl;
  kkint32  row = 0;
  kkint32  col = 0;
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




ConfusionMatrix2Ptr  ConfussionMatrix2List::DeriveAverageConfusionMatrix ()  const
{
  if  (QueueSize () == 0)
  {
    return NULL;
  }

  const_iterator  cmIDX = begin ();
  const ConfusionMatrix2Ptr  firstCM = *cmIDX;

  RunLog&  log = firstCM->Log ();

  ConfusionMatrix2Ptr  meanCM = new ConfusionMatrix2 (firstCM->ImageClasses (), log);

  for  (cmIDX = begin ();  cmIDX != end ();  cmIDX++)
  {
    const ConfusionMatrix2Ptr  cm = *cmIDX;
    meanCM->AddIn (*cm); 
  }

  double  factor = 1.0 / (double)QueueSize ();

  meanCM->FactorCounts (factor);

  return  meanCM;
}  /* DeriveAverageConfusionMatrix */

