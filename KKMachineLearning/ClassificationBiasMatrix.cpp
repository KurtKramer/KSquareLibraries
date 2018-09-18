#include "FirstIncludes.h"
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <vector>
#include "MemoryDebug.h"
using namespace  std;

#include "KKBaseTypes.h"
#include "KKException.h"
#include "KKStr.h"
#include "Matrix.h"
#include "OSservices.h"
#include "RunLog.h"
using namespace  KKB;

#include "MLClass.h"
using namespace  KKMLL;

#include "ClassificationBiasMatrix.h"



ClassificationBiasMatrix::ClassificationBiasMatrix (const KKStr&  _configFileName,
                                                    MLClassList&  _classes,
                                                    RunLog&       _runLog
                                                   ):
   biasFileName        (),
   classes             (new MLClassList (_classes)),
   configFileName      (_configFileName),
   configFileNameFromMatrixBiasFile (),
   configDateTime      (),
   counts              (NULL),
   dateTimeFileWritten (),
   errMsgs             (),
   numClasses          (0),
   probabilities       (NULL),
   runLog              (_runLog),
   valid               (true)
{
  biasFileName = osRemoveExtension (configFileName) + ".BiasMatrix.txt";

  ifstream  sr (biasFileName.Str ());

  if  (!sr.is_open ())
  {
    valid = false;
    KKStr  errorMsg = "Error opening ConfigFileName[" + configFileName + "]";
    errMsgs.push_back (errorMsg);
    return;
  }

  DeclareMatrix ();

  try
  {
    ReadXML (sr);
  }
  catch  (const exception& e)
  {
    valid = false;
    errMsgs.push_back (e.what ());
  }
  catch (...)
  {
    valid = false;
    errMsgs.push_back ("ClassificationBiasMatrix    Exception occurred reading XML File");
  }
  sr.close ();
}



ClassificationBiasMatrix::ClassificationBiasMatrix (const ClassificationBiasMatrix &  cbm):
   biasFileName        (cbm.biasFileName),
   classes             (NULL),
   configFileName      (cbm.configFileName),
   configFileNameFromMatrixBiasFile (cbm.configFileNameFromMatrixBiasFile),
   configDateTime      (cbm.configDateTime),
   counts              (NULL),
   dateTimeFileWritten (cbm.dateTimeFileWritten),
   errMsgs             (),
   numClasses          (cbm.numClasses),
   probabilities       (NULL),
   runLog              (cbm.runLog),
   valid               (true)

{
  if  (cbm.counts)
    counts = new MatrixD (*cbm.counts);

  if  (cbm.classes)
    classes = new MLClassList (*cbm.classes);

  if  (cbm.probabilities)
    probabilities = new MatrixD (*cbm.probabilities);
}




 ClassificationBiasMatrix::ClassificationBiasMatrix (const ConfusionMatrix2&  cm,
                                                     RunLog&                  _runLog
                                                    ):
   biasFileName        (),
   classes             (NULL),
   configFileName      (),
   configFileNameFromMatrixBiasFile (),
   configDateTime      (),
   counts              (NULL),
   dateTimeFileWritten (),
   errMsgs             (),
   numClasses          (0),
   probabilities       (NULL),
   runLog              (_runLog),
   valid               (true)
{
  classes = new MLClassList (cm.MLClasses ());
  numClasses = classes->QueueSize ();
  DeclareMatrix ();
  BuildFromConfusionMatrix (cm);
}



ClassificationBiasMatrix::ClassificationBiasMatrix (MLClassList&  _classes,
                                                    RunLog&          _runLog
                                                   ):
   biasFileName        (),
   classes             (new MLClassList (_classes)),
   configFileName      (),
   configFileNameFromMatrixBiasFile (),
   configDateTime      (),
   counts              (NULL),
   dateTimeFileWritten (),
   errMsgs             (),
   numClasses          (0),
   probabilities       (NULL),
   runLog              (_runLog),
   valid               (true)

{
  numClasses = classes->QueueSize ();
  DeclareMatrix ();
}



ClassificationBiasMatrix::ClassificationBiasMatrix (RunLog&  _runLog):
   biasFileName        (),
   classes             (new MLClassList ()),
   configFileName      (),
   configFileNameFromMatrixBiasFile (),
   configDateTime      (),
   counts              (NULL),
   dateTimeFileWritten (),
   errMsgs             (),
   numClasses          (0),
   probabilities       (NULL),
   runLog              (_runLog),
   valid               (true)
{
  BuildTestMatrix ();

  //ofstream  sw ("c:\\Temp\\ClassificationBiasMatrix_Test.txt");
  //TestPaperResults (sw);
  //sw.close ();
}



ClassificationBiasMatrix::~ClassificationBiasMatrix ()
{
  delete  probabilities;
  probabilities = NULL;
  delete  counts;
  counts = NULL;
  delete  classes;
  classes = NULL;
}



ClassificationBiasMatrixPtr  ClassificationBiasMatrix::BuildFromIstreamXML (istream&         f,
                                                                            MLClassList&  classes,
                                                                            RunLog&          log
                                                                           )
{
  ClassificationBiasMatrixPtr cbm = new ClassificationBiasMatrix (classes, log);
  cbm->ReadXML (f);
  return  cbm;
}  /* BuildFromIstreamXML */



void  ClassificationBiasMatrix::DeclareMatrix ()
{
  numClasses = classes->QueueSize ();
  probabilities = new MatrixD (numClasses, numClasses);
  counts        = new MatrixD (numClasses, numClasses);
  for  (kkuint32 r = 0;  r < numClasses;  r++)
  {
    for  (kkuint32 c = 0;  c < numClasses;  c++)
    {
      (*probabilities)[r][c] = 0.0;
      (*counts)[r][c] = 0.0;
    }
  }
}  /* DeclareMatrix */



void  ClassificationBiasMatrix::BuildTestMatrix ()
{
  classes = new MLClassList ();

  classes->PushOnBack (MLClass::CreateNewMLClass ("0"));
  classes->PushOnBack (MLClass::CreateNewMLClass ("1"));
  classes->PushOnBack (MLClass::CreateNewMLClass ("2"));
  classes->PushOnBack (MLClass::CreateNewMLClass ("3"));
  classes->PushOnBack (MLClass::CreateNewMLClass ("4"));
  classes->PushOnBack (MLClass::CreateNewMLClass ("5"));
  classes->PushOnBack (MLClass::CreateNewMLClass ("6"));

  numClasses = classes->QueueSize ();

  probabilities = new MatrixD (numClasses, numClasses);
  counts        = new MatrixD (numClasses, numClasses);

  auto& r0 = (*probabilities)[0];
  auto& r1 = (*probabilities)[1];
  auto& r2 = (*probabilities)[2];
  auto& r3 = (*probabilities)[3];
  auto& r4 = (*probabilities)[4];
  auto& r5 = (*probabilities)[5];
  auto& r6 = (*probabilities)[6];

  r0[0] = 0.710;  r0[1] = 0.059;  r0[2] = 0.010;  r0[3] = 0.010;  r0[4] = 0.007;  r0[5] = 0.031;  r0[6] = 0.175;
  r1[0] = 0.073;  r1[1] = 0.873;  r1[2] = 0.001;  r1[3] = 0.007;  r1[4] = 0.008;  r1[5] = 0.013;  r1[6] = 0.024;
  r2[0] = 0.078;  r2[1] = 0.012;  r2[2] = 0.556;  r2[3] = 0.035;  r2[4] = 0.066;  r2[5] = 0.179;  r2[6] = 0.074;
  r3[0] = 0.030;  r3[1] = 0.028;  r3[2] = 0.054;  r3[3] = 0.560;  r3[4] = 0.019;  r3[5] = 0.177;  r3[6] = 0.132;
  r4[0] = 0.205;  r4[1] = 0.054;  r4[2] = 0.107;  r4[3] = 0.046;  r4[4] = 0.366;  r4[5] = 0.157;  r4[6] = 0.065;
  r5[0] = 0.158;  r5[1] = 0.025;  r5[2] = 0.076;  r5[3] = 0.064;  r5[4] = 0.175;  r5[5] = 0.449;  r5[6] = 0.054;
  r6[0] = 0.289;  r6[1] = 0.096;  r6[2] = 0.033;  r6[3] = 0.065;  r6[4] = 0.018;  r6[5] = 0.072;  r6[6] = 0.427;
}  /* BuildTestMatrix  */



void  ClassificationBiasMatrix::TestPaperResults (ostream&   sw)
{
  VectorDouble classCounts (7, 0.0);
  classCounts[0] = 2891;
  classCounts[1] = 1965;
  classCounts[2] = 495;
  classCounts[3] = 1399;
  classCounts[4] = 676; 
  classCounts[5] = 1191;
  classCounts[6] = 1752;

  VectorDouble   adjCounts; 
  VectorDouble   stdErrors;

  PerformAdjustmnts (classCounts, adjCounts, stdErrors);

  kkint32  x = 0;

  sw << endl << endl;

  PrintBiasMatrix (sw);

  sw << endl << endl;

  sw << "Desc";
  for  (x = 0;  x < 7;  x++)
    sw << "\t" << StrFormatInt (x, "ZZ0");
  sw << endl;

  sw << "ClassifiedCounts";
  for  (x = 0;  x < 7;  x++)
    sw << "\t" << StrFormatDouble (classCounts[x], "ZZ,ZZ0.0");
  sw << endl;

  sw << "adjCounts";
  for  (x = 0;  x < 7;  x++)
    sw  << "\t" << StrFormatDouble (adjCounts[x], "ZZZ,ZZ0.0");
  sw << endl;

  sw << "stdErrors";
  for  (x = 0;  x < 7;  x++)
    sw << "\t" << StrFormatDouble (stdErrors[x], "ZZ,ZZ0.0");

  sw << endl << endl;
}  /* TestPaperResults*/



void  ClassificationBiasMatrix::ReadXML (istream&  sr)
{
  char  buff[10240];
  KKStr l (512);

  if  (sr.eof ())
    return;

  MLClassListPtr  fileClasses = NULL;
 
  sr.getline (buff, sizeof (buff));
  while  (!sr.eof ())
  {
    l = buff;
    l.TrimRight ();
    if  (l.CompareIgnoreCase ("</ClassificationBiasMatrix>") == 0)
      break;

    KKStr  lineName = l.ExtractToken2 ("\t");
    if  (!lineName.Empty ())
    {
      KKStr  fieldValue = l.ExtractToken2 ("\t");
      
      if  (lineName.CompareIgnoreCase ("Classes") == 0)
      {
        delete  fileClasses;  fileClasses = NULL;
        fileClasses = MLClassList::BuildListFromDelimtedStr (fieldValue, ',');
        if  (classes == NULL)
          classes = new MLClassList (*fileClasses);
      }

      else if  (lineName.CompareIgnoreCase ("ConfigDateTime") == 0)
      {
        configDateTime = fieldValue;
      }

      else if  (lineName.CompareIgnoreCase ("ConfigFileName") == 0)
      {
        configFileNameFromMatrixBiasFile = fieldValue;
      }

      else if  (lineName.CompareIgnoreCase ("ConfigFileDateTime") == 0)
      {
        configDateTime = fieldValue;
      }

      else if  (lineName.CompareIgnoreCase ("DateTime") == 0)
      {
        dateTimeFileWritten = fieldValue;
      }

      else if  (lineName.CompareIgnoreCase ("DateTimeFileWritten") == 0)
      {
        dateTimeFileWritten = fieldValue;
      }

      else if  (lineName.CompareIgnoreCase ("FileName") == 0)
      {
      }

      else if  (lineName.CompareIgnoreCase ("NumClasses") == 0)
      {
        numClasses = fieldValue.ToUint32 ();
      }

      else if  (lineName.CompareIgnoreCase ("<SimpleConfusionMatrix>") == 0)
      {
        ReadSimpleConfusionMatrix (sr, fileClasses);
      } 
    }

    if  (!sr.eof ())
      sr.getline (buff, sizeof (buff));
  }
}  /* ReadXML */




void  ClassificationBiasMatrix::ReadSimpleConfusionMatrix (istream&           sr,
                                                           MLClassListPtr  fileClasses
                                                          )
{
  //  'classes'     - The class order that the owner of this object is expecting.
  //  'fileClasses' - The order that the classes are stored in the text file.


  if  ((classes == NULL)  ||  (fileClasses == NULL))
  {
    KKStr  errMsg = "ReadSimpleConfusionMatrix   ***ERROR***  The 'Classes'  line was never provided.";
    runLog.Level (-1) << errMsg << endl;
    valid = false;
    throw KKException (errMsg);
  }

  kkint32  classesColIdx = 0;

  char  buff[10240];
  KKStr  l;
  while  (!sr.eof ())
  {
    sr.getline (buff, sizeof (buff));
    l = buff;
    l.TrimLeft ();
    l.TrimRight ();

    if  (l.CompareIgnoreCase ("</SimpleConfusionMatrix>") == 0)
      break;

    KKStr  lineName = l.ExtractToken2 ("\t");

    if  (lineName.CompareIgnoreCase ("DataRow") == 0)
    {
      if  (fileClasses == NULL)
      {
        KKStr  errMsg = "ReadSimpleConfusionMatrix   ***ERROR***  'Classes'  was not provided before 'DataRow'.";
        runLog.Level (-1) << errMsg << endl;
        valid = false;
        throw KKException (errMsg);
      }

      KKStr  className = l.ExtractToken2 ("\t");
      KKStr  data      = l.ExtractToken2 ("\t");

      MLClassPtr  pc = MLClass::CreateNewMLClass (className);
      kkint32  classesIdx     = classes->PtrToIdx (pc);
      kkint32  fileClassesIdx = fileClasses->PtrToIdx (pc);

      if  (classesIdx < 0)
      {
        KKStr  errMsg = "ReadSimpleConfusionMatrix   ***ERROR***  DataRow specifies class[" + className + "] which is not defined by caller";
        runLog.Level (-1) << errMsg << endl;
        valid = false;
        throw KKException (errMsg);
      }

      if  (fileClassesIdx < 0)
      {
        KKStr errMsg = "ReadSimpleConfusionMatrix   ***ERROR***  DataRow specifies class[" + className + "] was not defined in 'Classes' line.";
        runLog.Level (-1) << errMsg << endl;
        valid = false;
        throw KKException (errMsg);
      }

      kkint32  classesRowIdx = classesIdx;

      VectorKKStr  dataFields = data.Split (',');
      if  (dataFields.size () != numClasses)
      {
        KKStr  errMsg = "ReadSimpleConfusionMatrix   ***ERROR***  DataRow Class[" + className + "]  number[" + StrFormatInt ((kkint32)dataFields.size (), "ZZZ0") + "] of values provided does not match number of Classes.";
        runLog.Level (-1) << errMsg << endl;
        valid = false;
        throw KKException (errMsg);
      }

      for  (kkuint32 c = 0;  c < numClasses;  c++)
      {
        pc = fileClasses->IdxToPtr (c);
        classesColIdx = classes->PtrToIdx (pc);

        VectorKKStr   parts = dataFields[c].Split (':');
        if  (parts.size () > 1)
        {
          (*counts)       [classesRowIdx][classesColIdx] = parts[0].ToDouble ();
          (*probabilities)[classesRowIdx][classesColIdx] = parts[1].ToDouble ();
        }
      }
    }
  }
}  /* ReadSimpleConfusionMatrix */



void  ClassificationBiasMatrix::WriteXML (std::ostream&  o)
{
  o << "<ClassificationBiasMatrix>" << std::endl;

  o << "BiasFileName" << "\t" << biasFileName << std::endl;

  if  ((!classes)  ||  (!counts)  ||  (!probabilities))
  {
    runLog.Level (-1) << "ClassificationBiasMatrix::WriteXML    ***ERROR***   Not all data is defined." << endl;
    return;
  }

  if  ((numClasses != classes->QueueSize ())  ||  
       (numClasses != counts->NumOfRows  ())  ||  
       (numClasses != probabilities->NumOfRows ())
      )
  {
    runLog.Level (-1) << "ClassificationBiasMatrix::WriteXML    ***ERROR***   Disagreement in variable dimensions." << endl;
    return;
  }

  o << "NumClasses"          << "\t" << numClasses                      << std::endl;
  o << "Classes"             << "\t" << classes->ToCommaDelimitedStr () << std::endl;
  o << "ConfigFileName"      << "\t" << configFileName                  << std::endl;
  o << "ConfigDateTime"      << "\t" << configDateTime                  << std::endl;
  o << "DateTimeFileWritten" << "\t" << dateTimeFileWritten             << std::endl;

  o << "<SimpleConfusionMatrix>" << std::endl;
  for  (kkuint32 rowIdx = 0;  rowIdx < numClasses;  rowIdx++)
  {
    o << "DataRow"                  << "\t" 
      << (*classes)[rowIdx].Name () << "\t";

    for  (kkuint32 colIdx = 0;  colIdx < numClasses;  colIdx++)
    {
      if  (colIdx > 0)
        o << ",";
      o << ((*counts)[rowIdx][colIdx]) << ":" << StrFormatDouble ((*probabilities)[rowIdx][colIdx], "ZZ0.0000");
    }
    o << std::endl;
  }

  o << "</SimpleConfusionMatrix>" << std::endl;

  o << "</ClassificationBiasMatrix>"  << std::endl;

}  /* WriteXML */



void  ClassificationBiasMatrix::BuildFromConfusionMatrix (const ConfusionMatrix2&  cm)
{
  kkuint32  classesRowIdx = 0;
  kkuint32  classesColIdx = 0;

  for  (classesRowIdx = 0;  classesRowIdx < numClasses;  classesRowIdx++)
  {
    double  knownCount = cm.CountsByKnownClass (classesRowIdx);

    for  (classesColIdx = 0;  classesColIdx < numClasses;  classesColIdx++)
    {
      double  predCount = cm.PredictedCountsCM (classesRowIdx, classesColIdx);
      double  prob = 0.0;
      if  (knownCount != 0.0)
        prob = predCount / knownCount;

      (*counts)       [classesRowIdx][classesColIdx] = predCount;
      (*probabilities)[classesRowIdx][classesColIdx] = prob;
    }
  }
}  /* BuildFromConfusionMatrix*/



void   ClassificationBiasMatrix::PerformAdjustmnts (const VectorDouble&  classifiedCounts,
                                                    VectorDouble&        adjCounts,
                                                    VectorDouble&        stdErrors
                                                   )
{
  // For description of calc's see the paper: 
  //    "Estimating the Taxonomic composition of a sample when individuals are classified with error"
  //     by Andrew Solow, Cabll Davis, Qiao Hu
  //     Woods Hole Oceanographic Institution, Woods Hole Massachusetts
  //     Marine Ecology Progress Series
  //     published 2006-july-06;  vol 216:309-311

  if  (classifiedCounts.size () != numClasses)
  {
    KKStr errMsg = "ClassificationBiasMatrix::PerformAdjustmnts  ***ERROR***   Disagreement in length of classifiedCounts[" + 
                   StrFormatInt ((kkint32)classifiedCounts.size (), "ZZZ0") + 
                   "]  and Prev Defined ClassList[" + StrFormatInt (numClasses, "ZZZ0") + "].";
    runLog.Level (-1) << errMsg << endl;
    valid = false;
    throw KKException (errMsg);
  }

  // We need to deal with the special case when one entry in the probability diagonal is zero.
  {
    for (kkuint32 x = 0;  x < numClasses;  x++)
    {
      if  ((*probabilities)[x][x] == 0.0)
      {
        // This will cause the inversion of the diagonal matrix to fail.  To deal
        // with this situation; I will steal some probability from other buckets on 
        // same row.

        double  totalAmtStolen = 0.0;
        double  percentToSteal = 0.01;
        for (kkuint32 i = 0;  i < numClasses;  i++)
        {
          if  ((*probabilities)[x][i] != 0.0)
          {
            double amtToSteal = (*probabilities)[x][i] * percentToSteal;
            (*probabilities)[x][i] = (*probabilities)[x][i] - amtToSteal;
            totalAmtStolen += amtToSteal;
          }
        }

        (*probabilities)[x][x] = totalAmtStolen;
      }
    }
  }

  MatrixD  m (numClasses, 1);
  for  (kkuint32 x = 0;  x < numClasses;  x++)
    m[x][0] = classifiedCounts[x];

  MatrixD  transposed = probabilities->Transpose ();
  MatrixD  Q = transposed.Inverse ();
  MatrixD  n = Q * m;

  MatrixD  varM (numClasses, numClasses);
  for  (kkuint32 j = 0;  j < numClasses;  j++)
  {
    double  varM_j = 0.0;
    for  (kkuint32 i = 0;  i < numClasses;  i++)
    {
      double  p = (*probabilities)[i][j];
      varM_j += n[i][0] * p * (1.0 - p);
    }
    varM[j][j] = varM_j;
  }

  for (kkuint32 j = 0;  j < numClasses;  j++)
  {
    for  (kkuint32 k = 0;  k < numClasses;  k++)
    {
      if  (j != k)
      {
        double  covM_jk = 0.0;
        for  (kkuint32 i = 0;  i < numClasses;  i++)
          covM_jk -= n[i][0] * (*probabilities)[i][j] * (*probabilities)[j][k];
        varM[j][k] = covM_jk;
      }
    }
  }

  MatrixD  varN = Q * varM * Q.Transpose ();

  adjCounts.clear ();
  stdErrors.clear ();
    
  for  (kkuint32 x = 0;  x < numClasses;  x++)
  {
    adjCounts.push_back (n[x][0]);
    stdErrors.push_back (sqrt (varN[x][x]));
  }

  return;
}  /* PerformAdjustmnts */



void  ClassificationBiasMatrix::PrintBiasMatrix (ostream& sw)
{
  if  (classes == NULL)
  {
    KKStr  errMsg = "ClassificationBiasMatrix::PrintBiasMatrix  ***ERROR***   'Classes' not defined;  this indicates that this object was not properly initialized.";
    runLog.Level (-1) << errMsg << endl;
    valid = false;
    throw KKException (errMsg);
  }

  sw << "BiasMatrix File Name            [" << biasFileName                     << "]" << endl
     << "Date Bias Matrix was Created    [" << dateTimeFileWritten              << "]" << endl
     << "Configuration File Name         [" << configFileName                   << "]" << endl
     << "ConfigFile Used for Bias Matrix [" << configFileNameFromMatrixBiasFile << "]" << endl
     << endl
     << endl;

  KKStr  tl1, tl2, tl3;

  classes->ExtractThreeTitleLines (tl1, tl2, tl3);
  {
    sw << ""      << "\t" << ""      << "\t" << tl1 << endl;
    sw << "Class" << "\t" << ""      << "\t" << tl2 << endl;
    sw << "Name"  << "\t" << "Count" << "\t" << tl3 << endl;
  }

  double  total = 0.0;

  VectorDouble  colTotals (numClasses, 0.0);
  VectorDouble  rowTotals (numClasses, 0.0);

  for  (kkuint32 row = 0;  row < numClasses;  row++)
  {
    double  rowTotal = 0;
    for  (kkuint32 col = 0;  col < numClasses;  col++)
    {
      rowTotal += (*counts)[row][col];
      colTotals[col] += (*counts)[row][col];
    }
    rowTotals[row] = rowTotal;

    sw << (*classes)[row].Name () << "\t" << StrFormatDouble (rowTotal, "zzz,zz0.00");
    for  (kkuint32 col = 0;  col < numClasses;  col++)
      sw << "\t" << StrFormatDouble ((*counts)[row][col], "###,##0.00");
    sw << endl;
    total += rowTotal;
  }
  sw << "Total" << "\t" << StrFormatDouble (total, "###,##0.00");
  for  (kkuint32 col = 0;  col < numClasses;  ++col)
     sw << "\t" << StrFormatDouble (colTotals[col], "ZZZ,ZZ0.00");
  sw << endl;
 
  sw << endl;
 
  for  (kkuint32 row = 0; row < numClasses;  ++row)
  {
    sw << (*classes)[row].Name ();
    double  rowPercent = 100.0 * rowTotals[row] / total;
    sw << "\t" << StrFormatDouble (rowPercent, "ZZ0.0000") + "%";

    for  (kkuint32 col = 0;  col < numClasses;  ++col)
      sw << "\t" << StrFormatDouble (100.0 * (*probabilities)[row][col], "ZZZ,ZZ0.00") << "%";
    sw << endl;
  }
  sw << endl;
}  /* PrintBiasMatrix */



void  ClassificationBiasMatrix::PrintAdjustedResults (ostream&             sw,
                                                      const VectorDouble&  classifiedCounts
                                                     )
{
  if  (classifiedCounts.size () != (kkuint32)numClasses)
  {
    KKStr  errMsg = "ClassificationBiasMatrix::PrintAdjustedResults  ***ERROR***    Number of entries in 'classifiedCounts' not equal the number of classes";
    cerr << "ClassificationBiasMatrix::PrintAdjustedResults  ***ERROR***  " << errMsg << endl;
    valid = false;
    throw KKException (errMsg);
  }

  try
  {
    VectorDouble   adjustedReults;
    VectorDouble   stdErrors;
    
    PerformAdjustmnts (classifiedCounts, adjustedReults, stdErrors);

    KKStr  tl1, tl2, tl3;
    classes->ExtractThreeTitleLines (tl1, tl2, tl3);
    sw << ""             << "\t" << "\t" << tl1 << endl;
    sw << ""             << "\t" << "\t" << tl2 << endl;
    sw << "Description"  << "\t" << "\t" << tl3 << endl;
   
    sw << "Classified Results" << "\t";
    for  (kkuint32 col = 0;  col < numClasses;  col)
      sw << "\t" << StrFormatDouble (classifiedCounts[col], "Z,ZZZ,ZZ0.0");
    sw << endl;

    sw << "Adjusted Results" << "\t";
    for  (kkuint32 col = 0;  col < numClasses;  col)
      sw << "\t" << StrFormatDouble (adjustedReults[col], "Z,ZZZ,ZZ0.0");
    sw << endl;

    sw << "Standard Errors" << "\t";
    for  (kkuint32 col = 0;  col < numClasses;  ++col)
      sw << "\t" << StrFormatDouble (stdErrors[col], "Z,ZZZ,ZZ0.0");
    sw << endl;
  }
  catch  (KKException&  e)
  {
    KKStr  errMsg = "ClassificationBiasMatrix::PrintAdjustedResults   ***ERROR***  KKException";
    runLog.Level (-1) << errMsg << endl << e.ToString () << endl;
    throw KKException (errMsg, e);
  }
  catch  (std::exception& e2)
  {
    KKStr  errMsg = "ClassificationBiasMatrix::PrintAdjustedResults   ***ERROR***  std::exception";
    runLog.Level (-1) << errMsg << endl << e2.what() << endl;
    throw KKException (errMsg, e2);
  }
  catch  (...)
  {
    KKStr  errMsg = "ClassificationBiasMatrix::PrintAdjustedResults   ***ERROR***  Exception(...)";
    runLog.Level (-1) << endl << errMsg << endl;
    throw KKException (errMsg);
  }
}  /* PrintAdjustedResults */
