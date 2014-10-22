#pragma once

#include "DateTime.h"
#include "KKStr.h"
#include "Matrix.h"
#include "RunLog.h"

#include "ConfusionMatrix2.h"
#include "MLClass.h"



//  Job in Life:   This module is the Managed version of "DataBaseServer".
//
// See the paper: 
//    "Estimating the Taxonomic composition of a sample when individules are classified with error"
//     by Andrew Solow, Cabll Davis, Qiao Hu
//     Woods Hole Ocanographic Institution, Woods Hole Massachusetts
//     Marine Ecology Progresss Series
//     published 2006-july-06;  vol 216:309-311


namespace KKMachineLearning 
{
  /**
   *@class  ClassificationBiasMatrix
   *@brief Assists in adjusting a Classifiers output for bias of a classifier.
   *@details
   *See the paper: 
   *@code
   *    "Estimating the Taxonomic composition of a sample when individuals are classified with error"
   *    by Andrew Solow, Cabll Davis, Qiao Hu
   *    Woods Hole Ocanographic Institution, Woods Hole Massachusetts
   *    Marine Ecology Progresss Series
   *    published 2006-july-06;  vol 216:309-311
   *@endcode
   *This class uses the results of a previous 10 fold cross validation confusion matrix to adjust the results of a
   *classification run.  The application CrossValidation will create a parameter file in the directory 
   *"${LarcosHomeDir}\\Classifier\\TrainingModels\\Save" that contains the confusion matrix.  The option "-BiasMatrix" 
   *tells the CrossValidation program to generate this parameter file.  It will have the same name as the Configuration
   *file with the extension ".BiasMatrix.txt" Appended to the end.
  */
  class  ClassificationBiasMatrix
  {
  public:
    typedef  ClassificationBiasMatrix*  ClassificationBiasMatrixPtr;

    typedef  KKB::RunLog      RunLog;
    typedef  KKB::KKStr       KKStr;
    typedef  KKB::DateTime    DateTime;
    typedef  KKB::Matrix      Matrix;
    typedef  KKB::MatrixPtr   MatrixPtr;


    /**
     *@brief  Will construct a ClassificationBiasMatrix instance from a input stream.
     *@param[in]      f        Input Stream.
     *@param[in]      classes  List of classes we expect to find in the input stream f.
     *@param[in,out]  log      Logging File.
     */
    static
    ClassificationBiasMatrixPtr  BuildFromIstreamXML (istream&         f,
                                                      MLClassList&  classes,
                                                      RunLog&          log
                                                     );


    /**
     *@brief Will construct an instance as defined in the Paper. 
     */
    ClassificationBiasMatrix (RunLog&  _runLog);   


    ClassificationBiasMatrix (const ClassificationBiasMatrix &  cbm);

    ClassificationBiasMatrix (MLClassList&  classes,
                              RunLog&          _runLog
                             );


    /**
     *@brief  Construct a ClassificationBiasMatrix instance from from the parameter file for '_configFileName'.
     *@param[in]     _configFileName The name of the BiasMatrix file will be derived from the configuration 
     *                               file name.  the term '_BiasMatrix.txt' will be appended onto the end.
     *@param[in]     _classes        List of classes that you are expecting to be in the 'BiasMatrix' file.
     *@param[in,out] _runLog         Logging file to use.
     */
    ClassificationBiasMatrix (const KKStr&     _configFileName,
                              MLClassList&  _classes,          // List of classes you expect to find in BiasMatrix file.
                              RunLog&          _runLog
                             );

    /**
     *@brief  Construct a ClassificationBiasMatrix instance from a ConfusionMatrix object.
     *@param[in]      cm        Confusion Matrix to get parameters from. 
     *@param[in,out]  _runLog   Logging file to use.
     */
    ClassificationBiasMatrix (const ConfusionMatrix2&  cm,
                              RunLog&                  _runLog
                             );

    ~ClassificationBiasMatrix ();

    DateTime  DateTimeFileWritten ()  const  {return  dateTimeFileWritten;}


    void   PerformAdjustmnts (const VectorDouble&  classifiedCounts,
                              VectorDouble&        adjCounts,
                              VectorDouble&        stdErrors
                             );

    void   PrintBiasMatrix (ostream&  sw);

    void   PrintAdjustedResults (ostream&              sw,
                                 const VectorDouble&   classifiedCounts
                                );

    void   TestPaperResults (ostream&    sw);

    void   WriteXML (ostream&  o);


  private:
    void  BuildFromConfusionMatrix (const ConfusionMatrix2&  cm);
    void  BuildTestMatrix ();
    void  DeclareMatrix ();
    void  ReadXML (istream&   r);
    void  ReadSimpleConfusionMatrix (istream&        sr,
                                     MLClassListPtr  fileClasses
                                    );
   

    KKStr           biasFileName;
    MLClassListPtr  classes;
    KKStr           configFileName;
    KKStr           configFileNameFromMatrixBiasFile;
    DateTime        configDateTime;       /**< TimeStamp of config file when this BiasMatrix was written. */
    MatrixPtr       counts;
    DateTime        dateTimeFileWritten;
    VectorKKStr     errMsgs;
    kkint32         numClasses;
    MatrixPtr       probabilities;
    RunLog&         runLog;
    bool            valid;
  };

  typedef  ClassificationBiasMatrix::ClassificationBiasMatrixPtr  ClassificationBiasMatrixPtr;
}  /*  namespace KKMachineLearning  */
