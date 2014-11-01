KSquareLibraries
================

[Project Page](http://kurtkramer.github.io/KSquareLibraries)

[Doxygen generated Documentation](http://kurtkramer.github.io/KSquareLibraries/Doxygen/html)


c++ Tool Kit routines developed over the past several years.

 *@section  Introduction   Introduction
 * The KKBaseLibrery is my tool box of handy objects that I have been building since 1994.  
 * It contains classes for string management(KKStr), Floating point Matrix operations, 
 * Image Processing Routines, common operating system routines, Statistics, Histogramming,
 * etc.
 * \n\n
 *
 *@section  PlatformIndependence    Platform Independence
 * All classes are meant to be Platform independent.  That is from the outside there should
 * be no need to worry about which O/S platform you are building for.  All O/S specific
 * code will be internal to KKBaseLibrery (KKB) classes.  For the most part all functions that 
 * require O/S specific knowledge are implemented in OSServices.h.  There are a couple of
 * exceptions such as GoalKeeper and ImageIO.h.
 * 
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
 * 
 *
 *@section KKQueue   KKQueue
 * I have a container template called KKQueue that is derived from the std::vector<> template.
 * It is specifically meant to work with pointers to its contents and understands the
 * concept of ownership.  That is a instance of a KKQueue template either owns its contents
 * or it does not.  If it owns them it will call the individual destructors for each item that it
 * contains when its destructor is called. Many classes that I have written use this template so
 * it would be worth reading its documentation.
 * 
 *
 *@section  ImageProcessing   Image Processing
 * There are several classes that aid in Image Processing.  The primary class is Raster which
 * can handle both Grayscale and Color images.  It supports various morphological operations
 * as well as Connected Component Analysis, Fourier transform, etc.  Other classes that work
 * with Raster are Blob, MorphOp, ContourFollower, MorphOpStretcher, Histogram, ImageIO.h, 
 * PixelValue, Point, Sobel, and several others.
