/*
 * RandomValueGeneratorBoxConcept.h
 *
 *  Created on: 23/11/2015
 *      Author: ssat335
 */

#ifndef RANDOMVALUEGENERATORBOXCONCEPT_H_
#define RANDOMVALUEGENERATORBOXCONCEPT_H_


class RandomValueGeneratorBoxConcept {
private:
	int m_sf_min;
	int m_exp_min;

	int m_sf_max;
	int m_exp_max;

private:
	int getExponentMantissa(double val, int* exp);

public:
	RandomValueGeneratorBoxConcept(double min, double max);
	double getRandomValue();
	virtual ~RandomValueGeneratorBoxConcept();
};

#endif /* RANDOMVALUEGENERATORBOXCONCEPT_H_ */
