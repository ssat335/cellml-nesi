#include <mpi.h>
#include <clocale>
#include <locale>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <cmath>
#include "Utils.h"

using namespace std;


std::string convert(const std::wstring& wstr)
{
    std::locale const loc("");			// construct a locale obj set to the env's default
    wchar_t const *from=wstr.c_str();	// ptr to const wide character argument
    std::size_t len=wstr.size();		// number of characters in the wide string argument
    std::vector<char> buffer(len+1);	// init char vector buffer of sufficient length to store the input wstring

	// fill buffer with wchar_t->char transformed wstr, using '_' as default character
    std::use_facet<std::ctype<wchar_t> >(loc).narrow(from,from+len,'_',&buffer[0]);

    return std::string(&buffer[0],&buffer[len]);	// return the translated string
}


std::wstring convert(const std::string& str)
{
    std::wstring wstr(str.size(), L' ');	// initialise a wide string filled with wchar ' ', length equal to input string
    wstr.resize(mbstowcs(&wstr[0], str.c_str(), str.size()));	// convert input string to a w-char str pointed by wstr and resize to the number of chars translated

    return wstr;	// return the translated wstring
}

double getExponentMantissa(double val, int* exp)
{
   *exp = (val == 0) ? 0 : (int)(1 + std::log10(std::fabs(val) ) );
   return val * pow((double)10 , -(*exp));
}

double roundDigitPrecision (double mantissa) {
	int val = mantissa * DIGIT_PRECISION; // int will round of the decimal values
	return (double) val/DIGIT_PRECISION; // typecast back to double
}

bool isAlleleRange(double val) {

	if (val < ALLELE_MIN || val > ALLELE_MAX) {
		return false;
	}
	return true;
}

double rnd_generate(double min, double max)
{
	// check if either min or max are within the range
	if (!isAlleleRange(min)) {
		min = ALLELE_MIN;
	}

	if (!isAlleleRange(max)) {
		max = ALLELE_MAX;
	}

	double r = (double)rand() / (double)RAND_MAX;
    int exp = 0;
    double mantissa = getExponentMantissa((min + r * (max - min)), &exp);
    return roundDigitPrecision(mantissa) * pow(10, exp);
}

double rnd_logarithmic_generate(double min, double max)
{
	if (!isAlleleRange(min)) {
		min = ALLELE_MIN;
	}

	if (!isAlleleRange(max)) {
		max = ALLELE_MAX;
	}

	int exp_min = 0;
	int exp_max = 0;

	double mantissa_min = getExponentMantissa(min, &exp_min);
	double mantissa_max = getExponentMantissa(max, &exp_max);

	// Randomise exp and mantissa.
    double r = (double)rand() / (double)RAND_MAX;
    double exp_rnd =  exp_min + r * (exp_max - exp_min);
    double mantissa_rnd = mantissa_min + r * (mantissa_max - mantissa_min);

    // return the randomised value
	return roundDigitPrecision(mantissa_rnd) * pow(10, exp_rnd);
}

const std::string currentDateTime()
{
	time_t now=time(NULL);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

	return buf;
}

int getProcessRank() {
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	return rank;
}
