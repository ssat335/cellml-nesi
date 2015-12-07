/*
 * RandomValueGeneratorBoxConcept.cpp
 *
 *  Created on: 23/11/2015
 *      Author: ssat335
 */

#include "RandomValueGeneratorBoxConcept.h"

#define MAX_LIMIT_EXP_BLOCK 9000
#define MAX_BLOCK_VAL 9999
#define MIN_BLOCK_VAL 1000

#include <assert.h>
#include <math.h>
#include <stdlib.h>

RandomValueGeneratorBoxConcept::RandomValueGeneratorBoxConcept(double min, double max) {
	assert(min < max && "Minimum value is not smaller than maximum value");
	m_exp_max = 0;
	m_exp_min = 0;
	m_sf_min = getExponentMantissa(min, &m_exp_min);
	m_sf_max = getExponentMantissa(max, &m_exp_max);
	assert((m_exp_max <= 12 && m_exp_max >= -13) && "Maximum exponent is not in bound");
	assert((m_exp_min <= 12 && m_exp_min >= -13) && "Minimum value exponent is not in bound");
}

int RandomValueGeneratorBoxConcept::getExponentMantissa(double val, int* exp)
{
	*exp = 0;
	// For 0, we assume the exp_block is -13 and value to be 9999
	if (val < 1e-12){
		*exp = -13;
		return 9999;
	}

	// Calculate the mantissa and exponent.
	double temp_exp = 0;
	double temp_val = val;
	while((signed long long) (temp_val) < 10000 && temp_exp >= -15) {
		temp_val = temp_val * 10;
		temp_exp--;
	}
	temp_val = round(temp_val);

	do {
		temp_val = temp_val / 10;
		temp_exp++;
	} while ((signed long long)temp_val >= 10000 && temp_exp <= 9);

	/* Actual mantissa value is kept as m_sf_XXXX * 1000, and
	 * therefore we have to factor exponential value appropriately*/
	*exp = temp_exp + 3;
	return (signed long long) temp_val;
}

double RandomValueGeneratorBoxConcept::getRandomValue() {
	/*
	 * Here, we assume that the exponent blocks range from -12 to 12, and
	 * each exponent has 9000 blocks. In the case of 0, we assume it to be exp -13
	 * and actual value to be 9999, to make the calculations easier.
	 *  -13      -12		  -11 		..........................	10		 11			12
	 * [9999][1000...9999][1000...9999].......................[1000...9999][1000...9999][1000...9999]
	 */

	/* Evaluate the total number of blocks between the maximum and minimum.*/

	// number of blocks from the block in minimum exp until the end block in the exp block
	int number_of_blocks_in_min_exp = MAX_LIMIT_EXP_BLOCK - m_sf_min + MIN_BLOCK_VAL; // both ends inclusive
	// number_of_blocks_in_min_exp is added with the 9999 * number of exp blocks in between + blocks in the max exp block.
	int total_blocks_between_vals_ends_inclusive = number_of_blocks_in_min_exp + // both ends inclusive
									MAX_LIMIT_EXP_BLOCK * (fabs(m_exp_max - m_exp_min) - 1) + //excluding both end blocks
									m_sf_max + 1 - MIN_BLOCK_VAL; // both the end blocks are inclusive

	int random_block = rand()%(total_blocks_between_vals_ends_inclusive) + 1; // can be anything from 1 (minimum val) to number_of_blocks (max val)

	// if the min value is 0.0 and random_block is 1, it means the selection is the 0 value.
	if (m_exp_min == -13 && m_sf_min == MAX_BLOCK_VAL) {
		if (random_block == 1)
			return 0.0;
	}

	/*Calculate the blocks from min to random block*/

	int exp_blocks = m_exp_min + fabs((m_sf_min - MIN_BLOCK_VAL  + random_block - 1) / MAX_LIMIT_EXP_BLOCK);
	int block_in_exp = fabs((m_sf_min - MIN_BLOCK_VAL + random_block - 1) % MAX_LIMIT_EXP_BLOCK)  + MIN_BLOCK_VAL;

	// The following is a straight forward code, iterating through each block. Does the same thing as the
	// previous two lines, but it will take longer to execute. Useful to get the exact
	// block and exp_block while debugging.

	/*	int exp_blocks = m_exp_min;
	int block_in_exp = m_sf_min;
	for(int i = 0; i < random_block - 1; i++) {
		block_in_exp++;
		if(block_in_exp == MAX_BLOCK_VAL + 1) {
			exp_blocks++;
			block_in_exp = MIN_BLOCK_VAL;
		}
	}*/

	assert((exp_blocks <= 12 && exp_blocks >= -12) && "Exponent is not in bound");
	assert((block_in_exp < 10000 && block_in_exp >= 1000) && "Block in an exp block has been incorrectly evaluated");
	return (double(block_in_exp)/1000) * pow((double)10 , exp_blocks);
}

RandomValueGeneratorBoxConcept::~RandomValueGeneratorBoxConcept() {
	// TODO Auto-generated destructor stub
}


