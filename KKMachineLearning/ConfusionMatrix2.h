#ifndef  _CONFUSIONMATRIX2_
#define  _CONFUSIONMATRIX2_

//***************************************************************************
//* Written by: Kurt Kramer                                                 *
//*        For: Research Work, Plankton recognition System                  *
//*                                                                         *
//*-------------------------------------------------------------------------*
//*  History                                                                *
//*                                                                         *
//*  Prog       Date      Description                                       *
//*  ------- -----------  ------------------------------------------------- *
//*  Kurt    Oct-19-2002  Increment Will now take ImageClasses instead of   *
//*                       numbers.  We will also make a unique copy of      *
//*                       mlClassesList.  This way we will not have to     *
//*                       worry abount the numbering in the classList       *
//*                       behind our back.                                  * 
//***************************************************************************
//* 


#include "MLClass.h"
#include "RunLog.h"
#include "KKStr.h"



namespace  KKMachineLearning
{

  /// <summary>
  /// A confusion matrix object that is used to record the results from a CrossValidation.
  /// <seealso cref="CrossValidation"
  /// </summary>
  class  ConfusionMatrix2
  {
  public:
    typedef  ConfusionMatrix2*  ConfusionMatrix2Ptr;

    ConfusionMatrix2 (const MLClassList&  _classes,  // Will make its own copy of '_classes'
                      RunLog&             _log
                     );
    
    
    ConfusionMatrix2 (const ConfusionMatrix2&  cm);


    //  Will construct an instance of 'ConfusionMatrix2'  from the contents of the 
    //  provided  'istream' object. 
    ConfusionMatrix2 (const MLClassList&  _classes,  // Will make its own copy of '_classes'
                      istream&            f,
                      kkint32             _bucketSize,
                      kkint32             _numOfBuckets,
                      kkint32             _numOfProbBuckets,
                      kkint32             _probBucketSize,
                      RunLog&             _log
                     );
                                           
    
    ~ConfusionMatrix2 ();

    double   Accuracy ();

    VectorFloat AccuracyByClass  ()  const;

    float    AccuracyNorm ()   {return AccuracyClassWeightedEqually ();}

    float    AccuracyClassWeightedEqually ();

    double   Accuracy (MLClassPtr  mlClass);

    KKStr    AccuracyStr ();

    void     AddIn (const ConfusionMatrix2&  cm);

    double   AvgPredProb ()  const;

    kkint32  ClassCount ()  const {return  classCount;}

    double   Count (MLClassPtr  mlClass);

    double   CountsByKnownClass (kkint32 knownClassIdx)  const;

    VectorDouble   CountsByKnownClass ()  const;

    void     FactorCounts (double  factor);    /**< Will multiply all counts by 'factor'  You would use this in conjunction with 'AddIn'.  */

    float    FMeasure (MLClassPtr  positiveClass)  const;

    const
    MLClassList&  ImageClasses () const  {return  classes;}

    void     Increment (MLClassPtr  _knownClass,
                        MLClassPtr  _predClass,
                        kkint32     _size,
                        double      _probability
                       );

    VectorDouble   PredictedCounts ()  const;

    double   PredictedCountsCM (kkint32 knownClassIdx, kkint32  predClassIdx)  const;

    void     PrintAccuracyByProbByClassHTML (ostream&  o);

    void     PrintConfusionMatrix (ostream&  _outFile);

    void     PrintConfusionMatrixAvgPredProbHTML (ostream&   o);

    void     PrintConfusionMatrixHTML (const char *title, ostream&  file);

    void     PrintConfusionMatrixLatexTable (ostream&  outFile);

    void     PrintConfusionMatrixNarrow (ostream&  outFile);

    void     PrintConfusionMatrixHTML (ostream&  outFile);

    void     PrintConfusionMatrixTabDelimited (ostream&  outFile);

    void     PrintTrueFalsePositivesTabDelimited (ostream&  outFile);

    void     PrintErrorBySize        (ostream&  outFile);

    void     PrintErrorByProb        (ostream&  outFile);

    void     PrintErrorBySizeByRows  (ostream&  outFile); 

    void     PrintErrorByProbByRows  (ostream&  outFile);

    void     PrintErrorBySizeReduced (ostream&  outFile);



    //***********************************************************
    //*                  One Line Summaries                     *
    //***********************************************************
    void   PrintProbDistributionTitle      (ostream&  outFile);
    void   PrintProbDistributionTotalCount (ostream&  outFile);
    void   PrintProbDistributionTotalError (ostream&  outFile);

    RunLog&  Log ()  {return log;}

    double    TotalCount ()   {return totalCount;}



    static
    ConfusionMatrix2Ptr  BuildFromIstreamXML (istream&  f,
                                              RunLog&   log
                                             );

    void  WriteXML (ostream&  f)  const;

    // This method writes a simple confusion matrix table; one row for each class.  The BIAS
    // adjustment routines can then utilize this data to adjust final counts.
    void  WriteSimpleConfusionMatrix (ostream&  f)  const;



  private:
    void  InitializeMemory ();

    void  PrintLatexTableColumnHeaders (ostream&  outFile);

    void  PrintSingleLine (ostream&  _outFile,
                           KKStr     _name,
                           double    _lineTotal,
                           double    _splits[]
                          );

    void  PrintSingleLineTabDelimited (ostream&      _outFile,
                                       const KKStr&  _name,
                                       double        _lineTotal,
                                       double        _splits[]
                                      );

    void  PrintSingleLineHTML (ostream&     _outFile,
                               const KKStr& _name,
                               double       _lineTotal,
                               kkint32      _knownClassNum,
                               double       _splits[]
                              );

    void  PrintSingleLineLatexTable (ostream&      _outFile,
                                     kkint32       _knownClassNum, 
                                     const KKStr&  _name,
                                     double        _lineTotal,
                                     double        _splits[]
                                    );


    void  PrintSingleLineShort (ostream&     _outFile,
                                const KKStr& _name,
                                double       _lineTotal,
                                double       _splits[]
                               );


    void  PrintPercentLine (ostream&  _outFile,
                            KKStr     _name,
                            double    _totalCount,
                            double    _splits[]
                           );


    void  PrintPercentLineLatexTable (ostream&     _outFile,
                                      kkint32      _rowNum,
                                      const KKStr& _name,
                                      double       _lineTotal,
                                      double       _splits[]
                                     );


    void  PrintPercentLineTabDelimited (ostream&     _outFile,
                                        const KKStr& _name,
                                        double        _lineTotal,
                                        double        _splits[]
                                       );


    void  PrintAvgPredProbLineHTML (ostream&      o,
                                    const KKStr&  _name,
                                    double        _totalAvgPredProbThisLine,
                                    double        _totalCountThisLine,
                                    kkint32       _knownClassNum,
                                    double        _avgPredProbs[],
                                    double        _numPredByClass[]
                                   );

    
    void  PrintPercentLineHTML (ostream&      _outFile,
                                const KKStr&  _name,
                                double        _lineTotal,
                                kkint32       _knownClassNum,
                                double        _splits[]
                               );


    void  PrintPercentLineShort (ostream&       _outFile,
                                 const KKStr&   _name,
                                 double         _lineTotal,
                                 double         _splits[]
                                );


    void  PrintErrorBySizeRowReduced (ostream&  outFile,
                                      kkint32   classNum
                                     );


    void  Read (istream& f);


    kkint32          bucketSize;
    kkint32          classCount;
    double**         correctByKnownClassByProb;
    double**         correctByKnownClassBySize;
    double           correctCount;
    double**         countByKnownClassByProb;
    double**         countByKnownClassBySize;
    double*          countsByKnownClass;

    MLClassList   classes;   /**< We will make our own unique copy of the MLClassList.  This way we know the
                                 * ordering which represents the numbering can not change behind our back.
                                 */
    RunLog&          log;
    double           numInvalidClassesPredicted;
    kkint32          numOfBuckets;
    kkint32          numOfProbBuckets;
    double**         predictedCountsCM;
    kkint32          probBucketSize;
    double           totalCount;
    double           totalPredProb;
    double*          totalPredProbsByKnownClass;   /**< Total Predicted Probabilities by Known Class. */
    double*          totalSizesByKnownClass;
    double**         totPredProbCM;

  };


  typedef  ConfusionMatrix2::ConfusionMatrix2Ptr  ConfusionMatrix2Ptr;


  class  ConfussionMatrix2List: public KKQueue<ConfusionMatrix2>
  {
  public:
    ConfussionMatrix2List (bool _owner = true);

    ~ConfussionMatrix2List ();

    ConfusionMatrix2Ptr  DeriveAverageConfusionMatrix () const;


  };  /* ConfussionMatrix2List */


  typedef  ConfussionMatrix2List*  ConfussionMatrix2ListPtr;


} /* namespace  KKMachineLearning */

#endif
