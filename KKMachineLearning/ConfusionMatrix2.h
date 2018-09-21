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
  /// <see also cref="CrossValidation"
  /// </summary>
  class  ConfusionMatrix2
  {
  public:
    typedef  ConfusionMatrix2*  ConfusionMatrix2Ptr;
    typedef  ConfusionMatrix2 const *  ConfusionMatrix2ConstPtr;

    ConfusionMatrix2 (const MLClassList&  _classes);
    
    ConfusionMatrix2 (const ConfusionMatrix2&  cm);


    /**  
     * Will construct an instance of 'ConfusionMatrix2'  from the contents of the provided  'istream' object. 
     *@param[in]  _classes  Will make local copy of this instance; this way we know the ordering which represents the numbering can not change behind our back.
     *@param[in]  f   File to write report to.
     *@param[in]  _bucketSize  Will keep statistics by size of particles.
     *@param[in]  _numOfBuckets Number of Size buckets that will be maintained.
     *@param[in]  _numOfProbBuckets  Maximum number of probability buckets to keep track of, 
     *@param[in]  _probBucketSize  Size of each probability bucket.
     *@param[in]  _log  Logger where messages are written to.
     */
    ConfusionMatrix2 (const MLClassList&  _classes,  // Will make its own copy of '_classes'
                      istream&            f,
                      kkuint32            _bucketSize,
                      kkuint32            _numOfBuckets,
                      kkuint32            _numOfProbBuckets,
                      kkuint32            _probBucketSize,
                      RunLog&             _log
                     );
                                           
    virtual
    ~ConfusionMatrix2 ();

    double   Accuracy ()  const;

    VectorFloat AccuracyByClass  ()  const;

    float    AccuracyNorm ()   {return AccuracyClassWeightedEqually ();}

    float    AccuracyClassWeightedEqually ()  const;

    double   Accuracy (MLClassPtr  mlClass)  const;

    KKStr    AccuracyStr ();

    void     AddIn (const ConfusionMatrix2&  cm,
                    RunLog&                  log
                   );

    double   AvgPredProb ()  const;

    kkuint32  ClassCount ()  const {return  classCount;}

    double   Count (MLClassPtr  mlClass);

    double   CountsByKnownClass (kkuint32 knownClassIdx)  const;

    const    VectorDouble&   CountsByKnownClass ()  const;

    void     FactorCounts (double  factor);    /**< Will multiply all counts by 'factor'  You would use this in conjunction with 'AddIn'.  */

    void     ComputeFundamentalStats (MLClassPtr mlClass,
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
                        double      _probability,
                        RunLog&     _log
                       );

    VectorDouble   PredictedCounts ()  const;

    double   PredictedCountsCM (kkuint32 knownClassIdx, kkuint32  predClassIdx)  const;

    void     PrintAccuracyByProbByClassHTML (ostream&  o)  const;

    void     PrintConfusionMatrix (ostream&  _outFile)  const;

    void     PrintConfusionMatrixAvgPredProbHTML (ostream&   o)  const;

    void     PrintConfusionMatrixHTML (const char *title, ostream&  file);

    void     PrintConfusionMatrixLatexTable (ostream&  outFile);

    void     PrintConfusionMatrixNarrow (ostream&  outFile);

    void     PrintConfusionMatrixHTML (ostream&  outFile)  const;

    void     PrintConfusionMatrixTabDelimited (ostream&  outFile)  const;

    void     PrintTrueFalsePositivesTabDelimited (ostream&  outFile)  const;

    void     PrintErrorBySize        (ostream&  outFile)  const;

    void     PrintErrorByProb        (ostream&  outFile)  const;

    void     PrintErrorBySizeByRows  (ostream&  outFile)  const; 

    void     PrintErrorByProbByRows  (ostream&  outFile)  const;

    void     PrintErrorBySizeReduced (ostream&  outFile)  const;

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
    kkuint32 AddClassToConfusionMatrix (MLClassPtr  newClass,
                                        RunLog&     log
                                       );

    void  InitializeMemory ();

    void  InitializeVector (vector<double>&  v,
                            kkuint32         x
                           );

    void  InitializeVectorDoublePtr (vector<double*>& v,
                                     kkuint32         numClasses,
                                     kkuint32         numBuckets
                                    );

    void  CopyVector (const vector<double>&  src,
                      vector<double>&        dest
                     );

    void  CopyVectorDoublePtr (const vector<double*>&  src,
                               vector<double*>&        dest,
                               kkuint32                numBuckets
                              );

    void  DeleteVectorDoublePtr (vector<double*>&  v);

    void  IncreaseVectorDoublePtr (vector<double*>&  v,
                                   kkuint32          numBucketsOld,
                                   kkuint32          numBucketsNew
                                  );

    void  MakeSureWeHaveTheseClasses (const MLClassList&  classList,
                                      RunLog&                  log
                                     );


    void  PrintLatexTableColumnHeaders (ostream&  outFile)  const;


    void  PrintSingleLine (ostream&  _outFile,
                           KKStr     _name,
                           double    _lineTotal,
                           double    _splits[]
                          )  const;


    void  PrintSingleLineTabDelimited (ostream&      _outFile,
                                       const KKStr&  _name,
                                       double        _lineTotal,
                                       double        _splits[]
                                      )  const;


    void  PrintSingleLineHTML (ostream&     _outFile,
                               const KKStr& _name,
                               double       _lineTotal,
                               kkuint32     _knownClassNum,
                               double       _splits[]
                              )  const;


    void  PrintSingleLineLatexTable (ostream&      _outFile,
                                     kkuint32      _knownClassNum, 
                                     const KKStr&  _name,
                                     double        _lineTotal,
                                     double        _splits[]
                                    )  const;


    void  PrintSingleLineShort (ostream&     _outFile,
                                const KKStr& _name,
                                double       _lineTotal,
                                double       _splits[]
                               )  const;


    void  PrintPercentLine (ostream&  _outFile,
                            KKStr     _name,
                            double    _totalCount,
                            double    _splits[]
                           )  const;


    void  PrintPercentLineLatexTable (ostream&     _outFile,
                                      kkuint32     _rowNum,
                                      const KKStr& _name,
                                      double       _lineTotal,
                                      double       _splits[]
                                     )  const;


    void  PrintPercentLineTabDelimited (ostream&     _outFile,
                                        const KKStr& _name,
                                        double       _lineTotal,
                                        double       _splits[]
                                       )  const;


    void  PrintAvgPredProbLineHTML (ostream&      o,
                                    const KKStr&  _name,
                                    double        _totalAvgPredProbThisLine,
                                    double        _totalCountThisLine,
                                    kkuint32      _knownClassNum,
                                    double        _avgPredProbs[],
                                    double        _numPredByClass[]
                                   )  const;

    
    void  PrintPercentLineHTML (ostream&      _outFile,
                                const KKStr&  _name,
                                double        _lineTotal,
                                kkuint32      _knownClassNum,
                                double        _splits[]
                               )  const;


    void  PrintPercentLineShort (ostream&       _outFile,
                                 const KKStr&   _name,
                                 double         _lineTotal,
                                 double         _splits[]
                                )  const;


    void  PrintErrorBySizeRowReduced (ostream&  outFile,
                                      kkuint32  classNum
                                     )  const;

    void  Read (istream&  f,
                RunLog&   log
               );

    kkuint32         bucketSize;
    kkuint32         classCount;
    MLClassList      classes;  /**< We will make our own unique copy of the MLClassList.
                                * This way we know the ordering which represents the numbering
                                * can not change behind our back.
                                */

    vector<double*>  correctByKnownClassByProb;
    vector<double*>  correctByKnownClassBySize;
    double           correctCount;
    vector<double*>  countByKnownClassByProb;
    vector<double*>  countByKnownClassBySize;
    vector<double>   countsByKnownClass;
    double           numInvalidClassesPredicted;
    kkuint32         numOfBuckets;
    kkuint32         numOfProbBuckets;
    vector<double*>  predictedCountsCM;
    kkuint32         probBucketSize;
    double           totalCount;
    double           totalPredProb;
    vector<double>   totalPredProbsByKnownClass;     /**< Total Predicted Probabilities by Known Class. */
    vector<double>   totalSizesByKnownClass;
    vector<double*>  totPredProbCM;
  };

  typedef  ConfusionMatrix2::ConfusionMatrix2Ptr  ConfusionMatrix2Ptr;

  typedef  ConfusionMatrix2::ConfusionMatrix2ConstPtr  ConfusionMatrix2ConstPtr;


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
