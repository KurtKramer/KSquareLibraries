EXEC = BaseLibrary
#CFLAGS = -O3 
CFLAGS = -g  
LIBS = -lfftw -lrt
FFTWAPI = ../OutsidePackages/fftw-3.3.2/api 
VPATH = ../KKBase
LIBDIR = ../KKBase

CC = g++ -W -Wall -Wextra -pedantic -std=c++0x 

FIRSTINCLUDES      = FirstIncludes.h



RANDOMNUMGENERATOR = RandomNumGenerator.h  KKBaseTypes.h
KKU_FFTW           = kku_fftw.h            KKBaseTypes.h
EIGENVECTOR        = EigenVector.h         KKBaseTypes.h
KKQUEUE            = KKQueue.h             KKBaseTypes.h

STR                = KKStr.h               KKBaseTypes.h         $(KKQUEUE)
KKSTRMATRIX        = KKStrMatrix.h         KKBaseTypes.h
RUNLOG             = RunLog.h              $(STR)

XMLSTREAM          = XmlStream.h           $(STR)                $(TOKENIZER)
ATOM               = Atom.h
MSGQUEUE           = MsgQueue.h            $(GOALKEEPER)
KKEXCEPTION        = KKException.h         KKBaseTypes.h         $(KKQUEUE)         $(STR)
KKTHREAD           = KKThread.h            $(MSGQUEUE)
KKTHREADMANAGER    = KKThreadManager.h     $(MSGQUEUE)           $(RUNLOG)

BITSTRING          = BitString.h           KKBaseTypes.h         $(STR)
DATETIME           = DateTime.h            $(STR)
KKSTRPARSER        = KKStrParser.h         $(DATETIME)           $(STR)

IMAGEIO            = ImageIO.h             $(STR)
OSSERVICES         = OSservices.h          $(STR)                $(DATETIME)        $(IMAGEIO)
HTMLREPORT         = HTMLReport.h          $(DATETIME)           $(STR)
GLOBALGOALKEEPER   = GlobalGoalKeeper.h    $(STR)
GOALKEEPER         = GoalKeeper.h          $(STR)
MATRIX             = Matrix.h              KKBaseTypes.h

COMPRESSPR         = Compressor.h          KKBaseTypes.h

STATISTICALFUNCTIONS = StatisticalFunctions.h  KKBaseTypes.h 

APPLICATION        = Application.h         $(STR)                $(RUNLOG)

IMAGESDIRTREE      = ImageDirTree.h        $(STR)

POINT              = Point.h               KKBaseTypes.h          $(KKQUEUE)         $(STR)

BLOB               = Blob.h                $(KKQUEUE)             $(POINT)

CONTOURFOLLOWER    = ContourFollower.h     KKBaseTypes.h          $(RUNLOG)
CONVEXHULL         = ConvexHull.h          KKBaseTypes.h

PIXELVALUE         = PixelValue.h          KKBaseTypes.h          $(STR)

RASTER             = Raster.h              KKBaseTypes.h          $(KKQUEUE)         $(STR)             $(PIXELVALUE)         $(POINT)
RASTERBUFFER       = RasterBuffer.h        $(GOALKEEPER)          $(STR)



MORPHOP            = MorphOp.h             $(KKQUEUE)             $(STR)             $(RUNLOG)
MORPHOPBINARIZE    = MorphOpBinarize.h     KKBaseTypes.h          $(MORPHOP)
MORPHOPEREROSION   = MorphOpErosion.h      KKBaseTypes.h          $(MORPHOP)
MORPHOPMASKEXCLUDE = MorphOpMaskExclude.h  KKBaseTypes.h          $(MORPHOP)
MORPHOPSTRETCHER   = MorphOpStretcher.h    KKBaseTypes.h          $(MORPHOP)



BMPIMAGE           = BMPImage.h            BMPheader.h            $(STR)
HISTOGRAM          = Histogram.h           KKBaseTypes.h          $(STR)
SOBEL              = Sobel.h               $(STR)
SEGMENTOROTSU      = SegmentorOTSU.h       KKBaseTypes.h          $(MATRIX)          $(STR)             $(RASTER)         $(RUNLOG)
SIMPLECOMPRESSOR   = SimpleCompressor.h    

CHART              = Chart.h               KKBaseTypes.h          $(STR)             $(RASTER)          $(POINT)

CONFIGURATION      = Configuration.h       $(STR)                 $(RUNLOG)

TOKENBUFFER        = TokenBuffer.h         $(STR)

TOKENIZER          = Tokenizer.h           $(TOKENBUFFER)

XMLSTREAM          = XmlStream.h           $(STR)                 $(TOKENBUFFER)

CMDLINEEXPANDER    = CmdLineExpander.h     $(STR)                 $(RUNLOG)


OBJECTS  = Application.o               Atom.o                      BitString.o                 Blob.o                 \
           BMPImage.o                  Chart.o                     CmdLineExpander.o           Compressor.o           \
           Configuration.o             ContourFollower.o           ConvexHull.o                DateTime.o             \
           EigenVector.o               GlobalGoalKeeper.o          GoalKeeper.o                Histogram.o            \
           HTMLReport.o                ImageIO.o                   ImageDirTree.o              KKBaseTypes.o          \
           KKException.o               KKStrMatrix.o               KKStr.o                     KKStrMatrix.o          \
           KKStrParser.o               KKThread.o                  KKThreadManager.o           kku_fftw.o             \
           Matrix.o                    MorphOp.o                   MorphOpBinarize.o           MorphOpErosion.o       \
           MorphOpMaskExclude.o        MorphOpStretcher.o          MsgQueue.o                  OSservices.o           \
           PixelValue.o                Point.o                     RandomNumGenerator.o        Raster.o               \
           RasterBuffer.o              RunLog.o                    SegmentorOTSU.o             SimpleCompressor.o     \
           Sobel.o                     StatisticalFunctions.o      TokenBuffer.o               Tokenizer.o            \
           XmlStream.o


all:	$(EXEC)

clean:
	rm *.o;  rm *.a	


$(EXEC): $(OBJECTS) 
	rm *.a; ar cq libBaseLibrary.a  $(OBJECTS)


Application.o :        Application.cpp          $(APPLICATION)           $(CMDLINEEXPANDER)       $(DATETIME)              $(OSSERVICES)  \
                       $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Application.cpp


Atom.o :               Atom.cpp                 $(ATOM)                  KKBaseTypes.h            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Atom.cpp


KKBaseTypes.o :        KKBaseTypes.cpp          KKBaseTypes.h            $(FIRSTINCLUDES) 
	$(CC) $(CFLAGS) -c KKBaseTypes.cpp


BitString.o :          BitString.cpp            $(BITSTRING)             KKBaseTypes.h            $(FIRSTINCLUDES)       
	$(CC) $(CFLAGS) -c BitString.cpp


Blob.o :               Blob.cpp                 $(BLOB)                  $(BITSTRING)             $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Blob.cpp


BMPImage.o :           BMPImage.cpp             $(BMPIMAGE)              $(OSSERVICES)            $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c BMPImage.cpp


Chart.o :              Chart.cpp                $(CHART)                 $(IMAGEIO)               $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Chart.cpp


CmdLineExpander.o :    CmdLineExpander.cpp      $(CMDLINEEXPANDER)       $(OSSERVICES)            $(STR)                   $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c CmdLineExpander.cpp


Compressor.o :         Compressor.cpp           $(COMPRESSOR)            $(OSSERVICES)
	$(CC) $(CFLAGS) -c Compressor.cpp
 

Configuration.o :      Configuration.cpp        $(CONFIGURATION)         $(OSSERVICES)            $(RUNLOG)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Configuration.cpp


ContourFollower.o :    ContourFollower.cpp      $(CONTOURFOLLOWER)       $(KKU_FFTW)              $(RASTER)                $(STR)                   $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c ContourFollower.cpp    -I$(FFTWAPI)   -I$(FFTWAPI)


ConvexHull.o :         ConvexHull.cpp           $(CONVEXHULL)            $(POINT)                 $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c $(LIBDIR)/$*.cpp


DateTime.o :           DateTime.cpp             $(DATETIME)              $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c $(LIBDIR)/$*.cpp


EigenVector.o :        EigenVector.cpp          $(EIGENVECTOR)           KKBaseTypes.h            $(FIRSTINCLUDES)
	$(CC)  $(CFLAGS) -c $(LIBDIR)/$*.cpp


GlobalGoalKeeper.o :   GlobalGoalKeeper.cpp     $(GLOBALGOALKEEPER)      $(GOALKEEPER)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c GlobalGoalKeeper.cpp


GoalKeeper.o :         GoalKeeper.cpp           $(GOALKEEPER)            $(OSSERVICES)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c GoalKeeper.cpp


Histogram.o :          Histogram.cpp            $(HISTOGRAM)             $(RASTER)                $(IMAGEIO)               $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Histogram.cpp


HTMLReport.o :         HTMLReport.cpp           $(HTMLREPORT)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c  HTMLReport.cpp


ImageIO.o :            ImageIO.cpp              $(IMAGEIO)               $(OSSERVICES)            $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c ImageIO.cpp


ImageDirTree.o :       ImageDirTree.cpp         $(IMAGEDIRTREE)          $(IMAGEIO)               $(OSSERVICES)            $(STR)                   $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c ImageDirTree.cpp


KKException.o :        KKException.cpp          $(KKEXCEPTION)           $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c KKException.cpp


KKStr.o :              KKStr.cpp                $(STR)                   $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c $(LIBDIR)/$*.cpp


KKStrMatrix.o :        KKStrMatrix.cpp          $(KKSTRMATRIX)           $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c KKStrMatrix.cpp


KKStrParser.o :        KKStrParser.cpp          $(KKSTRPARSER)           KKBaseTypes.h            $(STR)                   $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c KKStrParser.cpp


kku_fftw.o :           kku_fftw.cpp             $(KKU_FFTW)              $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c kku_fftw.cpp


KKThread.o :           KKThread.cpp             $(KKTHREAD)              $(KKEXCEPTION)           $(MSGQUEUE)              $(OSSERVICES)             $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c KKThread.cpp


KKThreadManager.o :    KKThreadManager.cpp      $(KKTHREADMANAGER)       $(KKTHREAD)              $(MSGQUEUE)              $(OSSERVICES)             $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c KKThreadManager.cpp


Matrix.o :             Matrix.cpp               $(MATRIX)                $(OSSERVICES)            $(STR)                   $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Matrix.cpp


MorphOp.o :            MorphOp.cpp              $(MORPHOP)               $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c MorphOp.cpp


MorphOpBinarize.o :    MorphOpBinarize.cpp      $(MORPHOPBINARIZE)       $(MORPHOP)               $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c MorphOpBinarize.cpp


MorphOpErosion.o :     MorphOpErosion.cpp       $(MORPHOPEREROSION)      $(MORPHOP)               $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c MorphOpErosion.cpp


MorphOpMaskExclude.o : MorphOpMaskExclude.cpp   $(MORPHOPMASKEXCLUDE)    $(MORPHOP)               $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c MorphOpMaskExclude.cpp


MorphOpStretcher.o : MorphOpStretcher.cpp       $(_MORPHOPSTRETCHER_)    $(MORPHOP)               $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c MorphOpStretcher.cpp


MsgQueue.o :           MsgQueue.cpp             $(MsgQueue)              $(GOALKEEPER)            $(OSSERVICES)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c MsgQueue.cpp


OSservices.o :         OSservices.cpp           $(OSSERVICES)            $(IMAGEIO)               $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c OSservices.cpp


PixelValue.o :         PixelValue.cpp           $(PIXELVALUE)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c PixelValue.cpp


Point.o :              Point.cpp                $(POINT)                 $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Point.cpp


RandomNumGenerator.o : RandomNumGenerator.cpp   $(RANDOMNUMGENERATOR)    $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c RandomNumGenerator.cpp


Raster.o :             Raster.cpp               $(RASTER)                $(BLOB)                  $(BMPIMAGE)        \
                       $(COMPRESSPR)            $(CONVEXHULL)            $(EIGENVECTOR)           $(GOALKEEPER)      \
                       $(HISTOGRAM)             $(IMAGEIO)               $(KKU_FFTW)              $(MATRIX)          \
                       $(OSSERVICES)            $(SIMPLECOMPRESSOR)      $(SOBEL)                 $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Raster.cpp  -I$(FFTWAPI)


RasterBuffer.o :       RasterBuffer.cpp         $(RASTERBUFFER)          $(BLOB)                  $(BMPIMAGE)   \
                       $(CONVEXHULL)            $(HISTOGRAM)             $(IMAGEIO)               $(OSSERVICES) \
                       $(SOBEL)
	$(CC) $(CFLAGS) -c RasterBuffer.cpp


RunLog.o :             RunLog.cpp               $(RUNLOG)                $(DATETIME)              $(OSSERVICES)  \
                       $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c $(LIBDIR)/$*.cpp


SegmentorOTSU.o :      SegmentorOTSU.cpp        $(SEGMENTOROTSU)         KKBaseTypes.h            $(RASTER)      \
                       $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c SegmentorOTSU.cpp


SimpleCompressor.o :   SimpleCompressor.cpp     $(SIMPLECOMPRESSOR)      $(OSSERVICES)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c $(LIBDIR)/$*.cpp


Sobel.o :              Sobel.cpp                KKBaseTypes.h            $(SOBEL)                 $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Sobel.cpp


StatisticalFunctions.o : StatisticalFunctions.cpp  $(STATISTICALFUNCTIONS)  KKBaseTypes.h         $(OSSERVICES)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c StatisticalFunctions.cpp


TokenBuffer.o :        TokenBuffer.cpp          $(TOKENBUFFER)           $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c TokenBuffer.cpp


Tokenizer.o :          Tokenizer.cpp            $(TOKENIZER)             $(STR)                   $(TOKENBUFFER)           $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Tokenizer.cpp


XmlStream.o :          XmlStream.cpp            $(XMLSTREAM)             KKBaseTypes.h            $(KKEXCEPTION)           $(TOKENIZER)             $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c XmlStream.cpp

