// Chi_Squared_1st.h

#ifndef CHI_SQUARED_CONTINUOUS_H
#define CHI_SQUARED_CONTINUOUS_H

#include "../Quantiles/GK.h"
#include "../Quantiles/ReservoirSampling.h"
//#include "../Quantiles/QDigestDouble.h"
//#include "../Quantiles/CMSQuantile.h"
#include <iostream>
#include <stdlib.h> 
#include<math.h>


class ChiSquare
{
public:
	ChiSquare();
	ChiSquare(double m,int q=1);
	/* q=1: GreenWaldKhanna Sketch
	   q=2: Q-Digest Sketch
	   q=3: Count Min Sketch
	   q=4 Reservoir Sampling
	*/
	
	~ChiSquare();
  	
	void insert(double val);
	double calculate_statistic_ifNormal(int num_buckets, double mean, double SD);
	double calculate_statistic(int num_buckets, double (*f)(double));
	double* GetUpper();
	double* GetLower();
	
private:
	double chi_squared;
	double memory;
	double *UpperBins;
	double *LowerBins;
	
	QuantileSketch* quantile_sketch;
	
	double RationalApproximation(double t);
	double NormalCDFInverse(double p, double mean, double SD);
	
};

#include"ChiSquareContinuous.cpp"
#endif
