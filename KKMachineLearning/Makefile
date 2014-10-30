EXEC = libPICL
LIBS = -lfftw -lrt
VPATH = ../KKBase
LIBDIR = ../KKBase  ../KKLineScanner
FFTWAPI = ../OutsidePackages/fftw-3.1.2/api

CFLAGS = -O3 -I ../KKBase  -I ../KKLineScanner  
#CFLAGS = -g  -I $(LIBDIR)


CC = g++


IMAGEFEATURESDATAINDEXED = ImageFeaturesDataIndexed.h   
IMAGEFEATURESNAMEINDEXED = ImageFeaturesNameIndexed.h   

ATTRIBUTE          = Attribute.h           $(FIRSTINCLUDES)

FEATURENUMLIST     = FeatureNumList.h      $(ATTRIBUTE)    

IMAGECLASS         = ImageClass.h

FILEDESC           = FileDesc.h            $(ATTRIBUTE)          $(IMAGECLASS)

ORDERINGS          = Orderings.h           $(FILEDESC)

CLASSSTATISTIC     = ClassStatistic.h      $(IMAGECLASS)

FEATUREVECTOR      = FeatureVector.h       $(ATTRIBUTE)          $(CLASSSTATISTIC)     FeatureFileIO.h   \
                     $(FEATURENUMLIST)     $(FILEDESC)           $(IMAGECLASS)

FACTORYFVPRODUCER  = FactoryFVProducer.h
FEATUREVECTORPRODUCER = FeatureVectorProducer.h

IMAGEFEATURES      = ImageFeatures.h       $(ATTRIBUTE)          $(CLASSSTATISTIC)     $(FEATURENUMLIST) \
                     $(FEATUREVECTOR)      $(FILEDESC)

FEATUREFILEIO      = FeatureFileIO.h       $(FEATURENUMLIST)     $(FEATUREVECTOR)      $(IMAGECLASS)

FEATUREFILEIOARFF    = FeatureFileIOArff.h     $(FEATUREFILEIO) 
FEATUREFILEIOC45     = FeatureFileIOC45.h      $(FEATUREFILEIO) 
FEATUREFILEIOCOLUMN  = FeatureFileIOColumn.h   $(FEATUREFILEIO)
FEATUREFILEIODSTWEB  = FeatureFileIODstWeb.h   $(FEATUREFILEIO)
FEATUREFILEIOKK      = FeatureFileIOKK.h       $(FEATUREFILEIO)   $(IMAGEFEATURES)
FEATUREFILEIOROBERTS = FeatureFileIORoberts.h  $(FEATUREFILEIO)  
FEATUREFILEIOSPARSE  = FeatureFileIOSparse.h   $(FEATUREFILEIO)
FEATUREFILEIOUCI     = FeatureFileIOUCI.h      $(FEATUREFILEIO)

NORMALIZATIONPARMS = NormalizationParms.h  $(FEATURENUMLIST)     $(ATTRIBUTE)

DUPLICATEIMAGES    = DuplicateImages.h     $(IMAGEFEATURES)

CLASSASSIGNMENTS   = ClassAssignments.h

CHAINHASH          = ChainHash.h     $(FEATURENUMLIST)  $(ATTRIBUTE)

BITREDUCTION       = BitReduction.h  $(CHAINHASH)  $(IMAGEFEATURES)  svm.h

CONFUSSIONMATRIX2  = ConfusionMatrix2.h   $(IMAGECLASS)

SVM                = svm.h

SVM2               = svm2.h                $(FEATURENUMLIST)     $(FEATUREVECTOR)

BINARYCLASSPARMS   = BinaryClassParms.h    $(SVM)                $(FEATURENUMLIST)


SVMPARAM           = SVMparam.h


SVMMODEL           = SVMModel.h            $(CLASSASSIGNMENTS)   $(FEATUREVECTOR)      $(FILEDESC)  \
                     $(IMAGECLASS)         $(SVM)                $(BITREDUCTION)       $(SVMPARAM)


FEATUREENCODER     = FeatureEncoder.h      $(ATTRIBUTE)          $(BITREDUCTION)       $(FEATUREVECTOR)   \
                     $(FILEDESC)           $(IMAGECLASS)         $(SVM)                $(SVMPARAM)


MODELPARAM         = ModelParam.h          $(FEATURENUMLIST)     $(FILEDESC)


MODELPARAMKNN      = ModelParamKnn.h       $(MODELPARAM)


MODELPARAMOLDSVM   = ModelParamOldSVM.h    $(BINARYCLASSPARMS)   $(FEATURENUMLIST)     $(IMAGECLASS)  \
                     $(FILEDESC)           $(MODELPARAM)         $(SVM)                $(SVMPARAM)


MODELPARAMSVMBASE  = ModelParamSvmBase.h   $(MODELPARAM)         $(SVM2)


FEATUREENCODER2    = FeatureEncoder2.h     $(ATTRIBUTE)          $(BITREDUCTION)       $(FEATURENUMLIST)  \
                     $(FEATUREVECTOR)      $(FILEDESC)           $(IMAGECLASS)         $(MODELPARAM)  


MODEL              = Model.h               $(CLASSASSIGNMENTS)   $(FEATUREENCODER2)    $(FEATUREVECTOR)      \
                     $(FILEDESC)           $(IMAGECLASS)         $(MODELPARAM)         $(NORMALIZATIONPARMS) \
                     $(SVM)                $(SVMPARAM)     


MODELKNN           = ModelKnn.h            $(FEATUREVECTOR)      $(MODEL)              $(MODELPARAMKNN)


MODELOLDSVM        = $(BINARYCLASSPARMS)   $(FILEDESC)           $(IMAGECLASS)         $(FEATUREENCODER2)    \
                     $(FEATURENUMLIST)     $(FEATUREVECTOR)      $(SVM)                $(MODEL)              \
                     $(MODELPARAMOLDSVM)   $(SVMMODEL)


MODELSVMBASE       = ModelSvmBase.h        $(FEATURENUMLIST)     $(FEATUREVECTOR)      $(FILEDESC)           \
                     $(IMAGECLASS)         $(MODEL)              $(MODELPARAM)         $(MODELPARAMSVMBASE)  \
                     $(SVM2)
                     

SIZEDISTRIBUTION   = SizeDistribution.h    $(IMAGECLASS)         $(IMAGEFEATURES)



SVMWRAPPER         = SvmWrapper.h          $(SVMPARAM)


TRAININGCLASS      = TrainingClass.h


TRAININGCONFIGURATION = TrainingConfiguration.h  $(SVMPARAM)             $(TRAININGCLASS)


TRAININGCONFIGURATION2 = TrainingConfiguration2.h  \
                     $(FILEDESC)           $(FEATUREVECTOR)      $(MODEL)          $(MODELPARAM)        \
                     $(MODELPARAMOLDSVM)   $(SVMPARAM)           $(TRAININGCLASS)


TRAININGPROCESS = TrainingProcess.h        $(IMAGECLASS)                \
                  $(IMAGEFEATURES)         $(IMAGEFEATURESDATAINDEXED)   $(IMAGEFEATURESNAMEINDEXED)  \
                  $(NORMALIZATIONPARMS)    svm.h                         $(SVMMODEL)                  \
                  $(TRAININGCONFIGURATION)


TRAININGPROCESS2   = TrainingProcess2.h    $(FEATUREVECTOR)      $(IMAGEFEATURESDATAINDEXED)   \
                     $(IMAGEFEATURESNAMEINDEXED)                 $(NORMALIZATIONPARMS)         \
                     $(MODEL)              $(MODELOLDSVM)        $(SVMMODEL)                   \
                    $(TRAININGCONFIGURATION2)


CLASSIFIER         = Classifier.h


CLASSIFIER2        = Classifier2.h         $(MODEL)          $(MODELPARAMOLDSVM)    $(MODELOLDSVM)  \
                     $(MODELPARAMOLDSVM)   $(SVMMODEL)    


CROSSVALIDATIONTHREEWAY = CrossValidationThreeWay.h

CROSSVALIDATION        = CrossValidation.h         $(BITREDUCTION)
CROSSVALIDATIONVOTING  = CrossValidationVoting.h
CROSSVALIDATIONMXN     = CrossValidationMxN.h

CLASSIFICATIONBIASMATRIX = ClassificationBiasMatrix.h  ConfusionMatrix2.h   ImageClass.h


CMDLINEEXPANDER = CmdLineExpander.h


OBJECTS  = Attribute.o                  BinaryClassParms.o           BitReduction.o               \
           ChainHash.o                  ClassAssignments.o                            \
           Classifier2.o                ClassStatistic.o             ClassificationBiasMatrix.o   \
           ConfusionMatrix2.o           CrossValidation2.o           CrossValidationMxN.o         \
           CrossValidationThreeWay.o    CrossValidationVoting.o      DuplicateImages.o            \
           FactoryFVProducer.o          FeatureVectorProducer.o                                   \
           FeatureEncoder.o             FeatureEncoder2.o            FeatureFileIO.o              \
           FeatureFileIOArff.o          FeatureFileIOC45.o           FeatureFileIOColumn.o        \
           FeatureFileIODstWeb.o        FeatureFileIOKK.o            FeatureFileIORoberts.o       \
           FeatureFileIOSparse.o        FeatureFileIOUCI.o           FeatureNumList.o             \
           FeatureVector.o              FileDesc.o                   ImageClass.o                 \
           ImageFeatures.o              ImageFeaturesDataIndexed.o   ImageFeaturesNameIndexed.o   \
           Model.o                      ModelSvmBase.o               ModelKnn.o                   \
           ModelOldSVM.o                ModelParam.o                 ModelParamSvmBase.o          \
           ModelParamKnn.o              ModelParamOldSVM.o           NormalizationParms.o         \
           Orderings.o                  ScannerExtractionImageManager.o                            \
           SizeDistribution.o           svm.o                        svm2.o                       \
           SVMModel.o                   SVMparam.o                   SvmWrapper.o                 \
           TrainingClass.o              TrainingConfiguration2.o     \
           TrainingProcess2.o
           

$(EXEC): $(OBJECTS) 
	ar cq libPicesLibrary.a  $(OBJECTS)


Attribute.o : Attribute.cpp   $(ATTRIBUTE)   $(FEATURENUMLIST)  $(IMAGECLASS)  $(IMAGEFEATURES)  
	$(CC) $(CFLAGS) -c Attribute.cpp


BinaryClassParms.o :   BinaryClassParms.cpp  $(BINARYCLASSPARMS) $(IMAGECLASS) svm.h
	$(CC) $(CFLAGS) -c BinaryClassParms.cpp


BitReduction.o :       BitReduction.cpp         $(BITREDUCTION)    $(FEATUREVECTOR)     $(FILEDESC)
	$(CC) $(CFLAGS) -c BitReduction.cpp


ChainHash.o :          ChainHash.cpp            $(CHAINHASH)       $(FEATURENUMLIST)
	$(CC) $(CFLAGS) -c ChainHash.cpp


ClassAssignments.o :   ClassAssignments.cpp     $(CLASSASSIGNMENTS)     $(IMAGECLASS)
	$(CC) $(CFLAGS) -c ClassAssignments.cpp


Classifier2.o :        Classifier2.cpp          $(CLASSIFIER2)            $(IMAGECLASS)   \
                       $(NORMALIZATIONPARMS)    $(TRAININGPROCESS2)
	$(CC) $(CFLAGS) -c Classifier2.cpp


ClassStatistic.o : ClassStatistic.cpp  $(CLASSSTATISTIC)   $(IMAGECLASS)   
	$(CC) $(CFLAGS) -c ClassStatistic.cpp


ClassificationBiasMatrix.o : ClassificationBiasMatrix.cpp  $(CLASSIFICATIONBIASMATRIX)    $(IMAGECLASS)   
	$(CC) $(CFLAGS) -c ClassificationBiasMatrix.cpp


ConfusionMatrix2.o :  ConfusionMatrix2.cpp    $(CONFUSSIONMATRIX2)
	$(CC) $(CFLAGS) -c ConfusionMatrix2.cpp


CrossValidation2.o :  ./CrossValidation.cpp          $(CROSSVALIDATION)     $(CLASSIFIER)            \
                      $(CONFUSSIONMATRIX2)           $(FEATUREVECTOR)       $(TRAININGCONFIGURATION) \
                      $(TRAININGPROCESS)
	$(CC) $(CFLAGS) -c ./CrossValidation.cpp  -o CrossValidation2.o



CrossValidationMxN.o :  ./CrossValidationMxN.cpp  \
                        $(CONFUSSIONMATRIX2)   $(CROSSVALIDATION)         $(FEATUREVECTOR)    \
                        $(ORDERINGS)           $(TRAININGCONFIGURATION)   $(TRAININGPROCESS)
	$(CC) $(CFLAGS) -c CrossValidationMxN.cpp  -o CrossValidationMxN.o


CrossValidationThreeWay.o : CrossValidationThreeWay.cpp  $(CROSSVALIDATIONTHREEWAY)     $(CLASSIFIER)     \
                            $(CONFUSSIONMATRIX2)         $(FEATUREVECTOR)               $(IMAGECLASS)     \
                            $(TRAININGCLASS)             $(TRAININGCONFIGURATION)       $(TRAININGPROCESS)
	$(CC) $(CFLAGS) -c CrossValidationThreeWay.cpp


CrossValidationVoting.o : CrossValidationVoting.cpp  $(CROSSVALIDATIONVOTING)                             \
                          $(CLASSIFIER)              $(CONFUSSIONMATRIX2)        $(FEATUREVECTOR)         \
                          $(FILEDESC)                $(IMAGECLASS)               $(TRAININGCONFIGURATION) \
                          $(TRAININGPROCESS)
	$(CC) $(CFLAGS) -c CrossValidationVoting.cpp



DuplicateImages.o :  DuplicateImages.cpp         $(DUPLICATEIMAGES)           $(FEATUREVECTOR) \
                     $(IMAGEFEATURESNAMEINDEXED) $(IMAGEFEATURESDATAINDEXED)
	$(CC) $(CFLAGS) -c DuplicateImages.cpp


FactoryFVProducer.o  FactoryFVProducer.cpp     $(FEATUREVECTOR)    $(FEATUREVECTORPRODUCER)
	$(CC)  $(CFLAGS) -c FactoryFVProducer.cpp


FeatureVectorProducer.o   FeatureVectorProducer.cpp  $(FEATUREVECTORPRODUCER )
	$(CC)  $(CFLAGS) -c FeatureVectorProducer.cpp


FeatureEncoder.o :   FeatureEncoder.cpp     $(FEATUREENCODER)   $(BITREDUCTION)  $(BINARYCLASSPARMS) \
                    $(FEATURENUMLIST)       $(FEATUREVECTOR)    $(SVMWRAPPER)
	$(CC)  $(CFLAGS) -c FeatureEncoder.cpp


FeatureEncoder2.o :  FeatureEncoder2.cpp    $(FEATUREENCODER2)  $(BINARYCLASSPARMS)    $(FEATURENUMLIST) \
                     $(FEATUREVECTOR)        $(SVMWRAPPER)
	$(CC)  $(CFLAGS) -c FeatureEncoder2.cpp


FeatureFileIO.o :   FeatureFileIO.cpp  $(FEATUREFILEIO)  $(FILEDESC)  $(IMAGECLASS)
	$(CC) $(CFLAGS) -c FeatureFileIO.cpp


FeatureFileIOArff.o : FeatureFileIOArff.cpp  $(FEATUREFILEIOARFF)  $(FILEDESC)  $(IMAGECLASS)
	$(CC) $(CFLAGS) -c FeatureFileIOArff.cpp 


FeatureFileIOC45.o : FeatureFileIOC45.cpp  $(FEATUREFILEIOC45)   $(FILEDESC)  $(IMAGECLASS)
	$(CC) $(CFLAGS) -c FeatureFileIOC45.cpp


FeatureFileIOColumn.o : FeatureFileIOColumn.cpp  $(FEATUREFILEIOCOLUMN)   $(FILEDESC)  $(IMAGECLASS)
	$(CC) $(CFLAGS) -c FeatureFileIOColumn.cpp


FeatureFileIODstWeb.o : FeatureFileIODstWeb.cpp  $(FEATUREFILEIODSTWEB)    $(FILEDESC)  $(IMAGECLASS)
	$(CC) $(CFLAGS) -c FeatureFileIODstWeb.cpp


FeatureFileIOKK.o : FeatureFileIOKK.cpp  $(FEATUREFILEIOKK)   $(FILEDESC)  $(IMAGECLASS)  $(IMAGEFEATURES)
	$(CC) $(CFLAGS) -c FeatureFileIOKK.cpp


FeatureFileIORoberts.o : FeatureFileIORoberts.cpp  $(FEATUREFILEIOROBERTS)
	$(CC) $(CFLAGS) -c FeatureFileIORoberts.cpp


FeatureFileIOSparse.o : FeatureFileIOSparse.cpp  $(FEATUREFILEIOSPARSE)
	$(CC) $(CFLAGS) -c FeatureFileIOSparse.cpp


FeatureFileIOUCI.o : FeatureFileIOUCI.cpp  $(FEATUREFILEIOUCI)
	$(CC) $(CFLAGS) -c FeatureFileIOUCI.cpp


FeatureNumList.o :   FeatureNumList.cpp  $(FEATURENUMLIST)  $(FILEDESC)
	$(CC) $(CFLAGS) -c FeatureNumList.cpp

FeatureVector.o :   FeatureVector.cpp  $(FEATUREVECTOR) $(FEATURENUMLIST)  $(FEATUREFILEIO)  $(IMAGECLASS) 
	$(CC) $(CFLAGS) -c FeatureVector.cpp


FileDesc.o :        FileDesc.cpp  $(FILEDESC)  $(FEATURENUMLIST)  $(IMAGECLASS)  $(IMAGEFEATURES)
	$(CC) $(CFLAGS) -c FileDesc.cpp


ImageClass.o :         ImageClass.cpp   $(IMAGECLASS)
	$(CC) $(CFLAGS) -c ImageClass.cpp


ImageFeatures.o :      ImageFeatures.cpp  $(IMAGEFEATURES)  $(FEATURENUMLIST)  $(FEATUREFILEIO)  $(IMAGECLASS)
	$(CC) $(CFLAGS) -c ImageFeatures.cpp 


ImageFeaturesDataIndexed.o : ImageFeaturesDataIndexed.cpp   $(IMAGEFEATURESDATAINDEXED)  $(IMAGEFEATURES)
	$(CC) $(CFLAGS) -c ImageFeaturesDataIndexed.cpp


ImageFeaturesNameIndexed.o : ImageFeaturesNameIndexed.cpp  $(IMAGEFEATURESNAMEINDEXED)   $(IMAGEFEATURES)
	$(CC) $(CFLAGS) -c ImageFeaturesNameIndexed.cpp


ModelParam.o :  ModelParam.cpp       $(MODELPARAM)        $(FILEDESC)            $(IMAGECLASS)        \
                $(MODELPARAM)        $(MODELPARAMKNN)     $(MODELPARAMOLDSVM)    $(MODELPARAMSVMBASE)
	$(CC) $(CFLAGS) -c ModelParam.cpp


ModelParamKnn.o :  ModelParamKnn.cpp       $(MODELPARAMKNN)    $(FILEDESC)   $(IMAGECLASS)         
	$(CC) $(CFLAGS) -c ModelParamKnn.cpp


ModelParamOldSVM.o :  ModelParamOldSVM.cpp       $(MODELPARAMOLDSVM)    $(BINARYCLASSPARMS) $(FILEDESC)   $(IMAGECLASS) 
	$(CC) $(CFLAGS) -c ModelParamOldSVM.cpp


ModelParamSvmBase.o :  ModelParamSvmBase.cpp       $(MODELPARAMSVMBASE)    $(FILEDESC)   $(IMAGECLASS) 
	$(CC) $(CFLAGS) -c ModelParamSvmBase.cpp


Model.o :  Model.cpp  $(MODEL)    $(BITREDUCTION)  $(BINARYCLASSPARMS)  $(FEATUREENCODER) \
                    $(FEATURENUMLIST)  $(FEATUREVECTOR)  $(SVMWRAPPER)
	$(CC) $(CFLAGS) -c Model.cpp


ModelKnn.o :          ModelKnn.h   $(MODELKNN)  $(MODEL)  $(FEATURENUMLIST)  $(FEATUREVECTOR)  $(SVMWRAPPER)
	$(CC) $(CFLAGS) -c ModelKnn.cpp


ModelOldSVM.o :        ModelOldSVM.cpp          $(BINARYCLASSPARMS)      $(FEATUREENCODER2)    $(FEATURENUMLIST)  \
                       $(FEATUREVECTOR)         $(MODELPARAMOLDSVM)      $(SVMPARAM)
	$(CC) $(CFLAGS) -c ModelOldSVM.cpp


ModelSvmBase.o :       ModelSvmBase.cpp         $(MODELSVMBASE)          $(FEATURENUMLIST)  \
                       $(FEATUREVECTOR)         $(SVM2)
	$(CC) $(CFLAGS) -c ModelSvmBase.cpp


NormalizationParms.o : NormalizationParms.cpp   $(NORMALIZATIONPARMS)    $(FEATURENUMLIST)    \
                       $(IMAGECLASS)            $(FEATUREVECTOR)         $(TRAININGCONFIGURATION)
	$(CC) $(CFLAGS) -c NormalizationParms.cpp


Orderings.o :          Orderings.cpp            $(ORDERINGS)             $(FEATUREFILEIO)    $(FILEDESC)   \
                       $(IMAGECLASS)            $(FEATUREVECTOR)
	$(CC) $(CFLAGS) -c Orderings.cpp


ScannerExtractionImageManager.o : ScannerExtractionImageManager.cpp    \
                                 $(FEATUREFILEIO)    $(IMAGECLASS)  $(IMAGEFEATURES)
	$(CC) $(CFLAGS) -c ScannerExtractionImageManager.cpp


SizeDistribution.o : SizeDistribution.cpp $(SIZEDISTRIBUTION)  $(IMAGECLASS) $(FEATUREVECTOR)
	$(CC) $(CFLAGS) -c SizeDistribution.cpp


svm.o :                svm.cpp              $(SVM)
	$(CC) $(CFLAGS) -c svm.cpp
	

svm2.o :               svm2.cpp             $(SVM2)              $(FEATUREENCODER)
	$(CC) $(CFLAGS) -c svm2.cpp


SVMModel.o :           SVMModel.cpp         $(SVMMODEL)          $(BITREDUCTION)   \
                       $(BINARYCLASSPARMS)  $(FEATUREENCODER)    $(FEATURENUMLIST) \
                       $(FEATUREVECTOR)     $(SVMWRAPPER)
	$(CC) $(CFLAGS) -c SVMModel.cpp



SVMparam.o :           SVMparam.cpp            $(SVMPARAM)    $(BINARYCLASSPARMS)  \
                       $(CLASSASSIGNMENTS)     $(FILEDESC)$   $(IMAGECLASS)   
	$(CC) $(CFLAGS) -c SVMparam.cpp


SvmWrapper.o :         SvmWrapper.cpp           $(SVMWRAPPER)           $(SVM)
	$(CC) $(CFLAGS) -c SvmWrapper.cpp


TrainingClass.o :      TrainingClass.cpp        $(TRAININGCLASS)        $(IMAGECLASS)
	$(CC) $(CFLAGS) -c TrainingClass.cpp


TrainingConfiguration.o : TrainingConfiguration.cpp   $(TRAININGCONFIGURATION)      $(BINARYCLASSPARMS)    \
                          $(FEATUREFILEIO)            $(FEATUREFILEIOPICES)         $(FEATUREVECTOR)       \
                          $(FILEDESC)                 $(IMAGECLASS)                 $(NORMALIZATIONPARMS)
	$(CC) $(CFLAGS) -c TrainingConfiguration.cpp



TrainingConfiguration2.o : TrainingConfiguration2.cpp  $(TRAININGCONFIGURATION2)      $(BINARYCLASSPARMS)    \
                           $(FACTORYFVPRODUCER)        $(FEATUREVECTORPRODUCER)                              \
                           $(FEATUREFILEIO)            $(FEATUREFILEIOPICES)          $(FEATURENUMLIST)      \
                           $(FEATUREVECTOR)            $(FILEDESC)                    $(IMAGECLASS)          \
                           $(IMAGEFEATURES)            $(MODEL)                       $(MODELPARAMKNN)       \
                           $(MODELPARAMOLDSVM)         $(MODELPARAMSVMBASE)           $(NORMALIZATIONPARMS)
	$(CC) $(CFLAGS) -c TrainingConfiguration2.cpp


TrainingProcess2.o :   TrainingProcess2.cpp         $(TRAININGPROCESS2)          $(CLASSASSIGNMENTS)      \
                       $(DUPLICATEIMAGES)           $(FEATUREFILEIO)             $(FEATUREFILEIOPICES)    \
                       $(FEATURENUMLIST)            $(FEATUREVECTOR)             $(IMAGECLASS)            \
                       $(MODEL)                     $(MODELOLDSVM)               $(MODELKNN)              \
                       $(MODELSVMBASE)              $(TRAININGCLASS)             $(TRAININGCONFIGURATION2)
	$(CC) $(CFLAGS)  -c TrainingProcess2.cpp


