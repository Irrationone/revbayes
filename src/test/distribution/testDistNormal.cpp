#include <iostream>
#include <cmath>
#include <assert.h>

#include "Argument.h"
#include "ConstantNode.h"
#include "DAGNode.h"
#include "RbDouble.h"
#include "RbFunction_dnorm.h"
#include "RbFunction_pnorm.h"
#include "RbFunction_rnorm.h"
#include "RbFunction_qnorm.h"
#include "RbObject.h"
#include "RbException.h"

int main(int argc, char **argv) {
	// create a DistNormal object
	RbDouble* sigma = new RbDouble(1.0);
	RbDouble* mu = new RbDouble(0.0);
	RbDouble* x = new RbDouble(0.5);
	
	RbFunction_dnorm   dnorm;
	RbFunction_pnorm   pnorm;
	RbFunction_qnorm   qnorm;
	RbFunction_rnorm   rnorm;	
	
	ConstantNode* cnSigma = new ConstantNode(sigma);
	ConstantNode* cnMu = new ConstantNode(mu);
	ConstantNode* cnX = new ConstantNode(x);
	
	Argument* aMu    = new Argument("mu", cnMu);
	Argument* aSigma = new Argument("sigma", cnSigma);
	Argument* aX     = new Argument("", cnX);
	
	std::vector<Argument*> args;
	args.push_back( aX );
	args.push_back( aSigma );
	args.push_back( aMu );

	// TODO some more meaningful test

	// calculate the pdf for some known parameters
	
	try 
	{
	
		double pdf = ((RbDouble*) dnorm.execute(args))->getValue();
		double expectedPDF = 0.3520653; //taken from R
		std::cout << "The pdf is " << pdf << '\n';
		std::cout << "The expected pdf is " << expectedPDF << '\n';
		assert(abs(pdf-expectedPDF) <= 0.0000001);

	// calculate the lnpdf for some known parameters
//	double lnpdf = (double) dnorm.execute(args);
//	double expectedLnPDF = -1.043939; //taken from R
//	std::cout << "The lnpdf is " << lnpdf << '\n';
//	std::cout << "The expected lnpdf is " << expectedLnPDF << '\n';
//	assert(abs(lnpdf-expectedLnPDF) <= 0.000001);

		// calculate the cdf for some known parameters
		double cdf = ((RbDouble*) pnorm.execute(args))->getValue();
		double expectedCDF = 0.6914625; //taken from R
		std::cout << "The cdf is " << cdf << '\n';
		std::cout << "The expected cdf is " << expectedCDF << '\n';
		assert(abs(cdf-expectedCDF) <= 0.0000001);

		// calculate the quantile for some known parameters
		double q = ((RbDouble*) qnorm.execute(args))->getValue();
		double expectedQ = 0.0; //taken from R
		std::cout << "The quantile is " << q << '\n';
		std::cout << "The expected quantile is " << expectedQ << '\n';
		assert(abs(q-expectedQ) <= 0.0000001);
	}
	catch (RbException e)
	{
		std::cout << e.getMessage() << std::endl;
	}
	// draw some random values some known parameters and compare this to the pdf and cdf.

	assert(true);

}
