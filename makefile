
# User can provide compiler mpicxx for local compilation. 
# By default the compiler is mpiicpc

ifndef $CC
	CC=mpiicpc
endif


# User-configuration
##############################################
XMLPARSER_PATH=/projects/uoa00322/AdvXMLParser
CELLML_PATH=/projects/uoa00322/cellml-sdk
##############################################

CFLAGS=-I $(XMLPARSER_PATH) -I $(CELLML_PATH)/include -O
LFLAGS=-L $(XMLPARSER_PATH) -ladvxml -L $(CELLML_PATH)/lib -lcellml -lcis -Wl,-rpath=$(CELLML_PATH)/lib 

SOURCES=RandomValueGeneratorBoxConcept.cpp VariablesHolder.cpp Genome.cpp experiment.cpp VirtualExperiment.cpp Utils.cpp LocalProgressObserver.cpp Distributor.cpp 
INCLUDES=RandomValueGeneratorBoxConcept.cpp VariablesHolder.h Genome.h VirtualExperiment.h Utils.h GAEngine.h GAEngine.cpp LocalProgressObserver.h Distributor.h

all: cellml-fitter

# Debug build flag that can be passed as make DEBUG=true
debug: CFLAGS += -g #-O0 -Wall -Wextra
debug: cellml-fitter

cellml-fitter: $(SOURCES) $(INCLUDES)
	$(CC) $(CFLAGS) $(SOURCES) -o cellml-fitter $(LFLAGS)

clean:
	rm -f cellml-fitter  *~ *.o
