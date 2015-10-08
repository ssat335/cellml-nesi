/*
 * Genome.h
 *
 *  Created on: 8/10/2015
 *      Author: ssat335
 */

#ifndef GENOME_H
#define GENOME_H

#include <vector>
#include <math.h>
#include "virtexp.h"

//Genome handler
class Genome
{
    private:
        ALLELE m_Alleles;
        double m_Fitness;
        bool m_Valid;

    public:
        Genome();
        Genome(const Genome& other);
        ~Genome();

		// assignment
        Genome operator=(const Genome& other);

		// Get an allele value from allele name (0.0 if not found)
        double allele(const std::wstring& name);

		// Get an allele value from indexing the genome (0.0 if out of range)
        double allele(int index);

		// Update allele in genome by name and value
        void allele(const std::wstring& name,double val);

		// Update the value of ith allele in genome
        void allele(int index,double val);

		// Get validity of genome
        bool valid() const { return m_Valid; }
        void valid(bool b) { m_Valid=b; }		// assign m_Valid member to b

		// Get the name of ith allele in genome (empty if oor)
        std::wstring name(int index);

		// Get the size of genome
        int size() { return m_Alleles.size(); }

		// Get the allele by indexing the genome sequence
        std::pair<std::wstring,double>& operator[](int index);

		// Get the fitness value
        double fitness() const { return m_Fitness; }
        void fitness(double v) { m_Fitness=v; m_Valid=(v!=INFINITY); }	// an INF fit genome is invalid

		// Compare genomes by fitness
		bool operator<(const Genome& other) const;
        bool operator>(const Genome& other) const;
        bool operator==(const Genome& other) const;

		// Test if two genomes are identical
        bool same(const Genome& other) const;

		// Store the genetic data in a temporary storage
        void var(VariablesHolder& v);

		// Rebuild a genome from a temporary sequence
        void set(VariablesHolder& v);
};

#endif /* GENOME_H */
