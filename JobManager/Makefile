EXEC = libJobManager
LIBS = -lfftw -lrt
VPATH = ../BaseLibrary
LIBDIR = ../BaseLibrary 
FFTWAPI = ../OutsidePackages/fftw-3.2.2/api
CFLAGS = -O3 -I$(LIBDIR)
#CFLAGS = -g   -I$(LIBDIR)

CC = g++

KKJOB              = KKJob.h
KKJOBMAMAGER       = KKJobManager.h   KKJob.h

OBJECTS	= KKJob.o  KKJobManager.o
          


all:	$(EXEC)

clean:
	rm *.o;  rm *.a	


$(EXEC): $(OBJECTS) 
	rm *.a; ar cq libJobManager.a  $(OBJECTS)






KKJob.o: KKJob.cpp  $(KKJOB)
	$(CC) $(CFLAGS) -c $*.cpp

KKJobManager.o: KKJobManager.cpp    $(KKJOBMAMAGER)  $(KKJOB)
	$(CC) $(CFLAGS) -c $*.cpp
































