/*
 * VariablesHolder.cpp
 *
 *  Created on: 16/10/2015
 *      Author: ssat335
 */


#include "VariablesHolder.h"

#include "Utils.h"
#include <stdio.h>
#include <algorithm>

using namespace std;

VariablesHolder::VariablesHolder()
{
}

VariablesHolder::VariablesHolder(const VariablesHolder& other)
{
	m_Vars.assign(other.m_Vars.begin(),other.m_Vars.end());
}

VariablesHolder::~VariablesHolder()
{
}

VariablesHolder& VariablesHolder::operator=(const VariablesHolder& other)
{
	if(&other!=this)
		m_Vars.assign(other.m_Vars.begin(),other.m_Vars.end());
	return *this;
}

double VariablesHolder::operator()(const std::wstring& name)
{
	// get iterator to the pair in m_Vars for which the first member equals name (end if no such pair)
	ALLELE::iterator it=find_if(m_Vars.begin(),m_Vars.end(),
								bind1st(pair_equal_to<std::wstring,double>(),name));
	return (it==m_Vars.end()?double(0.0):it->second);
}

double VariablesHolder::operator()(const std::wstring& name,double val)
{
	// find if matching allele already exists in VarHold
	ALLELE::iterator it=find_if(m_Vars.begin(),m_Vars.end(),
								bind1st(pair_equal_to<std::wstring,double>(),name));

	if(it!=m_Vars.end())
		it->second=val;	// update the existing allele
	else
		m_Vars.push_back(std::make_pair<std::wstring,double>(std::wstring(name),double(val)));	// add the allele

	return val;
}

std::wstring VariablesHolder::name(int index)
{
	return (index>=0 && index<m_Vars.size())?m_Vars[index].first:std::wstring();
}

bool VariablesHolder::exists(const std::wstring& name)
{
	ALLELE::iterator it=find_if(m_Vars.begin(),m_Vars.end(),
								bind1st(pair_equal_to<std::wstring,double>(),name));
	return (it!=m_Vars.end());
}

size_t VariablesHolder::size()
{
	// is the size of m_Vars vector i.e. number of alleles in stored in the m_Vars vector
	return m_Vars.size();
}

void VariablesHolder::collate(std::vector<double>& v)
{
	for(ALLELE::iterator it=m_Vars.begin();it!=m_Vars.end();++it)
	{
		v.push_back(it->second);
	}
}

void VariablesHolder::print(FILE *pfout)
{
	for(ALLELE::iterator it=m_Vars.begin();it!=m_Vars.end();++it)
	{
		fprintf(pfout,"%s=%.5e  ",convert(it->first).c_str(),it->second);
	}
	fprintf(pfout,"\n");
}

bool VariablesHolder::fillup(std::vector<double>& v)
{
	// check if the sizes are equal
	if(v.size()!=m_Vars.size())
		return false;

	// assign allele values
	for(int i=0;i<v.size();i++)
	{
		m_Vars[i].second=v[i];
	}
	return true;
}

