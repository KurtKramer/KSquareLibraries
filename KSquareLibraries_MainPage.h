
/**
 *@mainpage  KSquareLibraries
 *
 *@section  KSquare-Introduction  "Introduction to KSquare Libraries"
 * The libraries with their included functions/classes are the accumulation of work 
 *that I have built up over the last several years, mostly during my graduate program.  
 *This has become a useful tool-kit for several projects that I have worked on. The 
 *code is meant to be O/S neutral such that it can be compiled and used in either Windows 
 *or Linux.
 *
 *@section KKBase  
 *Functionality that is common to other libraries and applications. Some examples 
 *includes @link KKB::KKStr string management @endlink, @link KKB::Matrix Matrix @endlik operations, 
 *@link KKB::Raster Image Processing @endlink, @link KKB::Tokenizer Token Parsing @endlink, 
 *@link StatisticalFunctions.h Statistics @endlink, Histogramming, and common operating system routines.  
 * Most O/S specific code is implemented in the module @link OSservices.h "osServices.cpp" @endlink.
 *
 *@section KKLineScanner  "Line Scanner"
 *Routines to support Line-Scan camera imagery files. Classes and code that support 
 * the processing of data from line-scan cameras. Support for Dynamic-Flat-Field 
 *correction. Storage and retrieval of imagery data in 2,3,4, and 8 bit-depth 
 *gray-scale with support for embedding instrumentation data as well as text. 
 *
 *@section  KKMachineLearning "Machine Learning"
 *Classes and Code that support Machine-Learning implementations. 
 *Examples:
 * - @link KKML::FeatureFileIO FeatureFileIO @endlink several common feature data file formats are support.  (sparse, arff, c45, etc...)
 * - @link KKML::TrainingConfiguration2 Training-Model-Configuration @endlink  class and routines that maintain classifier parameters; such as classifier type.
 * - Machine Learning Classes (@link KKML::MLClass MLClass @endlink )  and containers for tracking @link KKML::MLClassList lists of classes @endlink.
 * - Hierarchical Class naming is supported.
 * - Containers for Feature Data @link KKML::FeatureVector "FeatureVector" @endlink, @link KKML::FeatureVectorList "FeatureVectorList" @endlink, stratifying by class.
 * - Feature data normalization routines.
 * - Confusion-Matrix
 * - CrossValidation - Example 10 fold CV; also (N x X) cross Validation; typically used by grading a classifier.
 * - Implementations of Classifiers'.
 *    -# Pair-Wise Feature selected SVM (Unique set of features for each pair of classes).
 *    -# Common Features SVM (One set of features for all class pairs).
 *    -# USF Cascading correlation neural networks (USF-Cas-Cor).
 *    -# Dual Classifier , simple two level classifier that utilizes two classifiers to make a multi level prediction.
 *
 *
 *@section  KKJobManager
 *Framework that assists in implementing background processes running on multiple cpu's.
 * 
 *@section  OutsideLibrariesUsed  "Outside Libraries"
 *External libraries required for full functionality. That is libraries and code from other 
 *sources.  Outside Libraries FFTW and ZLIB To use all the classes in this Library you will 
 *need the libraries fftw and lib123.  "fftw" stands for "Fastest Fourier Transform in The West".  
 *It can be downloaded from http://www.fftw.org/.  "zlib123" is a library that is used to compress 
 *and un-compress data. can be found at http://www.zlib.net/ and is used by the Compressor class.
 *Their are two Macros that determine weather the two libraries listed above are utilized or more 
 *simple implementations included in KKBase. 'FFTW_AVAILABLE' - Indicates that you want to utilize 
 *the FFTW library.  otherwise a simple Fourier transform will be utilized. 'ZLIB_AVAILABLE' - 
 *Indicates that 'zlib' library is available.  If not defined the class Compressor will not do 
 *anything.
 */
