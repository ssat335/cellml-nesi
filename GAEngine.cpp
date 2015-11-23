#include "GAEngine.h"
#include "Genome.h"
#include "RandomValueGeneratorBoxConcept.h"
#include <set>

bool reverse_compare(const Genome& v1,const Genome& v2) 
{
	return (v1<v2); 
}


template<class COMP>
GAEngine<COMP>::GAEngine():m_MaxPopulation(0),m_Generations(1),
					m_CrossProbability(0.2),m_MutationProbability(0.01),
					m_bBestFitnessAssigned(false),
					m_crossPartition(0),m_mutatePartition(0)
{
}

template<class COMP>
GAEngine<COMP>::~GAEngine()
{
}
 
template<class COMP>
void GAEngine<COMP>::set_borders(int max_population)
{
	m_MaxPopulation=max_population;
	m_Population.resize(max_population);
}

// Initialise the population with randomly generated genomes
template<class COMP>
bool GAEngine<COMP>::Initialise()
{
	// Create a representative genome for the population
	Genome v;

	// exit exceptions
	if(!m_Population.size() || !m_AlleleList.size())
		return false;

	// Attach alleles to the representative genome
	for(typename std::vector<std::wstring>::iterator it=m_AlleleList.begin();it!=m_AlleleList.end();++it)	// iterate through the list of alleles
	{
		// Create each allele with all values initialised to 0.0
		v.allele(*it,(double)0.0);
	}

	// Fill the population with each mutation of the genome
	for(int i=0;i<m_Population.size();i++)
	{
		mutate(std::wstring(),v,true);	// mutate all the alleles of v
		m_Population[i]=v;				// a randomly generated genome enters the population
	}

	return true;
}

template<class COMP>
void GAEngine<COMP>::AddAllele(const std::wstring& name)
{
    m_AlleleList.push_back(name);
}

template<class COMP>
void GAEngine<COMP>::AddLimit(const std::wstring& name,double lower,double upper)
{
    m_Limits[name]=std::make_pair(double(lower),double(upper));
}

template<class COMP>
void GAEngine<COMP>::var_template(VariablesHolder& v)
{
    for(std::vector<std::wstring>::iterator it=m_AlleleList.begin();it!=m_AlleleList.end();++it)
    {
        v(*it,0.0);
    }
}

template<class COMP>
double GAEngine<COMP>::GetBest(VariablesHolder& v)
{
    v=m_bestVariables;
    return m_bestFitness;
}

template<class COMP>
WorkItem *GAEngine<COMP>::var_to_workitem(VariablesHolder& h)
{
    WorkItem *w=new WorkItem;
    w->key=0;	// key initialised to 0
    h.collate(w->data);
    return w;
}

template<class COMP>
void GAEngine<COMP>::process_workitem(WorkItem *w,double answer)
{
    if(w->key<m_Population.size())
    {
        Genome& g=m_Population[w->key];		// get the genome corresponding to this workitem
        g.fitness(answer);	// assign the evaluated fitness to the genome
    }
    delete w;
}

// Run the GA engine for given number of generations
template<class COMP>
void GAEngine<COMP>::RunGenerations(int gener)
{
	VariablesHolder v;		// temporary genome storage
	m_Generations=gener;	// number of generations to run

	// Create initial fitness set
	for(int i=0;i<m_Population.size();i++)
	{
		Genome& g=m_Population[i];	// get the ith Genome in population

		g.var(v);	// store the genomic information in a temporary variable
		WorkItem *w=var_to_workitem(v);		// collate the genome in a workitem for processing
		w->key=i;	// the key stores a reference to genome

		Distributor::instance().push(w);	// push this work onto the distributor singleton
	}

	// Process the work item collected by distributor
	// evaluate and assign fitness for each Genome in population
	Distributor::instance().process(observer,this);

	std::sort(m_Population.begin(),m_Population.end(),reverse_compare);		// sort population in ascending order of fitness

	// Update fittest genome
	// the fittest genome after sorting population is the 0th member
	if(!m_bBestFitnessAssigned || m_bestFitness>m_Population[0].fitness())
	{
		m_bestFitness=m_Population[0].fitness();	// assign min ftns as the best fitness
		m_Population[0].var(m_bestVariables);		// update the bestVars
		m_bBestFitnessAssigned=true;
	}

	print_stage(0);

	for(int g=1;g<=gener;g++)
	{
		// Do the genetics
		int limit=m_Population.size();
		POPULATION prev(m_Population);	// temporary copy of current pop vector
		m_Population.clear();			// clear current population vector

		// SELECTION
		// Weighted-selection with replacement from the previous generation's population
		for(int i=0;i<limit;i++)
		{
			int mem=select_weighted(prev);		// genome's index [p.size()-1 when err]

			// Genetic operator feedback
			if(verbosity>3)
				printf("SELECT: Adding %d to population\n",mem);

			m_Population.push_back(prev[mem]);	// add the selected genome into the new population for breeding
		}

		// Print the new selected population
		if(verbosity>2)
		{
			printf("--------------------------------------------------------\n");
			printf("Selected Population:\n");
			print_population(m_Population);
			printf("--------------------------------------------------------\n");
		}

		// CROSSOVER
		if(m_crossPartition)
		{
			double prob = m_crossPartition * 10;

			// Declare a set to store the populations available for crossover
			std::set<int> pendingGenomes;

			// Initialise the set with all the population indexes
			for (int i = 0; i < m_Population.size(); ++i)
			    pendingGenomes.insert(pendingGenomes.end(), i);

			// Initialise a set to store the crossovered indices
			std::set<int> xoverIndices;

			// perform cross over here until genomes are available for crossover
			while(pendingGenomes.size() > 1)
			{
				// pick 'top' member and remove from any future crossover
				std::set<int>::iterator it = pendingGenomes.begin();
				int index_mate1 = *it;
				pendingGenomes.erase(it);

				//  pick potential mate from remaining members randomly and remove from any future crossover
				double  rnd_select = rnd_generate(0, pendingGenomes.size() - 1 );
				it = pendingGenomes.begin();
				std::advance(it, round(rnd_select));
				int index_mate2 = *it;
				pendingGenomes.erase(it);

				// check the chance of crossover
				double p=rnd_generate(0.0,100.0);
				if(p < prob) {		// chance to skip crossover
					if(verbosity>3)
					{
						printf("CROSSOVER:\n");
						printf("-");
						print_genome(m_Population, index_mate1);
						printf("-");
						print_genome(m_Population, index_mate2);
					}

					cross(m_Population[index_mate1],m_Population[index_mate2], (int)rnd_generate(1.0,m_Population[0].size()));

					xoverIndices.insert(index_mate1);
					xoverIndices.insert(index_mate2);

					if(verbosity>3)
					{
						printf("+");
						print_genome(m_Population, index_mate1);
						printf("+");
						print_genome(m_Population, index_mate2);
					}
				}
			}

			// Move the cross-over stage population as the current population
			if(verbosity>3)
			{
				printf("--------------------------------------------------------\n");
				printf("Population after cross-over:\n");
				print_population(m_Population);
				printf("--------------------------------------------------------\n");
			}

			for(std::set<int>::iterator it = xoverIndices.begin(); it != xoverIndices.end(); ++it)
			{
				int index = *it;
				m_Population[index].var(v);	// store the Xover operated genome in template
				Distributor::instance().remove_key(index);	// remove previously requested processing

				// Set-up workitem for Xover'd genome job
				WorkItem *w=var_to_workitem(v);
				w->key=index;
				Distributor::instance().push(w);
			}
			// genomes weight-selected into population that did not undergo Xover do not need to be re-worked for fitness

		}

		// MUTATION
		if(m_mutatePartition)
		{
			// Mutate the population
			for(int i = 0; i < m_Population.size(); i++)
			{
				// Genetic operator feedback
				// Output genomes pre-mutation
				if(verbosity>3)
				{
					printf("MUTATION:\n");
					printf("-");
					print_genome(m_Population, i);
				}

				// mutate the whole chromosome if genome is invalid. Else mutate allele based on mutation probability
				mutate(std::wstring(),m_Population[i],!(m_Population[i].valid()));

				// Output genomes post-mutation
				if(verbosity>3)
				{
					printf("+");
					print_genome(m_Population,i);
					printf("---------------------------------------\n");
				}

				m_Population[i].var(v);
				Distributor::instance().remove_key(i); //remove previously requested processing
				WorkItem *w=var_to_workitem(v);
				m_Population[i].set(v);		// TODO may not be necessary
				w->key=i;
				Distributor::instance().push(w);
			}

			// Print population after mutation
			if(verbosity>2)
			{
				printf("--------------------------------------------------------\n");
				printf("Mutation:\n");
				print_population(m_Population);
				printf("--------------------------------------------------------\n");
			}
		}

		// Distribute the fitness evaluation for this generation
		Distributor::instance().process(observer,this);
		// Sort the population
		std::sort(m_Population.begin(),m_Population.end(),reverse_compare);

		if(m_Population.size()>m_MaxPopulation)
		{
			//Cull it
			m_Population.erase(m_Population.begin()+m_MaxPopulation,m_Population.end());
		}

		// update best fitness
		if(!m_bBestFitnessAssigned || m_bestFitness>m_Population[0].fitness())
		{
			m_bestFitness=m_Population[0].fitness();
			m_Population[0].var(m_bestVariables);
			m_bBestFitnessAssigned=true;
		}
		print_stage(g);
	}
}

template<class COMP>
void GAEngine<COMP>::print_config(const int gener)
{
	printf("Genetic Algorithm:\n");
	printf("Generations=%d  Population=%d  MutationRate=%lf  CrossoverRate=%lf \n",gener,m_MaxPopulation,m_MutationProbability,m_CrossProbability);
		
	// Allele list	
	for(int i=0;i<m_AlleleList.size();i++)
	{
		printf("* %s: [%.8e,%.8e]\n",convert(m_AlleleList[i]).c_str(),m_Limits[m_AlleleList[i]].first,m_Limits[m_AlleleList[i]].second);
	}
}

template<class COMP>
void GAEngine<COMP>::print_genome(POPULATION& population, int ind_genome)
{
	VariablesHolder v;
	population[ind_genome].var(v);	// store alleles data in a temporary variable
	printf("[%d] ", ind_genome);		// print the genome's index

	for(int i=0;;i++)
	{
		std::wstring name=v.name(i);
		// sequence all alleles in genome
		if(name.empty())
			break;
		printf("%s=%.8e   ",convert(name).c_str(),v(name));
	}
	std::cout << std::endl;
}

template<class COMP>
void GAEngine<COMP>::print_population(POPULATION& population)
{
	int popsize=population.size();
	for(int i=0;i<popsize;i++)
	{
		print_genome(population, i);
	}
}

template<class COMP>
void GAEngine<COMP>::print_stage(int g)
{
	//verbose summary of GA: print all chromosomes of curr gen
	if(verbosity>1)
	{
		printf("--------------------------------------------------------\n");
		std::cout << currentDateTime() << std::endl;	// tag generation output with timestamp

		for(int j=0;j<m_Population.size();j++)
		{
			//print validity, generation #, and fitness of each chromosome
			printf("%s[%d](%lf) ",(m_Population[j].valid()?(m_Population[j].fitness()<0?"!":" "):"*"),g,m_Population[j].fitness());

			// Sequence the chromosome
			print_genome(m_Population,j);
		}
		printf("--------------------------------------------------------\n");
	}

	//shorter summary of GA: print currently fittest chromosome
	// and fittest chromosome of this generation
	else if(verbosity==1)
	{
		std::cout << currentDateTime() << std::endl;	// tag generation output with timestamp

		//VariablesHolder v;
		double f;

		// Fittest chromosome in this gen is the first genome in sorted population
		f=m_Population[0].fitness();

		printf("Generation %d. Best fitness: %lf\n",g,f);
		print_genome(m_Population, 0);
		printf("--------------------------------------------------------\n");
	}
}

template<class COMP>
void GAEngine<COMP>::mutate(const std::wstring& name,Genome& g,bool mutate_all)
{
	// mutate everything or mutate alleles based on mutation probability
	double prob=(mutate_all?101.0:m_MutationProbability * 100);

	for(int i=0;i<g.size();i++)
	{
		double p=rnd_generate(0.0,100.0);

		// mutate if p <= prob
		if(p>prob) {		// chance to skip mutation
			continue;
		}
		if(!name.size() || g.name(i)==name)
		{
			LIMITS::iterator it=m_Limits.find(g.name(i));	// check for param limits of this allele
			double val;

			if(it==m_Limits.end())
			{
				// no limits, just use [0,RAND_MAX] as a limit
				RandomValueGeneratorBoxConcept generator(0,RAND_MAX);
				val = generator.getRandomValue();
			}
			else
			{
				// restrict RNG to set limits.
				RandomValueGeneratorBoxConcept generator(it->second.first,it->second.second);
				val = generator.getRandomValue();
			}

			g.allele(i,val);	// set the RNG value to allele

			if(name.size())		// only mutate this allele if name specified
				break;
		}
	}
}

template<class COMP>
bool GAEngine<COMP>::cross(Genome& one,Genome& two, int crosspoint)
{
	Genome n1,n2;

	// check if genomes' alleles have same size and crosspoint lies in valid range
	if(one.size()!=two.size() || one.size()<crosspoint+1)
		return false;
	// genomes equal size and crosspoint valid

	// swap alleles before the crosspoint
	for(int i=0;i<crosspoint;i++)
	{
		n1[i]=two[i];
		n2[i]=one[i];
	}

	for(int i=crosspoint;i<one.size();i++)
	{
		n2[i]=two[i];
		n1[i]=one[i];
	}

	one = n1;
	two = n2;

	return true;
}

template<class COMP>
void GAEngine<COMP>::build_rnd_sample(std::vector<int>& sample,int count,bool reject_duplicates,bool check_valid)
{
	double limit=(double)m_Population.size()-0.5;

	for(;count>0;count--)
	{
		int v;

		// randomly assign an int to v 
		// if reject_duplicates set true, add a unique index to sample
		// if check_valid set true, add a valid index
		do
		{
			// nested do-while until genome is both valid and unique
			do
			{
				v=(int)(rnd_generate(0.0,limit));
			} while(check_valid && !m_Population[v].valid());
		} while(reject_duplicates && std::find(sample.begin(),sample.end(),v)!=sample.end());
		//Found next genome
		sample.push_back(v);
	}
}

template<class COMP>
void GAEngine<COMP>::build_rnd_sample_tournament(std::vector<int>& sample,int count,bool reject_duplicates,bool check_valid)
{
	double limit=(double)m_Population.size()-0.5;
	count*=2; //create tournament pairs
	int index=sample.size();

	for(;count>0;count--)
	{
		int v;

		do
		{
			v=(int)(rnd_generate(0.0,limit));
			if(check_valid && !m_Population[v].valid())
				continue;
		} while(reject_duplicates && std::find(sample.begin(),sample.end(),v)!=sample.end());
		//Found next value
		sample.push_back(v);
	}
	//let the fight begins!
	for(int i=index;i<sample.size();i++)
	{
		Genome& one=m_Population[sample[i]];
		Genome& two=m_Population[sample[i+1]];

		if(one>two)
			sample.erase(sample.begin()+i);
		else
			sample.erase(sample.begin()+i+1);
	}
}

template<class COMP>
void GAEngine<COMP>::build_rnd_sample_rnd(std::vector<int>& sample,double prob,bool check_valid)
{
	// if check_valid, only appends indices of m_Population for which Genomes are valid, at given probability (%)
	// else (check_valid==false), appends Genomes at given rate (%)
	for(int i=0;i<m_Population.size();i++)
	{
		if((!check_valid || m_Population[i].valid()) && prob>=rnd_generate(0.0,100.0))
			sample.push_back(i);
	}
}

template<class COMP>
int GAEngine<COMP>::select_weighted(POPULATION& p)
{
	double sum=0.0;
	double zero_lim=0.000000000001;

	// total population fitness
	for(int i=0;i<p.size();i++)
	{
		sum+=(p[i].valid()?1.0/(p[i].fitness()?p[i].fitness():zero_lim):0.0);	//TODO sum can overflow?
	}
	// use a randomly selected threshold for a cumulative-sum selection
	double choice=sum*rnd_generate(0.0,1.0);
	for(int i=0;i<p.size();i++)
	{
		choice-=1.0/(p[i].fitness()?p[i].fitness():zero_lim);
		if(choice<=0.0)
			return i;
	}
	// choice is larger than total sum
	return p.size()-1;	// return the index to last (least-fit) genome
}
