namespace  KKB
{
/**
 *@namespace  KKB  
 *@brief KKB The namespace for KKBase Library supporting general functionality needed by almost any application. 
 *@section  Introduction   Introduction
 * The KKBase Librery is my tool box of handy objects that I have been building since 1994.  
 * It contains classes for string management(KKStr), Floating point Matrix operations, 
 * Image Processing Routines, common operating system routines, Statistics, Histogramming,
 * etc. These classes are meant to be Operating System(OS) neutral in that from the outside 
 * of the library they perform identically no matter what OS you are on.
 * \n
 *
 *@section  PlatformIndependence    Platform Independence
 * All classes are meant to be Platform independent.  That is from the outside there should
 * be no need to worry about which O/S platform you are building for.  All O/S specific
 * code will be internal to KKBaseLibrery (KKB) classes.  For the most part all functions that 
 * require O/S specific knowledge are implemented in OSServices.h.  There are a couple of
 * exceptions such as GoalKeeper and ImageIO.h.
 * \n
 *
 *@section  OutsideLibraries  Outside Libraries FFTW and ZLIB.
 * To use all the classes in this Library you will need the libraries fftw and zlib123.  
 * "fftw" stands for "Fastest Fourier Transform in The West".  It can be downloaded from 
 * http://www.fftw.org/.  "zlib123" is a library that is used to compress and uncompress 
 * data. can be found at http://www.zlib.net/.  It is used by the Compressor class.
 *
 * Macros that should be defined.
 * 'FFTW_AVAILABLE' - Indicates that you want to utilize the FFTW library.  otherwise a simple
 *                    Fourier transform will be utilized.
 * 'ZLIB_AVAILABLE' - Indicates that 'zlib' library is available.  If not defined the class
 *                     Compressor will not do anything.
 * \n
 *
 *@section KKQueue   KKQueue
 * I have a container template called KKQueue that is derived from the std::vector<> template.
 * It is specifically meant to work with pointers to its contents and understands the
 * concept of ownership.  That is a instance of a KKQueue template either owns its contents
 * or it does not.  If it owns them it will call the individual destructors for each item that it
 * contains when its destructor is called. Many classes that I have written use this template so
 * it would be worth reading its documentation.
 * \n
 *
 *@section  ImageProcessing   Image Processing
 * There are several classes that aid in Image Processing.  The primary class is Raster which
 * can handle both Grayscale and Color images.  It supports various morphological operations
 * as well as Connected Component Analysis, Fourier transform, etc.  Other classes that work
 * with Raster are Blob, BMPheader, BMPheader.h, ContourFollower, Histogram, ImageIO.h, 
 * PixelValue, Point, and Sobel.
 *
 *@section  Application  Base class for applications.
 * Contains basic support for applications such as @link KKB::CmdLineExpander command-line @endlink processing and logging. 
 * There is logic to process specified text files for additional parameters. A RunLog instance will be created and
 * inherited by all derived classes; the command line will process and -Log parameter specified.  Derived classes can intercept 
 * command line parameters simple y implementing the virtual method 
 * @link Application::ProcessCmdLineParameter ProcessCmdLineParameter @endlink
 */
}
