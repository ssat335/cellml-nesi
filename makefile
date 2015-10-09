
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

SOURCES=Genome.cpp experiment.cpp virtexp.cpp utils.cpp cellml_observer.cpp distributor.cpp 
INCLUDES=Genome.h utils.h GAEngine.h GAEngine.cpp cellml_observer.h distributor.h

all: cellml-fitter

# Debug build flag that can be passed as make DEBUG=true
debug: CFLAGS += -g #-O0 -Wall -Wextra
debug: cellml-fitter



cellml-fitter: $(SOURCES) $(INCLUDES)
	$(CC) $(CFLAGS) $(SOURCES) -o cellml-fitter $(LFLAGS)

clean:
	rm -f cellml-fitter  *~ *.o
