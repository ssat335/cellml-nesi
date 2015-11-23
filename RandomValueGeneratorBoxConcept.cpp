/*
 * RandomValueGeneratorBoxConcept.cpp
 *
 *  Created on: 23/11/2015
 *      Author: ssat335
 */

#include "RandomValueGeneratorBoxConcept.h"

#define MAX_LIMIT_EXP_BLOCK 9999

#include <assert.h>
#include <math.h>
#include <stdlib.h>

RandomValueGeneratorBoxConcept::RandomValueGeneratorBoxConcept(double min, double max) {
	assert(min < max && "Minimum value is not smaller than maximum value");
	assert(min >= 0 && "Random minimum limit value cannot be negative");
	assert(max >=0 && "Random maximum limit value cannot be negative");
	m_exp_max = 0;
	m_exp_min = 0;
	m_sf_min = getExponentMantissa(min, &m_exp_min);
	m_sf_max = getExponentMantissa(max, &m_exp_max);
	assert((m_exp_max <= 12 || m_exp_max >= -13) && "Maximum exponent is not in bound");
	assert((m_exp_min <= 12 || m_exp_min >= -13) && "Minimum value exponent is not in bound");
}

int RandomValueGeneratorBoxConcept::getExponentMantissa(double val, int* exp)
{
	*exp = 0;
	// For 0, we assume the exp_block is -13 and value to be 9999
	if (val == 0){
		*exp = -13;
		return 9999;
	}

	// Calculate the mantissa and exponent. NOTE: 1e-5 will be here evaluated as 1000e-8
	double temp_exp = 0;
	double temp_val = val;
	while((signed long long) (temp_val) < 10000) {
		temp_val = temp_val * 10;
		temp_exp--;
	}

	do {
		temp_val = temp_val / 10;
		temp_exp++;
	} while ((signed long long)temp_val >= 10000);

	*exp = temp_exp;
	return (signed long long) temp_val;
}

double RandomValueGeneratorBoxConcept::getRandomValue() {
	/*
	 * Here, we assume that the exponent blocks range from -12 to 12, and
	 * each exponent has 9999 blocks. In the case of 0, we assume it to be exp -13
	 * and actual value to be 9999, to make the calculations easier.
	 *  -13     -12		  -11 		..................		10		 11			12
	 * [9999][1...9999][1...9999].......................[1...9999][1...9999][1...9999]
	 */

	/* Evaluate the total number of blocks between the maximum and minimum.*/

	// number of blocks from the block in minimum exp until the end block in the exp block
	int number_of_blocks_in_min_exp = MAX_LIMIT_EXP_BLOCK - m_sf_min; // both ends inclusive
	// number_of_blocks_in_min_exp is added with the 9999 * number of exp blocks in between + blocks in the max exp block.
	int total_blocks_between_vals_ends_inclusive = number_of_blocks_in_min_exp + // both ends inclusive
									MAX_LIMIT_EXP_BLOCK * (fabs(m_exp_max - m_exp_min) - 1) + //excluding both end blocks
									m_sf_max + 1; // both the end blocks are inclusive

	int random_block = rand()%(total_blocks_between_vals_ends_inclusive) + 1; // can be anything from 1 (minimum val) to number_of_blocks (max val)

	// if the min value is 0.0 and random_block is 1, it means the selection is the 0 value.
	if (m_exp_min == -13 && m_sf_min == 9999) {
		if (random_block == 1)
			return 0.0;
	}

	/*Calculate the blocks from min to random block*/

	int exp_blocks = fabs((m_sf_min + random_block - 1) / MAX_LIMIT_EXP_BLOCK);
	int block_in_exp = fabs((m_sf_min + random_block - 1) % MAX_LIMIT_EXP_BLOCK);

	if (!block_in_exp)
	{
		// this means the block is not defined, and the end of last block should be the exp block
		block_in_exp = 9999;
		exp_blocks--;
	}

	exp_blocks += m_exp_min;
	assert((exp_blocks <= 12 || exp_blocks >= -12) && "Exponent is not in bound");
	assert(block_in_exp < 10000 && "Block in an exp block has been incorrectly evaluated");
	return block_in_exp * pow((double)10 , exp_blocks);
}

RandomValueGeneratorBoxConcept::~RandomValueGeneratorBoxConcept() {
	// TODO Auto-generated destructor stub
}


