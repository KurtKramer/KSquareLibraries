EXEC = KKBase
#CFLAGS = -O3 
CFLAGS = -g  
LIBS = -lfftw -lrt
FFTWAPI = ../OutsidePackages/fftw-3.3.2/api 
VPATH = ../KKBase
IN_PATH = .
OUT_PATH = ./debug
INPUTFILE = -c $(IN_PATH)/$*.cpp
OUTPUTFILE = -o $(OUT_PATH)/$*.o

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
KKOBSERVER         = KKObserver.h          $(MSGQUEUE)
KKOBSERVABLE       = KKObservable.h        $(MSGQUEUE)

KKTHREAD           = KKThread.h            $(MSGQUEUE)
KKTHREADMANAGER    = KKThreadManager.h     $(MSGQUEUE)           $(RUNLOG)

BITSTRING          = BitString.h           KKBaseTypes.h         $(ATOM)            $(STR)      $(XMLSTREAM)
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
MORPHOPSTRUCT      = MorphOpStruct.h       $(MORPHOP)
MORPHOPBINARIZE    = MorphOpBinarize.h     KKBaseTypes.h          $(MORPHOP)         $(MORPHOPSTRUCT)

MORPHOPBMIFILTERING = MorphOpBmiFiltering.h  KKBaseTypes.h          $(MORPHOP)


MORPHOPDILATION    = MorphOpDilation.h     KKBaseTypes.h          $(MORPHOP)

MORPHOPEREROSION   = MorphOpErosion.h      KKBaseTypes.h          $(MORPHOP)
MORPHOPMASKEXCLUDE = MorphOpMaskExclude.h  KKBaseTypes.h          $(MORPHOP)
MORPHOPSTRETCHER   = MorphOpStretcher.h    KKBaseTypes.h          $(MORPHOP)



BMPIMAGE           = BMPImage.h            BMPheader.h            $(STR)
HISTOGRAM          = Histogram.h           KKBaseTypes.h          $(STR)
MORPHOPSOBEL       = MorphOpSobel.h        $(STR)
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
           KKException.o               KKObservable.o              KKStrMatrix.o               KKStr.o                \
           KKStrMatrix.o          \
           KKStrParser.o               KKThread.o                  KKThreadManager.o           kku_fftw.o             \
           Matrix.o                    MorphOp.o                   MorphOpBinarize.o           MorphOpBmiFiltering.o  \
           MorphOpErosion.o       \
           MorphOpMaskExclude.o        MorphOpStretcher.o          MsgQueue.o                  OSservices.o           \
           PixelValue.o                Point.o                     RandomNumGenerator.o        Raster.o               \
           RasterBuffer.o              RunLog.o                    SegmentorOTSU.o             SimpleCompressor.o     \
           MorphOpSobel.o              StatisticalFunctions.o      TokenBuffer.o               Tokenizer.o            \
           XmlStream.o


all:	$(EXEC)

clean:
	rm *.o; rm *.a	


$(EXEC): $(OBJECTS) 
	rm $(OUT_PATH)/*.a ; ar cq $(OUT_PATH)/libKKBase.a  $(OUT_PATH)/*.o


Application.o :        Application.cpp          $(APPLICATION)           $(CMDLINEEXPANDER)       $(DATETIME)              $(OSSERVICES)  \
                       $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)  $(INPUTFILE)  $(OUTPUTFILE)


Atom.o :               Atom.cpp                 $(ATOM)                  KKBaseTypes.h            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


KKBaseTypes.o :        KKBaseTypes.cpp          KKBaseTypes.h            $(FIRSTINCLUDES) 
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


BitString.o :          BitString.cpp            $(BITSTRING)             KKBaseTypes.h            $(FIRSTINCLUDES)       
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


Blob.o :               Blob.cpp                 $(BLOB)                  $(BITSTRING)             $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


BMPImage.o :           BMPImage.cpp             $(BMPIMAGE)              $(OSSERVICES)            $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


Chart.o :              Chart.cpp                $(CHART)                 $(IMAGEIO)               $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


CmdLineExpander.o :    CmdLineExpander.cpp      $(CMDLINEEXPANDER)       $(OSSERVICES)            $(STR)                   $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


Compressor.o :         Compressor.cpp           $(COMPRESSOR)            $(OSSERVICES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)
 

Configuration.o :      Configuration.cpp        $(CONFIGURATION)         $(OSSERVICES)            $(RUNLOG)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


ContourFollower.o :    ContourFollower.cpp      $(CONTOURFOLLOWER)       $(KKU_FFTW)              $(RASTER)                $(STR)    \
                       $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)  -I$(FFTWAPI)


ConvexHull.o :         ConvexHull.cpp           $(CONVEXHULL)            $(POINT)                 $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


DateTime.o :           DateTime.cpp             $(DATETIME)              $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


EigenVector.o :        EigenVector.cpp          $(EIGENVECTOR)           KKBaseTypes.h            $(FIRSTINCLUDES)
	$(CC)  $(CFLAGS)  $(INPUTFILE)  $(OUTPUTFILE)


GlobalGoalKeeper.o :   GlobalGoalKeeper.cpp     $(GLOBALGOALKEEPER)      $(GOALKEEPER)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


GoalKeeper.o :         GoalKeeper.cpp           $(GOALKEEPER)            $(OSSERVICES)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


Histogram.o :          Histogram.cpp            $(HISTOGRAM)             $(RASTER)                $(IMAGEIO)               $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


HTMLReport.o :         HTMLReport.cpp           $(HTMLREPORT)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


ImageIO.o :            ImageIO.cpp              $(IMAGEIO)               $(OSSERVICES)            $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


ImageDirTree.o :       ImageDirTree.cpp         $(IMAGEDIRTREE)          $(IMAGEIO)               $(OSSERVICES)            $(STR)                   $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


KKException.o :        KKException.cpp          $(KKEXCEPTION)           $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


KKObserver.o :         KKObserver.cpp           $(KKOBSERVER)            $(FIRSTINCLUDES)         $(GLOBALGOALKEEPER)      KKBaseTypes.h   \
                       $(KKEXCEPTION)           $(OSSERVICES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


KKObservable.o :       KKObservable.cpp         $(KKOBSERVABLE)          $(GLOBALGOALKEEPER)      KKBaseTypes.h            $(KKEXCEPTION)  \
                       $(KKOBSERVER)            $(OSSERVICES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


KKStr.o :              KKStr.cpp                $(STR)                   $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


KKStrMatrix.o :        KKStrMatrix.cpp          $(KKSTRMATRIX)           $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


KKStrParser.o :        KKStrParser.cpp          $(KKSTRPARSER)           KKBaseTypes.h            $(STR)                   $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


kku_fftw.o :           kku_fftw.cpp             $(KKU_FFTW)              $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


KKThread.o :           KKThread.cpp             $(KKTHREAD)              $(KKEXCEPTION)           $(MSGQUEUE)              $(OSSERVICES)       $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


KKThreadManager.o :    KKThreadManager.cpp      $(KKTHREADMANAGER)       $(KKTHREAD)              $(MSGQUEUE)              $(OSSERVICES)             $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


Matrix.o :             Matrix.cpp               $(MATRIX)                $(OSSERVICES)            $(STR)                   $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


MorphOp.o :            MorphOp.cpp              $(MORPHOP)               $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


MorphOpBinarize.o :    MorphOpBinarize.cpp      $(MORPHOPBINARIZE)       $(MORPHOP)               $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


MorphOpBmiFiltering.o : MorphOpBmiFiltering.cpp $(MORPHOPBMIFILTERING)   $(MORPHOP)               $(MORPHOPMASKEXCLUDE)    $(RASTER)   \
                        $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


MorphOpDilation.o :    MorphOpDilation.cpp      $(MORPHOPDILATION)       $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


MorphOpErosion.o :     MorphOpErosion.cpp       $(MORPHOPEREROSION)      $(MORPHOP)               $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


MorphOpMaskExclude.o : MorphOpMaskExclude.cpp   $(MORPHOPMASKEXCLUDE)    $(MORPHOP)               $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


MorphOpSobel.o :       MorphOpSobel.cpp         KKBaseTypes.h            $(MORPHOPSOBEL)          $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


MorphOpStretcher.o : MorphOpStretcher.cpp       $(MORPHOPSTRETCHER)    $(MORPHOP)               $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


MsgQueue.o :           MsgQueue.cpp             $(MsgQueue)              $(GOALKEEPER)            $(OSSERVICES)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


OSservices.o :         OSservices.cpp           $(OSSERVICES)            $(IMAGEIO)               $(FIRSTINCLUDES)   \
                       $(KKEXCEPTION)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


PixelValue.o :         PixelValue.cpp           $(PIXELVALUE)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


Point.o :              Point.cpp                $(POINT)                 $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


RandomNumGenerator.o : RandomNumGenerator.cpp   $(RANDOMNUMGENERATOR)    $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


Raster.o :             Raster.cpp               $(RASTER)                KKBaseTypes.h            $(BLOB)            \
                       $(BMPIMAGE)              $(COMPRESSPR)            $(CONVEXHULL)            $(EIGENVECTOR)     \
                       $(GOALKEEPER)            $(HISTOGRAM)             $(IMAGEIO)               $(KKEXCEPTION)     \
                       $(KKU_FFTW)              $(MATRIX)                $(MORPHOPBINARIZE)       $(MORPHOPDILATION) \
                       $(MORPHOPEREROSION)      $(MORPHOPSTRETCHER)      $(MORPHOPSOBEL)          $(OSSERVICES)      \
                       $(SIMPLECOMPRESSOR)      $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE) -I$(FFTWAPI)


RasterBuffer.o :       RasterBuffer.cpp         $(RASTERBUFFER)          $(BLOB)                  $(BMPIMAGE)   \
                       $(CONVEXHULL)            $(HISTOGRAM)             $(IMAGEIO)               $(OSSERVICES) \
                       $(MORPHOPSOBEL)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


RunLog.o :             RunLog.cpp               $(RUNLOG)                $(DATETIME)              $(OSSERVICES)  \
                       $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


SegmentorOTSU.o :      SegmentorOTSU.cpp        $(SEGMENTOROTSU)         KKBaseTypes.h            $(RASTER)      \
                       $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


SimpleCompressor.o :   SimpleCompressor.cpp     $(SIMPLECOMPRESSOR)      $(OSSERVICES)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


StatisticalFunctions.o : StatisticalFunctions.cpp  $(STATISTICALFUNCTIONS)  KKBaseTypes.h         $(OSSERVICES)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


TokenBuffer.o :        TokenBuffer.cpp          $(TOKENBUFFER)           $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


Tokenizer.o :          Tokenizer.cpp            $(TOKENIZER)             $(STR)                   $(TOKENBUFFER)           $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


XmlStream.o :          XmlStream.cpp            $(XMLSTREAM)             KKBaseTypes.h            $(KKEXCEPTION)           $(TOKENIZER)  \
                       $(FIRSTINCLUDES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)

