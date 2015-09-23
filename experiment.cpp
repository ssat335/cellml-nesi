#include <mpi.h>
#include "cellml_observer.h"
#include "distributor.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "AdvXMLParser.h"
#include "GAEngine.h"
#include "GAEngine.cpp"
#include "virtexp.h"
#include "utils.h"


using namespace std;
using namespace AdvXMLParser;

using namespace iface::cellml_api;
using namespace iface::cellml_services;

#define ZERO_LIM 1e-10

ObjRef<iface::cellml_api::CellMLBootstrap> bootstrap;	//CellML api bootstrap
ObjRef<iface::cellml_services::CellMLIntegrationService> cis;


VariablesHolder var_template;	//template for the variables, just holds names of the variables

int verbosity=0;	// verbosity initialised to 0

void usage(const char *name)
{
	std::cerr << "Error: usage: " << name << " <experiment definition xml> [-v [-v [...]]]\n";
	std::cerr << "Where -v increases the verbosity of the output" << std::endl;
}

//Open and read XML configuration file
//nSize is assigned the size of file
//return the contents of the file as a null-terminated char array
char *OpenXmlFile(const char *name,long& nSize)
{
    FILE *f=fopen(name,"rb");	// open file "name" for reading
    char *pBuffer=NULL;	// initialise a buffer for storing C-string to a nullptr

	//check for file open error
    if(!f)
        return NULL;
	
	//obtain file size
    fseek(f,0,SEEK_END);
    nSize=ftell(f);
    fseek(f,0,SEEK_SET);

	//allocate memory to contain the whole file
    pBuffer=new char[nSize+1];

    size_t result = fread(pBuffer,nSize,1,f);	//copy the file into buffer (usage of fread)

    // if result number differs from the count parameter, either a reading error occurred
    // or the end-of-file was reached while reading.
    if (result != 1) {
    	return NULL;
    }
    
	pBuffer[nSize]=0;	// null terminate the char array buffer
    fclose(f);

    return pBuffer;
}

// Initialise GA engine
// returns number of generations to run GA; -1 if any error occurred
int SetAndInitEngine(GAEngine<COMP_FUNC >& ga, const Element& elem)
{
	//Get GA parameters from XML file
    int initPopulation=atoi(elem.GetAttribute("InitialPopulation").GetValue().c_str());
    double mutation=atof(elem.GetAttribute("Mutation_proportion").GetValue().c_str());
    double cross=atof(elem.GetAttribute("Crossover_proportion").GetValue().c_str());
    int generations=atoi(elem.GetAttribute("Generations").GetValue().c_str());
    int RNG_type=atoi(elem.GetAttribute("RNG").GetValue().c_str());

    // Check for default and limit for params
    if(!initPopulation)
        initPopulation=100;
    if(cross>1.0)
        cross=1.0;
    if(mutation>1.0)
        mutation=1.0;
	if(generations<0)
	{
		std::cerr << "Error: SetAndInitEngine: invalid value for Generations: setting to default 0" << std::endl;
		generations=0;
	}
	if((RNG_type<0)||(RNG_type>1))
	{
		std::cerr << "Error: SetAndInitEngine: invalid value for RNG: setting to default linear-type RNG" << std::endl;
		RNG_type=0;
	}
    ga.prob_cross()=cross;
    ga.prob_mutate()=mutation;
    ga.part_cross()=(int)((double)initPopulation*cross);
    ga.part_mutate()=(int)((double)initPopulation*mutation);
    ga.RNG_method()=RNG_type;

    // Read alleles information
    for(int i=0;;i++)
    {
        const Element& al=elem("Alleles",0)("Allele",i);	// get sub-Allele element in XML
        std::wstring name;
        if(al.IsNull())
           break;	// no more alleles specified
        name=convert(al.GetAttribute("Name").GetValue());
		ga.AddAllele(name);

		// Validate parameter limits for non-negativity
		double min_lim=atof(al.GetAttribute("LowerBound").GetValue().c_str());
		double max_lim=atof(al.GetAttribute("UpperBound").GetValue().c_str());
		
		if(min_lim>max_lim)
		{
			std::cerr << "Error: SetAndInitEngine: invalid limits for Allele[" << i << "]: UpperBound should not be less than LowerBound: " << currentDateTime() << std::endl;
			return -1;
		}

		// Check for negative allele range for log-type RNG
		if((min_lim<0.0)&&(RNG_type==1))
		{
			std::cerr << "Error: SetAndInitEngine: Log-type RNG method does not support negative range: resetting RNG to linear-type (0): " << currentDateTime() << std::endl;
			
			// set RNG method to negative compatible linear-type
			RNG_type=0;
			ga.RNG_method()=0;
		}

		// TODO do below iff log-type is appropriate
		// reassign zero bounds to ZERO_LIM
		if(min_lim==0.0)
		{
			std::cerr << "Error: SetAndInitEngine: 0.0 is an invalid LowerBound for Allele[" << i << "]: resetting to ZERO_LIM=" << ZERO_LIM << ": " << currentDateTime() << std::endl;
			min_lim=ZERO_LIM;
		}
		if(max_lim==0.0)
		{
			std::cerr << "Error: SetAndInitEngine: 0.0 is an invalid UpperBound for Allele[" << i << "]: resetting to ZERO_LIM=" << ZERO_LIM << ": " << currentDateTime() << std::endl;
			max_lim=ZERO_LIM;
		}
		
        ga.AddLimit(name,min_lim,max_lim);
        var_template(name,0.0);		// update allele list in var_template
    }
    ga.set_borders(initPopulation);		// set max population of GA and initialise the population with default genomes

	// return the num of generations to run GA
	return generations;
}


//Initialise var_template with Alleles in XML; all param (allele) names are updated into var_template
void initialize_template_var(const Element& elem)
{
	// Read alleles information in XML
    for(int i=0;;i++)
    {
        const Element& al=elem("Alleles",0)("Allele",i);
        std::wstring name; 
        if(al.IsNull())
           break;	// no more alleles specified
        name=convert(al.GetAttribute("Name").GetValue());
        // Add this parameter as an allele in var_template
		var_template(name,0.0);
    }
}


// Observer callback to process workitem in the GA context
bool observer(WorkItem *w,double answer,void *g)
{
    GAEngine<COMP_FUNC> *ga=(GAEngine<COMP_FUNC> *)g;	// GAEngine typecasting
   
    ga->process_workitem(w,answer);		// assign 'answer' as fitness to the genome corresponding to the workitem w
    return true;
}


double do_compute(std::vector<double>& val)
{
	// fill-up the tmp's allele values with supplied data
    var_template.fillup(val);

	// Evaluate the chromosome fitness
    return VEGroup::instance().Evaluate(var_template);
}


// Slave process
// Returns only when appropriate quit command is received from the master
void run_slave(int proc)
{
    double req;
    MPI_Status stat;
    std::vector<double> data;

    var_template.collate(data);
    while(1)
    {
        //check if data is received
        MPI_Probe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&stat);
        if(stat.MPI_TAG==TAG_QUIT)
        {
            //Quit signal received
            break;
        }
        //Receive compute request and process it
        MPI_Recv(&data[0],data.size(),MPI_DOUBLE,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&stat);
        req=do_compute(data);
        //returns the result of the computations
        MPI_Send(&req,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD);
    }
}


int main(int argc,char *argv[])
{
    char *pBuffer=NULL;
    long nSize=0;
    GAEngine<COMP_FUNC > ga;	// initialise GA engine for the program
	int proc,nproc;
    int generations=0;
    const char *filename=NULL;

    MPI_Init(&argc,&argv);

    if(argc<2)
    {
		// warn user for incorrect usage at command line
        usage(argv[0]);
        return -1;
    }

    for(int i=1;i<argc;i++)
    {
        if(!strcmp(argv[i],"-v"))
			// if an arg string is "-v" increment verbosity
            verbosity++;
        else
			// other arg string becomes the filename
            filename=argv[i];
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &proc);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

	srand(time(NULL)*proc);		// unique seed for RNG

    //Load and initialise CellML API
    bootstrap=CreateCellMLBootstrap();
    cis=CreateIntegrationService();

	// Read input file and store contents in buffer
    if((pBuffer=OpenXmlFile(filename,nSize)) == NULL)
    {
		std::cerr << "Error: main: error opening input file " << argv[1] << ": " << currentDateTime() << std::endl;
		return -1;
    }
	// Read success: pBuffer is a C-string containing file and nSize is size of file

    //Load the experiments package: Virtual Experiment data and GA parameters
    try
    {
        Parser parser;

        ObjRef<CellMLComponentSet> comps;	// TODO comps not referenced elsewhere in project

		// Parse the XML contents in buffer
        auto_ptr<Document> pDoc(parser.Parse(pBuffer,nSize));	// can throw an exception
		// Get the root of the XML structure
        const Element& root=pDoc->GetRoot();

		// Load virtual experiments
		// load all virtual experiments in the XML file
		for(int i=0;;i++)
        {
			// load the VE data in file
            VirtualExperiment *vx=VirtualExperiment::LoadExperiment(root("VirtualExperiments",0)("VirtualExperiment",i));
			if(!vx)
               break;	// loaded all the VE in file
			
			// Quit program with err msg if VE is invalid
			if (!vx->isValid())
				return -1;

			// add each VE into the group singleton
            VEGroup::instance().add(vx);
        }
		
		// load the GA parameters from file and initialise the engine
        if(!proc)
        {
			// Master processor initialises the GA engine
			// get max generations (-1 for errors) and GA parameters
            generations=SetAndInitEngine(ga,root("GA",0));
        }
        else
        {
			// The slaves initialise the template variable holder to create gene-pool
            initialize_template_var(root("GA",0));
        }
    }
    catch(ParsingException &e)
    {
		std::cerr << "Error: main: parsing error at line " << e.GetLine() << std::endl;
    }
    delete [] pBuffer;	// free memory used to store file

	// Wait until all the clients are ready
    MPI_Barrier(MPI_COMM_WORLD);

    // Only master task needs GA engine to be initialised and used   
    if(!proc)
    {
		// Print a summary of configuration of GA engine and VEs
		ga.print_config(generations);
		VEGroup::instance().print_summary();

		// Validate and run GA
		if(generations<0)
		{
			std::cerr << "Error: main: invalid settings for the Genetic Algorithm: " << currentDateTime() << std::endl;
		}
		else
		{
			// Initialise the population in GA engine
			ga.Initialise();
			// Run GA
			ga.RunGenerations(generations);
        
			// Print best genome from the run
			VariablesHolder v;
			double bf=ga.GetBest(v);
			std::cout << "==========================================\n";
			fprintf(stdout,"BEST GENOME (%lf):\n",bf);
			v.print(stdout);
			std::cout << "==========================================\n";
		}

        Distributor::instance().finish();	// request end of service to all slaves
    }
    else
    {
        run_slave(proc);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    return 0;
}
