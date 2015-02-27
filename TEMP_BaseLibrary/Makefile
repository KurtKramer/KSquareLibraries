EXEC = BaseLibrary
CFLAGS = -O3 
#CFLAGS = -g
LIBS = -lfftw3 -lrt
#FFTWAPI = ../OutsidePackages/fftw-3.1.2/
FFTWAPI = /usr/include
#FFTWAPI = ../OutsidePackages/fftw-3.3.2/
ZLIBAPI = ../OutsidePackages/zlib123/
VPATH = ../BaseLibrary
LIBDIR = ../BaseLibrary

#CC = gcc -D FFTW_AVAILABLE
CC = gcc 

FIRSTINCLUDES      = FirstIncludes.h

RANDOMNUMGENERATOR = RandomNumGenerator.h  BasicTypes.h
KKU_FFTW           = kku_fftw.h            BasicTypes.h
EIGENVECTOR        = EigenVector.h         BasicTypes.h
KKQUEUE            = KKQueue.h             BasicTypes.h

STR                = Str.h                 BasicTypes.h          $(KKQUEUE)

BITSTRING          = BitString.h           BasicTypes.h          $(STR)
DATETIME           = DateTime.h            $(STR)
KKSTRPARSER        = KKStrParser.h         $(DATETIME)           $(STR)

RUNLOG             = RunLog.h              $(STR)
IMAGEIO            = ImageIO.h             $(STR)
OSSERVICES         = OSservices.h          $(STR)                $(DATETIME)        $(IMAGEIO)
HTMLREPORT         = HTMLReport.h          $(DATETIME)           $(STR)
GOALKEEPER         = GoalKeeper.h          $(STR)
GLOBALGOALKEEPER   = GlobalGoalKeeper.h    ${STR}
MATRIX             = Matrix.h              BasicTypes.h


COMPRESSPR         = Compressor.h          BasicTypes.h

STATISTICALFUNCTIONS = StatisticalFunctions.h  BasicTypes.h 

APPLICATION        = Application.h         $(STR)                $(RUNLOG)

IMAGESDIRTREE      = ImageDirTree.h        $(STR)

POINT              = Point.h               BasicTypes.h          $(KKQUEUE)         $(STR)

BLOB               = Blob.h                $(KKQUEUE)            $(POINT)

CONTOURFOLLOWER    = ContourFollower.h     BasicTypes.h          $(RUNLOG)
CONVEXHULL         = ConvexHull.h          BasicTypes.h
MSGQUEUE           = MsgQueue.h            $(GOALKEEPER)

KKTHREAD           = KKThread.h            $(MSGQUEUE)
KKTHREADMANAGER    = KKThreadManager.h     $(MSGQUEUE)           $(RUNLOG)

PIXELVALUE         = PixelValue.h          BasicTypes.h          $(STR)

RASTER             = Raster.h              BasicTypes.h          $(KKQUEUE)         $(STR)             $(PIXELVALUE)         $(POINT)
RASTERBUFFER       = RasterBuffer.h        $(GOALKEEPER)

BMPIMAGE           = BMPImage.h            BMPheader.h           $(STR)
HISTOGRAM          = Histogram.h           BasicTypes.h          $(STR)
SOBEL              = Sobel.h               $(STR)
SEGMENTOROTSU      = SegmentorOTSU.h       BasicTypes.h          $(MATRIX)          $(STR)             $(RASTER)         $(RUNLOG)
SIMPLECOMPRESSOR   = SimpleCompressor.h    
  
TOKENBUFFER        = TokenBuffer.h         $(STR)

TOKENIZER          = Tokenizer.h           $(TOKENBUFFER)

XMLSTREAM          = XmlStream.h           $(STR)                $(TOKENIZER)

CHART              = Chart.h               BasicTypes.h          $(STR)             $(RASTER)          $(POINT)

CONFIGURATION      = Configuration.h       $(STR)                $(RUNLOG)

CMDLINEEXPANDER    = CmdLineExpander.h     $(STR)                $(RUNLOG)


OBJECTS  = Application.o               BasicTypes.o                BitString.o                 Blob.o                 \
           BMPImage.o                  Chart.o                     CmdLineExpander.o           Compressor.o           \
           Configuration.o             ContourFollower.o           ConvexHull.o                DateTime.o             \
           EigenVector.o               GoalKeeper.o                GlobalGoalKeeper.o          Histogram.o            \
           HTMLReport.o           \
           ImageIO.o                   ImageDirTree.o              KKStrParser.o               kku_fftw.o             \
           KKThread.o                  KKThreadManager.o           Matrix.o                    MsgQueue.o             \
           OSservices.o                PixelValue.o                Point.o                     RandomNumGenerator.o   \
           Raster.o                    RasterBuffer.o              RunLog.o                    SegmentorOTSU.o        \
           SimpleCompressor.o          Sobel.o                     StatisticalFunctions.o      Str.o                  \
           TokenBuffer.o               Tokenizer.o                 XmlStream.o 


all:	$(EXEC)

clean:
	rm *.o; rm *.a
	
Clean:
	rm *.o; rm *.a
	


$(EXEC): $(OBJECTS) 
	ar cq libBaseLibrary.a  $(OBJECTS)



Application.o :        Application.cpp          $(APPLICATION)           $(CMDLINEEXPANDER)       $(DATETIME)              $(OSSERVICES)  \
                       $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Application.cpp


BasicTypes.o :         BasicTypes.cpp           BasicTypes.h             $(FIRSTINCLUDES) 
	$(CC) $(CFLAGS) -c BasicTypes.cpp


BitString.o :          BitString.cpp            $(BITSTRING)             BasicTypes.h             $(FIRSTINCLUDES)       
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
	$(CC) $(CFLAGS) -c Compressor.cpp -I$(ZLIBAPI)
 

Configuration.o :      Configuration.cpp        $(CONFIGURATION)         $(OSSERVICES)            $(RUNLOG)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Configuration.cpp


ContourFollower.o :    ContourFollower.cpp      $(CONTOURFOLLOWER)       $(KKU_FFTW)              $(RASTER)                $(STR)                   $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c ContourFollower.cpp   
#	$(CC) $(CFLAGS) -c ContourFollower.cpp -I$(FFTWAPI)   


ConvexHull.o :         ConvexHull.cpp           $(CONVEXHULL)            $(POINT)                 $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c $(LIBDIR)/$*.cpp


DateTime.o :           DateTime.cpp             $(DATETIME)              $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c $(LIBDIR)/$*.cpp


EigenVector.o :        EigenVector.cpp          $(EIGENVECTOR)           BasicTypes.h             $(FIRSTINCLUDES)
	$(CC)  $(CFLAGS) -c $(LIBDIR)/$*.cpp


GoalKeeper.o :         GoalKeeper.cpp           $(GOALKEEPER)            $(OSSERVICES)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c GoalKeeper.cpp


GlobalGoalKeeper.o :   GlobalGoalKeeper.cpp     $(GLOBALGOALKEEPER)      $(OSSERVICES)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c GlobalGoalKeeper.cpp


Histogram.o :          Histogram.cpp            $(HISTOGRAM)             $(RASTER)                $(IMAGEIO)               $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Histogram.cpp


HTMLReport.o :         HTMLReport.cpp           $(HTMLREPORT)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c  HTMLReport.cpp


ImageIO.o :            ImageIO.cpp              $(IMAGEIO)               $(OSSERVICES)            $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c ImageIO.cpp


ImageDirTree.o :       ImageDirTree.cpp         $(IMAGEDIRTREE)          $(IMAGEIO)               $(OSSERVICES)            $(STR)                   $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c ImageDirTree.cpp


KKStrParser.o :        KKStrParser.cpp          $(KKSTRPARSER)           BasicTypes.h             $(STR)                   $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c KKStrParser.cpp
	
	
KKThread.o :        KKThread.cpp                $(KKTHREAD)              BasicTypes.h             $(MSGQUEUE)              $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c KKThread.cpp


KKThreadManager.o : KKThreadManager.cpp         $(KKTHREADMANAGER)       $(KKTHREAD)              $(MSGQUEUE)              $(OSSERVICES) 
	$(CC) $(CFLAGS) -c KKThreadManager.cpp


kku_fftw.o :           kku_fftw.cpp             $(KKU_FFTW)              $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c kku_fftw.cpp 
#	$(CC) $(CFLAGS) -c kku_fftw.cpp  -I$(FFTWAPI) 


Matrix.o :             Matrix.cpp               $(MATRIX)                $(OSSERVICES)            $(STR)                   $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Matrix.cpp


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
	$(CC) $(CFLAGS) -c Raster.cpp
#	$(CC) $(CFLAGS) -c Raster.cpp  -I$(FFTWAPI)


RasterBuffer.o :       RasterBuffer.cpp         $(RASTERBUFFER)                $(BLOB)                 $(BMPIMAGE)  \
                       $(CONVEXHULL)            $(HISTOGRAM)             $(IMAGEIO)              $(OSSERVICES) \
                       $(SOBEL)
	$(CC) $(CFLAGS) -c RasterBuffer.cpp


RunLog.o :             RunLog.cpp               $(RUNLOG)                $(DATETIME)              $(OSSERVICES)  \
                       $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c $(LIBDIR)/$*.cpp


SegmentorOTSU.o :      SegmentorOTSU.cpp        $(SEGMENTOROTSU)         BasicTypes.h             $(RASTER)      \
                       $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c SegmentorOTSU.cpp


SimpleCompressor.o :   SimpleCompressor.cpp     $(SIMPLECOMPRESSOR)      $(OSSERVICES)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c $(LIBDIR)/$*.cpp


Sobel.o :              Sobel.cpp                BasicTypes.h             $(SOBEL)                 $(RASTER)                $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Sobel.cpp


StatisticalFunctions.o : StatisticalFunctions.cpp  $(STATISTICALFUNCTIONS)  BasicTypes.h          $(OSSERVICES)            $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c StatisticalFunctions.cpp


Str.o :                Str.cpp                  $(STR)                   $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c $(LIBDIR)/$*.cpp


TokenBuffer.o :        TokenBuffer.cpp          $(TOKENBUFFER)           $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c TokenBuffer.cpp


Tokenizer.o :          Tokenizer.cpp            $(TOKENIZER)             $(STR)                   $(TOKENBUFFER)           $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c Tokenizer.cpp


XmlStream.o :          XmlStream.cpp            $(XMLSTREAM)             $(STR)                   $(BITSTRING)              $(TOKENIZER)           $(FIRSTINCLUDES)
	$(CC) $(CFLAGS) -c XmlStream.cpp


