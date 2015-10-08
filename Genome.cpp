/*
 * Genome.cpp
 *
 *  Created on: 8/10/2015
 *      Author: ssat335
 */


#include "Genome.h"


Genome::Genome():m_Fitness(0.0),m_Valid(true)
{
}

Genome::Genome(const Genome& other):m_Fitness(other.m_Fitness),m_Valid(other.m_Valid)
{
	m_Alleles.assign(other.m_Alleles.begin(),other.m_Alleles.end());
}

Genome::~Genome()
{
}

Genome Genome::operator=(const Genome& other)
{
	if(&other!=this)
	{
		m_Alleles.assign(other.m_Alleles.begin(),other.m_Alleles.end());
		m_Fitness=other.m_Fitness;
		m_Valid=other.m_Valid;
	}
	return *this;
}

double Genome::allele(const std::wstring& name)
{
	ALLELE::iterator it=find_if(m_Alleles.begin(),m_Alleles.end(),
								bind1st(pair_equal_to<std::wstring,double>(),name));
	return (it==m_Alleles.end()?double(0.0):it->second);
}

double Genome::allele(int index)
{
	return ((index>=0 && index<m_Alleles.size())?m_Alleles[index].second:0.0);
}

void Genome::allele(const std::wstring& name,double val)
{
	ALLELE::iterator it=find_if(m_Alleles.begin(),m_Alleles.end(),
								bind1st(pair_equal_to<std::wstring,double>(),name));
	if(it!=m_Alleles.end())
		it->second=val;
	else
		m_Alleles.push_back(std::make_pair<std::wstring,double>(std::wstring(name),double(val)));
}

void Genome::allele(int index,double val)
{
	if(index>=0 && index<m_Alleles.size())
		m_Alleles[index].second=val;
}

std::wstring Genome::name(int index)
{
	return ((index>=0 && index<m_Alleles.size())?m_Alleles[index].first:std::wstring());
}

std::pair<std::wstring,double>& Genome::operator[](int index)
{
	while(m_Alleles.size()<=index)
		m_Alleles.push_back(std::make_pair(std::wstring(),double(0.0)));
	return m_Alleles[index];
}

bool Genome::operator<(const Genome& other) const
{
	if(valid() && other.valid())
		return fitness()<other.fitness();
	return valid();
}

bool Genome::operator>(const Genome& other) const
{
    if(valid() && other.valid())
        return fitness()>other.fitness();
    return valid();
}

bool Genome::operator==(const Genome& other) const
{
    if(valid() && other.valid())
        return fitness()==other.fitness();
    return false;
}

bool Genome::same(const Genome& other) const
{
	if(!(valid() && other.valid()))
		return false;

	if(other.m_Alleles.size()!=m_Alleles.size())
        return false;
    for(int i=0;i<m_Alleles.size();i++)
    {
        if(m_Alleles[i].first!=other.m_Alleles[i].first ||
		   m_Alleles[i].second!=other.m_Alleles[i].second)
            return false;
    }
    return true;
}

void Genome::var(VariablesHolder& v)
{
	// iterate through the alleles in this genome
    for(ALLELE::iterator it=m_Alleles.begin();it!=m_Alleles.end();++it)
    {
		// update each allele in v
        v(it->first,it->second);
    }
}

void Genome::set(VariablesHolder& v)
{
	// rebuild alleles from that stored in a varholder
    m_Alleles.clear();	// clear m_Alleles vector

    for(int k=0;;k++)
    {
		std::wstring name=v.name(k);
		if(name.empty())	// k reached the end of m_Var in v
			break;

        m_Alleles.push_back(std::make_pair<std::wstring,double>(name,v(name)));		// append a pair made from m_Vars of v to m_Alleles
    }
}

