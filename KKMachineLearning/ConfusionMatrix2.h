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
//*  Kurt    Oct-19-2002  Increment Will now take MLClasses instead of      *
//*                       numbers.  We will also make a unique copy of      *
//*                       mlClassList. This way we will not have to worry   *
//*                       about the numbering in the classList behind our   *
//*                       back.                                             * 
//***************************************************************************
//* 

#include "RunLog.h"
#include "KKStr.h"

#include "MLClass.h"

namespace  KKMLL
{

  /// <summary>
  /// A confusion matrix object that is used to record the results from a CrossValidation.
  /// <seealso cref="CrossValidation"
  /// </summary>
  class  ConfusionMatrix2
  {
  public:
    typedef  ConfusionMatrix2*  ConfusionMatrix2Ptr;

    ConfusionMatrix2 (const MLClassList&  _classes,  /**< Will make its own copy of '_classes'. */
                      RunLog&             _log
                     );
    
    ConfusionMatrix2 (const ConfusionMatrix2&  cm);


    /**  
     * Will construct an instance of 'ConfusionMatrix2'  from the contents of the provided  'istream' object. 
     *@param[in]  _classes  Will make local copy of this instance; this way we know the ordering which represents the numbering can not change behind our back.
     *@param[in]  f   File to write report to.
     *@param[in]  _bucketSize  Will keep statistics by size of particles.
     *@param[in]  _numOfBuckets Number of Size buckes that will be maintained.
     *@param[in]  _numOfProbBuckets  Maximum number of probability buckets to keep track of, 
     *@param[in]  _probBucketSize  Size of each probability bucket.
     *@param[in]  _log  Logger where messages are writen to.
     */
    ConfusionMatrix2 (const MLClassList&  _classes,  // Will make its own copy of '_classes'
                      istream&            f,
                      kkint32             _bucketSize,
                      kkint32             _numOfBuckets,
                      kkint32             _numOfProbBuckets,
                      kkint32             _probBucketSize,
                      RunLog&             _log
                     );
                                           
    virtual
    ~ConfusionMatrix2 ();

    double   Accuracy ();

    VectorFloat AccuracyByClass  ()  const;

    float    AccuracyNorm ()   {return AccuracyClassWeightedEqually ();}

    float    AccuracyClassWeightedEqually ();

    double   Accuracy (MLClassPtr  mlClass);

    KKStr    AccuracyStr ();

    void     AddIn (const ConfusionMatrix2&  cm,
                    RunLog&                  log
                   );

    double   AvgPredProb ()  const;

    kkint32  ClassCount ()  const {return  classCount;}

    double   Count (MLClassPtr  mlClass);

    double   CountsByKnownClass (kkint32 knownClassIdx)  const;

    const    VectorDouble&   CountsByKnownClass ()  const;

    void     FactorCounts (double  factor);    /**< Will multiply all counts by 'factor'  You would use this in conjunction with 'AddIn'.  */

    void     ComputeFundamentalStats (MLClassPtr ic,
                                      double&    truePositives,
                                      double&    trueNegatives,
                                      double&    falsePositives,
                                      double&    falseNegatives
                                     )
                                   const;

    float    FMeasure (MLClassPtr  positiveClass,
                       RunLog&     log
                      )  const;

    const
    MLClassList&  MLClasses () const  {return  classes;}

    void     Increment (MLClassPtr  _knownClass,
                        MLClassPtr  _predClass,
                        kkint32     _size,
                        double           _probability,
                        RunLog&          _log
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

    double    TotalCount ()   {return totalCount;}

    static
    ConfusionMatrix2Ptr  BuildFromIstreamXML (istream&  f,
                                              RunLog&   log
                                             );

    void  WriteXML (ostream&  f)  const;


    /**
     * Meant to work with 'ClassificationStatus.cs' of PicesCommander. This will write a simple
     * confusion matrix table; one row for each class. 'ClassificationStatus.cs' will then use this
     * data to adjust for bias in the learner.
     */
    void  WriteSimpleConfusionMatrix (ostream&  f)  const;

  private:
    kkint32 AddClassToConfusionMatrix (MLClassPtr  newClass,
                                       RunLog&     log
                                      );

    void  InitializeMemory ();

    void  InitializeVector (vector<double>&  v,
                            kkint32          x
                           );

    void  InitializeVectorDoublePtr (vector<double*>& v,
                                     kkint32          numClasses,
                                     kkint32          numBuckets
                                    );

    void  CopyVector (const vector<double>&  src,
                      vector<double>&        dest
                     );

    void  CopyVectorDoublePtr (const vector<double*>&  src,
                               vector<double*>&        dest,
                               kkint32                 numBuckets
                              );

    void  DeleteVectorDoublePtr (vector<double*>&  v);

    void  IncreaseVectorDoublePtr (vector<double*>&  v,
                                   int               numBucketsOld,
                                   int               numBucketsNew
                                  );

    void  MakeSureWeHaveTheseClasses (const MLClassList&  classList,
                                      RunLog&                  log
                                     );


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

    void  Read (istream&  f,
                RunLog&   log
               );

    kkint32          bucketSize;
    kkint32          classCount;
    vector<double*>  correctByKnownClassByProb;
    vector<double*>  correctByKnownClassBySize;
    double           correctCount;
    vector<double*>  countByKnownClassByProb;
    vector<double*>  countByKnownClassBySize;
    vector<double>   countsByKnownClass;

    MLClassList  classes;  /**< We will make our own unique copy of the MLClassList.
                                 * This way we know the ordering which represents the numbering
                                 * can not change behind our back.
                                 */
    kkint32          numOfBuckets;
    kkint32          numOfProbBuckets;
    vector<double*>  predictedCountsCM;
    vector<double*>  totPredProbCM;
    kkint32          probBucketSize;
    double           totalCount;
    double           totalPredProb;
    vector<double>   totalPredProbsByKnownClass;     /**< Total Predicted Probabilities by Known Class. */
    vector<double>   totalSizesByKnownClass;

    double           numInvalidClassesPredicted;
  };

  typedef  ConfusionMatrix2::ConfusionMatrix2Ptr  ConfusionMatrix2Ptr;

#define  _ConfussionMatrix2_Defined_


  class  ConfussionMatrix2List: public KKQueue<ConfusionMatrix2>
  {
  public:
    ConfussionMatrix2List (bool _owner = true);

    ~ConfussionMatrix2List ();

    ConfusionMatrix2Ptr  DeriveAverageConfusionMatrix (RunLog&  log) const;


  };  /* ConfussionMatrix2List */

  typedef  ConfussionMatrix2List*  ConfussionMatrix2ListPtr;

#define  _ConfussionMatrix2List_Defined_

} /* namespace  KKMLL */

#endif
