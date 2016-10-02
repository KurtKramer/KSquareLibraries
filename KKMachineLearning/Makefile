EXEC = libKKMachineLearning
LIBS = -lfftw -lrt
LIBDIR = ../KKBase  ../KKLineScanner
FFTWAPI = ../OutsidePackages/fftw-3.1.2/api
IN_PATH = .
OUT_PATH = ./debug
INPUTFILE = -c $(IN_PATH)/$*.cpp
OUTPUTFILE = -o $(OUT_PATH)/$*.o
VPATH = $(OUT_PATH)

CFLAGS = -O3 -I ../KKBase  -I ../KKLineScanner  
#CFLAGS = -g  -I $(LIBDIR)


CC = g++ -W -Wall -Wextra -pedantic -std=c++0x 


IMAGEFEATURESDATAINDEXED = ImageFeaturesDataIndexed.h   
IMAGEFEATURESNAMEINDEXED = ImageFeaturesNameIndexed.h   

ATTRIBUTE          = Attribute.h           $(FIRSTINCLUDES)

FEATURENUMLIST     = FeatureNumList.h      $(ATTRIBUTE)    

MLCLASS            = MLClass.h

FILEDESC           = FileDesc.h            $(ATTRIBUTE)          $(MLCLASS)

ORDERINGS          = Orderings.h           $(FILEDESC)

CLASSSTATISTIC     = ClassStatistic.h      $(MLCLASS)

FEATUREVECTOR      = FeatureVector.h       $(ATTRIBUTE)          $(CLASSSTATISTIC)     FeatureFileIO.h   \
                     $(FEATURENUMLIST)     $(FILEDESC)           $(MLCLASS)

FACTORYFVPRODUCER  = FactoryFVProducer.h

FEATUREVECTORPRODUCER = FeatureVectorProducer.h

FEATUREFILEIO      = FeatureFileIO.h       $(FEATURENUMLIST)     $(FEATUREVECTOR)      $(MLCLASS)

FEATUREFILEIOARFF    = FeatureFileIOArff.h     $(FEATUREFILEIO) 
FEATUREFILEIOC45     = FeatureFileIOC45.h      $(FEATUREFILEIO) 
FEATUREFILEIOCOLUMN  = FeatureFileIOColumn.h   $(FEATUREFILEIO)
FEATUREFILEIODSTWEB  = FeatureFileIODstWeb.h   $(FEATUREFILEIO)
FEATUREFILEIOROBERTS = FeatureFileIORoberts.h  $(FEATUREFILEIO)  
FEATUREFILEIOSPARSE  = FeatureFileIOSparse.h   $(FEATUREFILEIO)
FEATUREFILEIOUCI     = FeatureFileIOUCI.h      $(FEATUREFILEIO)

NORMALIZATIONPARMS = NormalizationParms.h  $(FEATURENUMLIST)     $(ATTRIBUTE)

DUPLICATEIMAGES    = DuplicateImages.h     $(FEATUREVECTOR)

CLASSASSIGNMENTS   = ClassAssignments.h

CONFUSSIONMATRIX2  = ConfusionMatrix2.h   $(MLCLASS)

SVM                = svm.h

SVM2               = svm2.h                $(FEATURENUMLIST)     $(FEATUREVECTOR)

BINARYCLASSPARMS   = BinaryClassParms.h    $(SVM)                $(FEATURENUMLIST)


SVMPARAM           = SVMparam.h


SVMMODEL           = SVMModel.h            $(CLASSASSIGNMENTS)   $(FEATUREVECTOR)      $(FILEDESC)  \
                     $(MLCLASS)            $(SVM)                $(SVMPARAM)


FEATUREENCODER     = FeatureEncoder.h      $(ATTRIBUTE)          $(FEATUREVECTOR)   \
                     $(FILEDESC)           $(IMAGECLASS)         $(SVM)                $(SVMPARAM)


MODELPARAM         = ModelParam.h          $(FEATURENUMLIST)     $(FILEDESC)


MODELPARAMKNN      = ModelParamKnn.h       $(MODELPARAM)


MODELPARAMOLDSVM   = ModelParamOldSVM.h    $(BINARYCLASSPARMS)   $(FEATURENUMLIST)     $(MLCLASS)  \
                     $(FILEDESC)           $(MODELPARAM)         $(SVM)                $(SVMPARAM)


MODELPARAMSVMBASE  = ModelParamSvmBase.h   $(MODELPARAM)         $(SVM2)


FEATUREENCODER2    = FeatureEncoder2.h     $(ATTRIBUTE)          $(FEATURENUMLIST)  \
                     $(FEATUREVECTOR)      $(FILEDESC)           $(MLCLASS)           $(MODELPARAM)  


MODEL              = Model.h               $(CLASSASSIGNMENTS)   $(FEATUREENCODER2)    $(FEATUREVECTOR)      \
                     $(FILEDESC)           $(MLCLASS)            $(MODELPARAM)         $(NORMALIZATIONPARMS) \
                     $(SVM)                $(SVMPARAM)     


MODELKNN           = ModelKnn.h            $(FEATUREVECTOR)      $(MODEL)              $(MODELPARAMKNN)


MODELOLDSVM        = $(BINARYCLASSPARMS)   $(FILEDESC)           $(MLCLASS)            $(FEATUREENCODER2)    \
                     $(FEATURENUMLIST)     $(FEATUREVECTOR)      $(SVM)                $(MODEL)              \
                     $(MODELPARAMOLDSVM)   $(SVMMODEL)


MODELSVMBASE       = ModelSvmBase.h        $(FEATURENUMLIST)     $(FEATUREVECTOR)      $(FILEDESC)           \
                     $(MLCLASS)            $(MODEL)              $(MODELPARAM)         $(MODELPARAMSVMBASE)  \
                     $(SVM2)
                     

SIZEDISTRIBUTION   = SizeDistribution.h    $(MLCLASS)         $(FEATUREVECTOR)



SVMWRAPPER         = SvmWrapper.h          $(SVMPARAM)


TRAININGCLASS      = TrainingClass.h


TRAININGCONFIGURATION2 = TrainingConfiguration2.h  \
                     $(FILEDESC)           $(FEATUREVECTOR)      $(MODEL)          $(MODELPARAM)        \
                     $(MODELPARAMOLDSVM)   $(SVMPARAM)           $(TRAININGCLASS)


TRAININGPROCESS2   = TrainingProcess2.h    $(FEATUREVECTOR)      $(IMAGEFEATURESDATAINDEXED)   \
                     $(IMAGEFEATURESNAMEINDEXED)                 $(NORMALIZATIONPARMS)         \
                     $(MODEL)              $(MODELOLDSVM)        $(SVMMODEL)                   \
                     $(TRAININGCONFIGURATION2)


CLASSIFIER2        = Classifier2.h         $(MODEL)          $(MODELPARAMOLDSVM)    $(MODELOLDSVM)  \
                     $(MODELPARAMOLDSVM)   $(SVMMODEL)    


CROSSVALIDATION        = CrossValidation.h
CROSSVALIDATIONVOTING  = CrossValidationVoting.h
CROSSVALIDATIONMXN     = CrossValidationMxN.h

CLASSIFICATIONBIASMATRIX = ClassificationBiasMatrix.h  ConfusionMatrix2.h   $(MLCLASS)


CMDLINEEXPANDER = CmdLineExpander.h


OBJECTS  = Attribute.o                  BinaryClassParms.o           \
           ClassAssignments.o           \
           Classifier2.o                ClassStatistic.o             ClassificationBiasMatrix.o   \
           ConfusionMatrix2.o           CrossValidation.o            CrossValidationMxN.o         \
           CrossValidationVoting.o      DuplicateImages.o            \
           FactoryFVProducer.o          FeatureVectorProducer.o                                   \
           FeatureEncoder.o             FeatureEncoder2.o            FeatureFileIO.o              \
           FeatureFileIOArff.o          FeatureFileIOC45.o           FeatureFileIOColumn.o        \
           FeatureFileIODstWeb.o        FeatureFileIORoberts.o       \
           FeatureFileIOSparse.o        FeatureFileIOUCI.o           FeatureNumList.o             \
           FeatureVector.o              FileDesc.o                   MLClass.o                    \
           ImageFeaturesDataIndexed.o   ImageFeaturesNameIndexed.o   \
           Model.o                      ModelSvmBase.o               ModelKnn.o                   \
           ModelOldSVM.o                ModelParam.o                 ModelParamSvmBase.o          \
           ModelParamKnn.o              ModelParamOldSVM.o           NormalizationParms.o         \
           Orderings.o                    \
           SizeDistribution.o           svm.o                        svm2.o                       \
           SVMModel.o                   SVMparam.o                   SvmWrapper.o                 \
           TrainingClass.o              TrainingConfiguration2.o     \
           TrainingProcess2.o
           

all:	$(EXEC)

clean:
	rm $(OUT_PATH)/*.o; rm $(OUT_PATH)/*.a	


$(EXEC): $(OBJECTS) 
	rm $(OUT_PATH)/*.a ; ar cq $(OUT_PATH)/KKMachineLearning.a  $(OUT_PATH)/*.o



Attribute.o : Attribute.cpp   $(ATTRIBUTE)   $(FEATURENUMLIST)  $(MLCLASS)  $(IMAGEFEATURES)  
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


BinaryClassParms.o :   BinaryClassParms.cpp  $(BINARYCLASSPARMS) $(MLCLASS) svm.h
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


ClassAssignments.o :   ClassAssignments.cpp     $(CLASSASSIGNMENTS)     $(MLCLASS)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


Classifier2.o :        Classifier2.cpp          $(CLASSIFIER2)            $(MLCLASS)   \
                       $(NORMALIZATIONPARMS)    $(TRAININGPROCESS2)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


ClassStatistic.o : ClassStatistic.cpp  $(CLASSSTATISTIC)   $(MLCLASS)   
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


ClassificationBiasMatrix.o : ClassificationBiasMatrix.cpp  $(CLASSIFICATIONBIASMATRIX)    $(MLCLASS)   
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


ConfusionMatrix2.o :  ConfusionMatrix2.cpp    $(CONFUSSIONMATRIX2)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


CrossValidation.o :   ./CrossValidation.cpp          $(CROSSVALIDATION)     $(CLASSIFIER)            \
                      $(CONFUSSIONMATRIX2)           $(FEATUREVECTOR)       $(TRAININGCONFIGURATION2) \
                      $(TRAININGPROCESS)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)



CrossValidationMxN.o :  ./CrossValidationMxN.cpp  \
                        $(CONFUSSIONMATRIX2)   $(CROSSVALIDATION)         $(FEATUREVECTOR)    \
                        $(ORDERINGS)           $(TRAININGCONFIGURATION2)  $(TRAININGPROCESS2)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)



CrossValidationVoting.o : CrossValidationVoting.cpp  $(CROSSVALIDATIONVOTING)                              \
                          $(CLASSIFIER)              $(CONFUSSIONMATRIX2)        $(FEATUREVECTOR)          \
                          $(FILEDESC)                $(MLCLASS)                  $(TRAININGCONFIGURATION2) \
                          $(TRAININGPROCESS)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)



DuplicateImages.o :  DuplicateImages.cpp         $(DUPLICATEIMAGES)           $(FEATUREVECTOR) \
                     $(IMAGEFEATURESNAMEINDEXED) $(IMAGEFEATURESDATAINDEXED)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


FactoryFVProducer.o : FactoryFVProducer.cpp     $(FEATUREVECTOR)    $(FEATUREVECTORPRODUCER)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


FeatureVectorProducer.o :  FeatureVectorProducer.cpp  $(FEATUREVECTORPRODUCER )
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


FeatureEncoder.o :   FeatureEncoder.cpp     $(FEATUREENCODER)   $(BINARYCLASSPARMS) \
                    $(FEATURENUMLIST)       $(FEATUREVECTOR)    $(SVMWRAPPER)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


FeatureEncoder2.o :  FeatureEncoder2.cpp    $(FEATUREENCODER2)  $(BINARYCLASSPARMS)    $(FEATURENUMLIST) \
                     $(FEATUREVECTOR)        $(SVMWRAPPER)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


FeatureFileIO.o :   FeatureFileIO.cpp  $(FEATUREFILEIO)  $(FILEDESC)  $(MLCLASS)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


FeatureFileIOArff.o : FeatureFileIOArff.cpp  $(FEATUREFILEIOARFF)  $(FILEDESC)  $(MLCLASS)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


FeatureFileIOC45.o : FeatureFileIOC45.cpp  $(FEATUREFILEIOC45)   $(FILEDESC)  $(MLCLASS)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


FeatureFileIOColumn.o : FeatureFileIOColumn.cpp  $(FEATUREFILEIOCOLUMN)   $(FILEDESC)  $(MLCLASS)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


FeatureFileIODstWeb.o : FeatureFileIODstWeb.cpp  $(FEATUREFILEIODSTWEB)    $(FILEDESC)  $(MLCLASS)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


FeatureFileIORoberts.o : FeatureFileIORoberts.cpp  $(FEATUREFILEIOROBERTS)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


FeatureFileIOSparse.o : FeatureFileIOSparse.cpp  $(FEATUREFILEIOSPARSE)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


FeatureFileIOUCI.o : FeatureFileIOUCI.cpp  $(FEATUREFILEIOUCI)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


FeatureNumList.o :   FeatureNumList.cpp  $(FEATURENUMLIST)  $(FILEDESC)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


FeatureVector.o :   FeatureVector.cpp  $(FEATUREVECTOR) $(FEATURENUMLIST)  $(FEATUREFILEIO)  $(MLCLASS) 
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


FileDesc.o :        FileDesc.cpp  $(FILEDESC)  $(FEATURENUMLIST)  $(MLCLASS)  $(IMAGEFEATURES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


MLClass.o :         MLClass.cpp   $(MLCLASS)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


ImageFeatures.o :      ImageFeatures.cpp  $(IMAGEFEATURES)  $(FEATURENUMLIST)  $(FEATUREFILEIO)  $(MLCLASS)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE) 


ImageFeaturesDataIndexed.o : ImageFeaturesDataIndexed.cpp   $(IMAGEFEATURESDATAINDEXED)  $(IMAGEFEATURES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


ImageFeaturesNameIndexed.o : ImageFeaturesNameIndexed.cpp  $(IMAGEFEATURESNAMEINDEXED)   $(IMAGEFEATURES)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


ModelParam.o :  ModelParam.cpp       $(MODELPARAM)        $(FILEDESC)            $(MLCLASS)        \
                $(MODELPARAM)        $(MODELPARAMKNN)     $(MODELPARAMOLDSVM)    $(MODELPARAMSVMBASE)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


ModelParamKnn.o :  ModelParamKnn.cpp       $(MODELPARAMKNN)    $(FILEDESC)   $(MLCLASS)         
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


ModelParamOldSVM.o :  ModelParamOldSVM.cpp       $(MODELPARAMOLDSVM)    $(BINARYCLASSPARMS) $(FILEDESC)   $(MLCLASS) 
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


ModelParamSvmBase.o :  ModelParamSvmBase.cpp       $(MODELPARAMSVMBASE)    $(FILEDESC)   $(MLCLASS) 
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


Model.o :  Model.cpp  $(MODEL)    $(BINARYCLASSPARMS)  $(FEATUREENCODER) \
                    $(FEATURENUMLIST)  $(FEATUREVECTOR)  $(SVMWRAPPER)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


ModelKnn.o :          ModelKnn.h   $(MODELKNN)  $(MODEL)  $(FEATURENUMLIST)  $(FEATUREVECTOR)  $(SVMWRAPPER)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


ModelOldSVM.o :        ModelOldSVM.cpp          $(BINARYCLASSPARMS)      $(FEATUREENCODER2)    $(FEATURENUMLIST)  \
                       $(FEATUREVECTOR)         $(MODELPARAMOLDSVM)      $(SVMPARAM)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


ModelSvmBase.o :       ModelSvmBase.cpp         $(MODELSVMBASE)          $(FEATURENUMLIST)  \
                       $(FEATUREVECTOR)         $(SVM2)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


NormalizationParms.o : NormalizationParms.cpp   $(NORMALIZATIONPARMS)    $(FEATURENUMLIST)    \
                       $(MLCLASS)               $(FEATUREVECTOR)         $(TRAININGCONFIGURATION2)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


Orderings.o :          Orderings.cpp            $(ORDERINGS)             $(FEATUREFILEIO)    $(FILEDESC)   \
                       $(MLCLASS)               $(FEATUREVECTOR)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


SizeDistribution.o : SizeDistribution.cpp $(SIZEDISTRIBUTION)  $(MLCLASS) $(FEATUREVECTOR)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


svm.o :                svm.cpp              $(SVM)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)
	

svm2.o :               svm2.cpp             $(SVM2)              $(FEATUREENCODER)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


SVMModel.o :           SVMModel.cpp         $(SVMMODEL)           \
                       $(BINARYCLASSPARMS)  $(FEATUREENCODER)    $(FEATURENUMLIST) \
                       $(FEATUREVECTOR)     $(SVMWRAPPER)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)



SVMparam.o :           SVMparam.cpp            $(SVMPARAM)    $(BINARYCLASSPARMS)  \
                       $(CLASSASSIGNMENTS)     $(FILEDESC)$   $(MLCLASS)   
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


SvmWrapper.o :         SvmWrapper.cpp           $(SVMWRAPPER)           $(SVM)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


TrainingClass.o :      TrainingClass.cpp        $(TRAININGCLASS)        $(MLCLASS)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


TrainingConfiguration2.o : TrainingConfiguration2.cpp  $(TRAININGCONFIGURATION2)      $(BINARYCLASSPARMS)    \
                           $(FACTORYFVPRODUCER)        $(FEATUREVECTORPRODUCER)                              \
                           $(FEATUREFILEIO)            $(FEATUREFILEIOPICES)          $(FEATURENUMLIST)      \
                           $(FEATUREVECTOR)            $(FILEDESC)                    $(MLCLASS)          \
                           $(IMAGEFEATURES)            $(MODEL)                       $(MODELPARAMKNN)       \
                           $(MODELPARAMOLDSVM)         $(MODELPARAMSVMBASE)           $(NORMALIZATIONPARMS)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


TrainingProcess2.o :   TrainingProcess2.cpp         $(TRAININGPROCESS2)          $(CLASSASSIGNMENTS)      \
                       $(DUPLICATEIMAGES)           $(FEATUREFILEIO)             $(FEATUREFILEIOPICES)    \
                       $(FEATURENUMLIST)            $(FEATUREVECTOR)             $(MLCLASS)            \
                       $(MODEL)                     $(MODELOLDSVM)               $(MODELKNN)              \
                       $(MODELSVMBASE)              $(TRAININGCLASS)             $(TRAININGCONFIGURATION2)
	$(CC) $(CFLAGS)   $(INPUTFILE)  $(OUTPUTFILE)


