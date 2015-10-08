#ifndef GA_ENGINE_H
#define GA_ENGINE_H

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <limits>
#include <functional>
#include <time.h>
#include <stdlib.h>
#ifdef SUPPORT_MPI
#include <mpi.h>
#endif
#include <limits>
#include "utils.h"
#include "virtexp.h"
#include "distributor.h"
#include <math.h>
#include "Genome.h"

extern int verbosity;



bool reverse_compare(const Genome& v1,const Genome& v2);
extern bool observer(WorkItem *w,double answer,void *);


template<class COMP>
class GAEngine
{
    private:
        typedef std::vector<Genome> POPULATION;
        POPULATION m_Population;
        int m_MaxPopulation;
        std::vector<std::wstring> m_AlleleList;
        double m_CrossProbability;
        double m_MutationProbability;
        int m_crossPartition;
        int m_mutatePartition;
        int m_Generations;
        double m_bestFitness;
        bool m_bBestFitnessAssigned;
        VariablesHolder m_bestVariables;
		int m_RNG;		// switch for altenative RNG method

    public:
        typedef Genome GENOME;

        GAEngine();
        ~GAEngine();

        double& prob_cross() { return m_CrossProbability; }
        double& prob_mutate() { return m_MutationProbability; }
        int& part_cross() { return m_crossPartition; }
        int& part_mutate() { return m_mutatePartition; }
		
		// Get RNG method selector
		int& RNG_method() { return m_RNG; }

		// Set the maximum population of GA
        void set_borders(int max_population);

		// Initialise the population with randomly generated genomes
        bool Initialise();

		// Size of a GAEngine
        int size() { return m_Population.size(); }

		// Add a new genotype into the central gene-pool
        void AddAllele(const std::wstring& name);

		// Set limits to an allele's values
        void AddLimit(const std::wstring& name,double lower,double upper);
		
		// Copy the gene-pool into a temporary storage
        void var_template(VariablesHolder& v);

		// Get the currently fittest genome and its fitness and store them
        double GetBest(VariablesHolder& v);

		// Create a work item to be processed from chromosome data
        WorkItem *var_to_workitem(VariablesHolder& h);

		// Process the results in the workitem by updating the fitness of the corresponding genome
        void process_workitem(WorkItem *w,double answer);

		// Run the GA engine for given number of generations
        void RunGenerations(int gener);

		// Print settings specific to the GA Engine
		void print_config(const int gener);

    private:
        typedef std::map<std::wstring,std::pair<double,double> > LIMITS;
        LIMITS m_Limits;

		// Print the genetic data of a member in population
		void print_genome(POPULATION& population, int ind_genome);

		// Print the genetic data of the current population
		void print_population(POPULATION& population);

		// Output a current summary of GA
		// TODO different verbosity settings
        void print_stage(int g);

		// Mutation operator
		// if name is an empty wstring, mutate all alleles
		// else only the allele with matching name is mutated at the mutation rate
		// setting mutate_all to false will mutate approx. just 1 allele
        void mutate(const std::wstring& name,Genome& g,bool mutate_all=false);

		// Crossover operator
		// swap allele-string before the crosspoint
        bool cross(Genome& one,Genome& two, int crosspoint);

		// Append a defined number of randomly selected indices to genomes onto a vector
        void build_rnd_sample(std::vector<int>& sample,int count,bool reject_duplicates,bool check_valid);
		
        void build_rnd_sample_tournament(std::vector<int>& sample,int count,bool reject_duplicates,bool check_valid);

        void build_rnd_sample_rnd(std::vector<int>& sample,double prob,bool check_valid);

		// Randomly select a member from the population using the inverse of fitness as the weight (Roullete selection)
        int select_weighted(POPULATION& p);
};

#endif

