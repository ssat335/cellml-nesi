/*
 * VariablesHolder.h
 *
 *  Created on: 16/10/2015
 *      Author: ssat335
 */

#ifndef VARIABLESHOLDER_H_
#define VARIABLESHOLDER_H_

#include <vector>
#include <string>

// define ALLELE as vector of <wstring, double> pairs ( i.e. ALLELE is a vector of 'allele's (pair<wstr,doub>) )
typedef std::vector<std::pair<std::wstring,double> > ALLELE;

class VariablesHolder
{
	private:
		ALLELE m_Vars;	//the member that stores alleles in a chromosome form ( i.e. vector <allele=pair<wstring allele_name,double allele_value> > )

	public:
		VariablesHolder();
		VariablesHolder(const VariablesHolder& other);	//copy other VarHolder's variables into this one
		~VariablesHolder();

		VariablesHolder& operator=(const VariablesHolder& other);

		// indexing an allele's value stored by its name
			// 0.0 returned if an allele by the supplied name does not exist
		double operator()(const std::wstring& name);

		// Update an allele in m_Var member and return updated allele value
		double operator()(const std::wstring& name,double val);

		// Search allele by index and return its name, if any
		std::wstring name(int index);

		// Existence of allele of given name
		bool exists(const std::wstring& name);

		// Size of a VariablesHolder object (number of alleles stored)
		size_t size();

		// Collate the genome sequence into a vector
		void collate(std::vector<double>& v);

		// print all alleles held in m_Vars to file
		void print(FILE *pfout);

		// Fill-up the chromosome with a supplied vector of allele values. Return true iff executed correctly
		bool fillup(std::vector<double>& v);
};



#endif /* VARIABLESHOLDER_H_ */
