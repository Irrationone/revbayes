//
//  RateMatrix_Pomo.cpp
//
//  Created by Bastien Boussau on 4/4/14.
//  Copyright (c) 2014 Bastien Boussau. All rights reserved.
//

#include "RateMatrix_Pomo.h"
#include "MatrixReal.h"
#include "RbException.h"
#include "TransitionProbabilityMatrix.h"

#include <cmath>
#include <iomanip>

using namespace RevBayesCore;

/** Construct rate matrix with n states */
RateMatrix_Pomo::RateMatrix_Pomo(size_t n) : AbstractRateMatrix( n ), N( 10 ), matrixSize( n ){
    std::vector<double> temp (4, 0.0);
    for (size_t i = 0; i<4 ; ++i) {
        mu.push_back(temp);
        s.push_back(1.0);
    }
    updateMatrix();
}

/** Construct rate matrix with n states, virtual population size, mutation rates, selection coefficients */
RateMatrix_Pomo::RateMatrix_Pomo(size_t n, const size_t vps, const std::vector<double> &mr, const std::vector<double> &sc) : AbstractRateMatrix( n ), N( vps ), matrixSize( n ){
    std::vector<double> temp (4, 0.0);
    for (size_t i = 0; i<4 ; ++i) {
        mu.push_back(temp);
        s.push_back(1.0);
    }
    setMutationRates(mr);
    setSelectionCoefficients(sc);
    updateMatrix();
}

/** Construct rate matrix with n states, a matrix of mutation rates, and a vector of selection coefficients */
RateMatrix_Pomo::RateMatrix_Pomo(size_t n,  const size_t vps, const RateMatrix &mm, const std::vector<double> sc)  : AbstractRateMatrix( n ), N( vps ), matrixSize( n ){
    std::vector<double> temp (4, 0.0);
    for (size_t i = 0; i<4 ; ++i) {
        mu.push_back(temp);
        s.push_back(1.0);
    }
    setMutationRates(mm);
    setSelectionCoefficients(sc);
    updateMatrix();
}


/** Copy constructor */
RateMatrix_Pomo::RateMatrix_Pomo(const RateMatrix_Pomo& m) : AbstractRateMatrix( m ), N(m.N), matrixSize(m.matrixSize), mu(m.mu), s(m.s), precision(m.precision), stationaryFreqs(m.stationaryFreqs) {

}


/** Destructor */
RateMatrix_Pomo::~RateMatrix_Pomo(void) {
    
}


RateMatrix_Pomo& RateMatrix_Pomo::operator=(const RateMatrix_Pomo &r) {
    
    if (this != &r)
    {
        RateMatrix::operator=( r );
    }
    
    return *this;
}

double RateMatrix_Pomo::averageRate(void) const
{
    return 1.0;
}

void RateMatrix_Pomo::buildRateMatrix(void) 
{
    
    // compute auxilliary variables
    double N2 = (double) (N*N);
    int Nminus1 = (int)N-1;
    double Nminus1d = (double) Nminus1;
    for (size_t i = 0 ; i < 4; i++) 
    {
        mu[i][i] = 0.0;
    }
    
    // calculate the transition probabilities
    for (size_t i=0; i< matrixSize; i++)
    {
    //The first 4 states are the monomorphic states; we can't directly change from one into another one
        for (size_t j=0; j< matrixSize; j++) 
        {
        (*theRateMatrix)[i][j] = 0.0;
        }
    }
    
    //Change from a monomorphic into a polymorphic state
    //(i.e. the first 4 lines in the matrix )
    //The 4..4+Nminus1 states are the AC matrix
    //Only 2 entries can differ from 0, (N-1)A and (N-1)C
    //(N-1)A can only come from monomorphic state A, i.e. i=0
    //(N-1)A is at the end of the submatrix, j=4+N-1
    (*theRateMatrix)[0][4 + Nminus1 - 1] = N2 * mu[0][1];
    //(N-1)C can only come from monomorphic state C, i.e. i=1
    //(N-1)C is at the begining of the submatrix, j=4+N
    (*theRateMatrix)[1][4] = N2 * mu[1][0];
    
    //The 4+Nminus1..4+2Nminus1 states are the AG matrix
    //Only 2 entries can differ from 0, (N-1)A and (N-1)G
    //(N-1)A can only come from monomorphic state A, i.e. i=0
    //(N-1)A is at the end of the submatrix, j=4+2*Nminus1
    (*theRateMatrix)[0][4 + 2*Nminus1 -1] = N2 * mu[0][2];
    //(N-1)G can only come from monomorphic state G, i.e. i=1
    //(N-1)G is at the begining of the submatrix, j=4+N
    (*theRateMatrix)[2][4 + Nminus1] = N2 * mu[2][0];

    //The 4+2Nminus1..4+3Nminus1 states are the AT matrix
    //Only 2 entries can differ from 0, (N-1)A and (N-1)T
    //(N-1)A can only come from monomorphic state A, i.e. i=0
    //(N-1)A is at the end of the submatrix, j=4+3*Nminus1
    (*theRateMatrix)[0][4 + 3*Nminus1 -1] = N2 * mu[0][3];
    //(N-1)T can only come from monomorphic state T, i.e. i=1
    //(N-1)T is at the begining of the submatrix, j=4+N
    (*theRateMatrix)[3][4 + 2*Nminus1] = N2 * mu[3][0];

    //The 4+3Nminus1..4+4Nminus1 states are the CG matrix
    //Only 2 entries can differ from 0, (N-1)C and (N-1)G
    //(N-1)C can only come from monomorphic state C, i.e. i=0
    //(N-1)C is at the end of the submatrix, j=4+4*Nminus1
    (*theRateMatrix)[1][4 + 4*Nminus1 -1] = N2 * mu[1][2];
    //(N-1)G can only come from monomorphic state G, i.e. i=1
    //(N-1)G is at the begining of the submatrix, j=4+N
    (*theRateMatrix)[2][4 + 3*Nminus1] = N2 * mu[2][1];

    //The 4+4Nminus1..4+5Nminus1 states are the CT matrix
    //Only 2 entries can differ from 0, (N-1)C and (N-1)T
    //(N-1)C can only come from monomorphic state C, i.e. i=0
    //(N-1)C is at the end of the submatrix, j=4+5*Nminus1
    (*theRateMatrix)[1][4 + 5*Nminus1 -1] = N2 * mu[1][3];
    //(N-1)T can only come from monomorphic state T, i.e. i=1
    //(N-1)T is at the begining of the submatrix, j=4+N
    (*theRateMatrix)[3][4 + 4*Nminus1] = N2 * mu[3][1];

    //The 4+5Nminus1..4+6Nminus1 states are the GT matrix
    //Only 2 entries can differ from 0, (N-1)G and (N-1)T
    //(N-1)G can only come from monomorphic state G, i.e. i=0
    //(N-1)G is at the end of the submatrix, j=4+5*Nminus1
    (*theRateMatrix)[2][4 + 6*Nminus1 -1] = N2 * mu[2][3];
    //(N-1)T can only come from monomorphic state T, i.e. i=1
    //(N-1)T is at the begining of the submatrix, j=4+N
    (*theRateMatrix)[3][4 + 5*Nminus1] = N2 * mu[3][2];


    //Now we move from a polymorphic state to a monomorphic state
    //(i.e. the first four columns in the matrix)
    //The [4..4+Nminus1[ states are the AC matrix
    //Only 2 entries can differ from 0, (N-1)A going to mono A and (N-1)C going to mono C
    //(N-1)A can only go to monomorphic state A, i.e. j=0
    //(N-1)A is at the end of the submatrix, i=4+N-1
    
    (*theRateMatrix)[4 + Nminus1 - 1][0] = computeEntryFromMoranProcessWithSelection(0, 1, Nminus1d);
//    double temp = (N-1)*(1+s[0]-s[1]);
  //  (*theRateMatrix)[4 + Nminus1 - 1][0] = temp / ( temp + 1) * (1) / N;
    //(N-1)C can only go to monomorphic state C, i.e. j=1
    //(N-1)C is at the begining of the submatrix, i=4
   // temp = (N-1)*(1+s[1]-s[0]);
    (*theRateMatrix)[4 ][1] = computeEntryFromMoranProcessWithSelection(1, 0, Nminus1d);
  //  (*theRateMatrix)[4 ][1] = temp / ( temp + 1) * (1) / N;

    //The 4+Nminus1..4+2Nminus1 states are the AG matrix
    //Only 2 entries can differ from 0, (N-1)A going to mono A and (N-1)G going to mono G
    //(N-1)A can only go to monomorphic state A, i.e. j=0
    //(N-1)A is at the end of the submatrix, i=4+2*(Nminus1)
  //  temp = (N-1)*(1+s[0]-s[2]);
    (*theRateMatrix)[4 + 2*Nminus1 - 1][0] = computeEntryFromMoranProcessWithSelection(0, 2, Nminus1d);
  //  (*theRateMatrix)[4 + 2*Nminus1 - 1][0] = temp / ( temp + 1) * (1) / N;
    //(N-1)G can only go to monomorphic state G, i.e. j=2
    //(N-1)G is at the begining of the submatrix, i=4
  //  temp = (N-1)*(1+s[2]-s[0]);
    (*theRateMatrix)[4 + Nminus1 ][2] = computeEntryFromMoranProcessWithSelection(2, 0, Nminus1d);
   // (*theRateMatrix)[4 + Nminus1 ][2] = temp / ( temp + 1) * (1) / N;

    //The 4+2Nminus1..4+3Nminus1 states are the AT matrix
    //Only 2 entries can differ from 0, (N-1)A going to mono A and (N-1)T going to mono T
    //(N-1)A can only go to monomorphic state A, i.e. j=0
    //(N-1)A is at the end of the submatrix, i=4+3*(Nminus1)
//    temp = (N-1)*(1+s[0]-s[3]);
    (*theRateMatrix)[4 + 3*Nminus1 - 1][0] = computeEntryFromMoranProcessWithSelection(0, 3, Nminus1d);
   // (*theRateMatrix)[4 + 3*Nminus1 - 1][0] = temp / ( temp + 1) * (1) / N;
    //(N-1)T can only go to monomorphic state T, i.e. j=3
    //(N-1)T is at the begining of the submatrix, i=4+ 2*Nminus1
   // temp = (N-1)*(1+s[3]-s[0]);
    (*theRateMatrix)[4 + 2*Nminus1 ][3] = computeEntryFromMoranProcessWithSelection(3, 0, Nminus1d);
   // (*theRateMatrix)[4 + 2*Nminus1 ][3] = temp / ( temp + 1) * (1) / N;

    //The 4+3Nminus1..4+4Nminus1 states are the CG matrix
    //Only 2 entries can differ from 0, (N-1)C going to mono C and (N-1)G going to mono G
    //(N-1)C can only go to monomorphic state C, i.e. j=1
    //(N-1)C is at the end of the submatrix, i=4+4*(Nminus1)
  //  temp = (N-1)*(1+s[1]-s[2]);
    (*theRateMatrix)[4 + 4*Nminus1 - 1][1] = computeEntryFromMoranProcessWithSelection(1, 2, Nminus1d);
//    (*theRateMatrix)[4 + 4*Nminus1 - 1][1] = temp / ( temp + 1) * (1) / N;
    //(N-1)G can only go to monomorphic state G, i.e. j=2
    //(N-1)G is at the begining of the submatrix, i=4
 //   temp = (N-1)*(1+s[2]-s[1]);
     (*theRateMatrix)[4 + 3*Nminus1 ][2] = computeEntryFromMoranProcessWithSelection(2, 1, Nminus1d);
  //  (*theRateMatrix)[4 + 3*Nminus1 ][2] = temp / ( temp + 1) * (1) / N;
    
    //The 4+4Nminus1..4+5Nminus1 states are the CT matrix
    //Only 2 entries can differ from 0, (N-1)C going to mono C and (N-1)T going to mono T
    //(N-1)C can only go to monomorphic state C, i.e. j=1
    //(N-1)C is at the end of the submatrix, i=4+5*(Nminus1)
 //   temp = (N-1)*(1+s[1]-s[3]);
   (*theRateMatrix)[4 + 5*Nminus1 - 1][1] =computeEntryFromMoranProcessWithSelection(1, 3, Nminus1d);
  //  (*theRateMatrix)[4 + 5*Nminus1 - 1][1] = temp / ( temp + 1) * (1) / N;
    //(N-1)T can only go to monomorphic state T, i.e. j=3
    //(N-1)T is at the begining of the submatrix, i=4
 //   temp = (N-1)*(1+s[3]-s[1]);
    (*theRateMatrix)[4 + 4*Nminus1 ][3] =computeEntryFromMoranProcessWithSelection(3, 1, Nminus1d);
  //  (*theRateMatrix)[4 + 4*Nminus1 ][3] = temp / ( temp + 1) * (1) / N;

    //The 4+5Nminus1..4+6Nminus1 states are the GT matrix
    //Only 2 entries can differ from 0, (N-1)G going to mono G and (N-1)T going to mono T
    //(N-1)G can only go to monomorphic state G, i.e. j=2
    //(N-1)G is at the end of the submatrix, i=4+6*(Nminus1)
  //  temp = (N-1)*(1+s[2]-s[3]);
    (*theRateMatrix)[4 + 6*Nminus1 - 1][2] = computeEntryFromMoranProcessWithSelection(2, 3, Nminus1d);
  //  (*theRateMatrix)[4 + 6*Nminus1 - 1][2] = temp / ( temp + 1) * (1) / N;
    //(N-1)T can only go to monomorphic state T, i.e. j=3
    //(N-1)T is at the begining of the submatrix, i=4
  //  temp = (N-1)*(1+s[3]-s[2]);
    (*theRateMatrix)[4 + 5*Nminus1 ][3] = computeEntryFromMoranProcessWithSelection(3, 2, Nminus1d);
   // (*theRateMatrix)[4 + 5*Nminus1 ][3] = temp / ( temp + 1) * (1) / N;

    
    //Now we need to fill the rest of the matrix, i.e. the B matrices along the diagonal.
    //In these B matrices, again most cells = 0.
    //The diagonal is such that it's 0 - (sum of the cells in the line)
    
    for (size_t k = 0; k <= 5; k++){
        //Definition of the fitnesses
        double f1, f2;
        if (k<3) {
            f1 = s[0];
        }
        else if (k<5) {
            f1 = s[1];
        }
        else {
            f1 = s[2];
        }
        if (k==0) {
            f2 = s[1];
        }
        else if (k==2 || k==4) {
            f2 = s[2];
        }
        else {
            f2 = s[3];
        }
        
        for (size_t i = 4; i < N ; ++i){
            for (size_t j = i+1; j < N; j++){
                if ( j==(i+1) ){
                    (*theRateMatrix)[i+Nminus1*k][j+Nminus1*k] = (f1*i/(f1*i + f2*(N-i)) * (N-i)/N);
                    std::cout << "Hehe " << 4+j+Nminus1*k-1 << " HHH " <<4+i+Nminus1*k-1 <<std::endl;
                    (*theRateMatrix)[j+Nminus1*k][i+Nminus1*k] = (f2*j/(f2*j + f1*(N-j)) * (N-j)/N);
                }
                /*else{
                    (*theRateMatrix)[4+i+9*k,4+j+9*k] = M[4+j+9*k, 4+i+9*k] = 0;
                }*/
            }
            
        }
      /*  Diagonal 
       for (i in 1:9){
            (*theRateMatrix)[4+j+9*k,4+i+9*k] <- 1 - sum((*theRateMatrix)[4+j+9*k,(1:58)[-(4+i+9*k)]])
        }*/
    }
    
    
    
    
    /*
    double cell1, cell2;
    //First, the B^N_AC matrix
    //The 4..4+Nminus1 states are the AC matrix
    //Cell 4,4 is its first cell.
    size_t firstCell = 4;
    //case i = 0: moving from 1A to 2As
    double temp = 1.0;
   // temp = 1+s[0]-s[1];
    //XXXXXXXXXXXX >>>>>>> PARIS
//    (*theRateMatrix)[firstCell][firstCell+1] = temp / (temp + N - 1) * (N-1) / N;
    (*theRateMatrix)[firstCell][firstCell+1] = computeEntryFromMoranProcessWithSelection(0, 1, temp);
        for (size_t i=1; i< N-1; i++)
    {
      //  temp = i*(1+s[0]-s[1]);
        //Loosing one A, from iA to (i-1)A
        //cell1 = (N - i) /  (temp + N - i) * i / N;
        temp = (double) (N-i-1);
        cell1 = computeEntryFromMoranProcessWithSelection(1, 0, temp);
        (*theRateMatrix)[firstCell+i][firstCell+i-1] = cell1;
        //Gaining one A, from iA to (i+1)A
        //cell2 = temp / (temp + N - i) * (N-i) / N;
        if (i != 8 ) {
            temp = (double) (i+1);
            cell2 = computeEntryFromMoranProcessWithSelection(0, 1, temp);
        }
        else {
            cell2 = 0.0;
        }
        (*theRateMatrix)[firstCell+i][firstCell+i+1] = cell2;
        (*theRateMatrix)[firstCell+i][firstCell+i] = 0 - cell1 - cell2 - (*theRateMatrix)[firstCell+i][0] - (*theRateMatrix)[firstCell+i][1] - (*theRateMatrix)[firstCell+i][2] - (*theRateMatrix)[firstCell+i][3];
    }

    //Then, the B^N_AG matrix
    //The 4+Nminus1..4+2Nminus1 states are the AG matrix
    //Cell 4+N,4+N is its first cell.
    firstCell = 4+Nminus1;
    //case i = 0: moving from 1A to 2As
   // temp = 1+s[0]-s[2];
   // (*theRateMatrix)[firstCell][firstCell+1] = temp / (temp + N - 1) * (N-1) / N;
    temp = 1.0;
    (*theRateMatrix)[firstCell][firstCell+1] = computeEntryFromMoranProcessWithSelection(0, 2, temp);
        for (size_t i=1; i< N-1; i++)
    {
       // temp = i*(1+s[0]-s[2]);
        //Loosing one A, from iA to (i-1)A
        //cell1 = (N - i) /  (temp + N - i) * i / N;
        temp = (double) (N-i-1);
        cell1 = computeEntryFromMoranProcessWithSelection(2, 0, temp);
        (*theRateMatrix)[firstCell+i][firstCell+i-1] = cell1;

        //Gaining one A, from iA to (i+1)A
        //cell2 = temp / (temp + N - i) * (N-i) / N;
        if (i != 8 ) {
        temp = (double) (i+1);
        cell2 = computeEntryFromMoranProcessWithSelection(0, 2, temp);
        }
        else {
            cell2 = 0.0;
        }
        (*theRateMatrix)[firstCell+i][firstCell+i+1] = cell2;
        (*theRateMatrix)[firstCell+i][firstCell+i] = 0 - cell1 - cell2 - (*theRateMatrix)[firstCell+i][0] - (*theRateMatrix)[firstCell+i][1] - (*theRateMatrix)[firstCell+i][2] - (*theRateMatrix)[firstCell+i][3];
    }

    //Then, the B^N_AT matrix
    //The 4+2Nminus1..4+3Nminus1 states are the AT matrix
    //Cell 4+N,4+N is its first cell.
    firstCell = 4+2*Nminus1;
    //case i = 0: moving from 1A to 2As
   // temp = 1+s[0]-s[3];
//    (*theRateMatrix)[firstCell][firstCell+1] = temp / (temp + N - 1) * (N-1) / N;
    temp = 1.0;
    (*theRateMatrix)[firstCell][firstCell+1] = computeEntryFromMoranProcessWithSelection(0, 3, temp);
        for (size_t i=1; i< N-1; i++) 
    {
        //temp = i*(1+s[0]-s[3]);
        //Loosing one A, from iA to (i-1)A
        //cell1 = (N - i) /  (temp + N - i) * i / N;
        temp = (double) (N-i-1);
        cell1 = computeEntryFromMoranProcessWithSelection(3, 0, temp);
        (*theRateMatrix)[firstCell+i][firstCell+i-1] = cell1;
        //Gaining one A, from iA to (i+1)A
        //cell2 = temp / (temp + N - i) * (N-i) / N;
        if (i != 8 ) {
        temp = (double) (i+1);
        cell2 = computeEntryFromMoranProcessWithSelection(0, 3, temp);
        }
        else {
            cell2 = 0.0;
        }
        (*theRateMatrix)[firstCell+i][firstCell+i+1] = cell2;
        (*theRateMatrix)[firstCell+i][firstCell+i] = 0 - cell1 - cell2 - (*theRateMatrix)[firstCell+i][0] - (*theRateMatrix)[firstCell+i][1] - (*theRateMatrix)[firstCell+i][2] - (*theRateMatrix)[firstCell+i][3];
    }

    //Then, the B^N_CG matrix
    //The 4+3Nminus1..4+4Nminus1 states are the CG matrix
    //Cell 4+N,4+N is its first cell.
    firstCell = 4+3*Nminus1;
    //case i = 0: moving from 1C to 2Cs
  //  temp = 1+s[1]-s[2];
   // (*theRateMatrix)[firstCell][firstCell+1] = temp / (temp + N - 1) * (N-1) / N;
     temp = 1.0;
    (*theRateMatrix)[firstCell][firstCell+1] = computeEntryFromMoranProcessWithSelection(1, 2, temp);
        for (size_t i=1; i< N-1; i++) 
    {
      //  temp = i*(1+s[1]-s[2]);
        //Loosing one C, from iC to (i-1)C
    //    cell1 = (N - i) /  (temp + N - i) * i / N;
        temp = (double) (N-i-1);
        cell1 = computeEntryFromMoranProcessWithSelection(2, 1, temp);
        (*theRateMatrix)[firstCell+i][firstCell+i-1] = cell1;
        //Gaining one C, from iC to (i+1)C
        if (i != 8 ) {
            temp = (double) (i+1);
            cell2 = computeEntryFromMoranProcessWithSelection(1, 2, temp);
        }
        else {
            cell2 = 0.0;
        }
       // cell2 = temp / (temp + N - i) * (N-i) / N;
        (*theRateMatrix)[firstCell+i][firstCell+i+1] = cell2;
        (*theRateMatrix)[firstCell+i][firstCell+i] = 0 - cell1 - cell2 - (*theRateMatrix)[firstCell+i][0] - (*theRateMatrix)[firstCell+i][1] - (*theRateMatrix)[firstCell+i][2] - (*theRateMatrix)[firstCell+i][3];
    }

    //Then, the B^N_CT matrix
    //The 4+4Nminus1..4+5Nminus1 states are the CT matrix
    //Cell 4+N,4+N is its first cell.
    firstCell = 4+4*Nminus1;
    //case i = 0: moving from 1C to 2Cs
   // temp = 1+s[1]-s[3];
   // (*theRateMatrix)[firstCell][firstCell+1] = temp / (temp + N - 1) * (N-1) / N;
    temp = 1.0;
    (*theRateMatrix)[firstCell][firstCell+1] = computeEntryFromMoranProcessWithSelection(1, 3, temp);
        for (size_t i=1; i< N-1; i++) 
    {
       // temp = i*(1+s[1]-s[3]);
        //Loosing one C, from iC to (i-1)C
        //cell1 = (N - i) /  (temp + N - i) * i / N;
        temp = (double) (N-i-1);
        cell1 = computeEntryFromMoranProcessWithSelection(3, 1, temp);
        (*theRateMatrix)[firstCell+i][firstCell+i-1] = cell1;
        //Gaining one C, from iC to (i+1)C
       // cell2 = temp / (temp + N - i) * (N-i) / N;
        if (i != 8 ) {
            temp = (double) (i+1);
            cell2 = computeEntryFromMoranProcessWithSelection(1, 3, temp);
        }
        else {
            cell2 = 0.0;
        }
        (*theRateMatrix)[firstCell+i][firstCell+i+1] = cell2;
        (*theRateMatrix)[firstCell+i][firstCell+i] = 0 - cell1 - cell2 - (*theRateMatrix)[firstCell+i][0] - (*theRateMatrix)[firstCell+i][1] - (*theRateMatrix)[firstCell+i][2] - (*theRateMatrix)[firstCell+i][3];
    }

    //Then, the B^N_GT matrix
    //The 4+5Nminus1..4+6Nminus1 states are the GT matrix
    //Cell 4+N,4+N is its first cell.
    firstCell = 4+5*Nminus1;
    //case i = 0: moving from 1G to 2Gs
   // temp = 1+s[2]-s[3];
   // (*theRateMatrix)[firstCell][firstCell+1] = temp / (temp + N - 1) * (N-1) / N;
    temp = 1.0;
    (*theRateMatrix)[firstCell][firstCell+1] = computeEntryFromMoranProcessWithSelection(2, 3, temp);
        for (size_t i=1; i< N-1; i++)
    {
      //  temp = i*(1+s[2]-s[3]);
        //Loosing one G, from iG to (i-1)G
       // cell1 = (N - i) /  (temp + N - i) * i / N;
        temp = (double) (N-i-1);
        cell1 = computeEntryFromMoranProcessWithSelection(3, 2, temp);
        (*theRateMatrix)[firstCell+i][firstCell+i-1] = cell1;
        //Gaining one G, from iG to (i+1)G
        if (i != 8 ) {
        temp = (double) (i+1);
     //   cell2 = temp / (temp + N - i) * (N-i) / N;
        cell2 = computeEntryFromMoranProcessWithSelection(2, 3, temp);
        }
        else {
            cell2 = 0.0;
        }
        if (i != 8 ) {
            (*theRateMatrix)[firstCell+i][firstCell+i+1] = cell2;
        }
        (*theRateMatrix)[firstCell+i][firstCell+i] = 0 - cell1 - cell2 - (*theRateMatrix)[firstCell+i][0] - (*theRateMatrix)[firstCell+i][1] - (*theRateMatrix)[firstCell+i][2] - (*theRateMatrix)[firstCell+i][3];
    }
    
    */
    
    //In the first 4 rows/columns, the diagonal is defined such that the sum by line is 1.
    double sum = 0.0;
    for (size_t i=0; i< matrixSize; i++)
    {
        sum += (*theRateMatrix)[0][i];
    }
    (*theRateMatrix)[0][0] = 0-sum;
    
    sum = 0.0;
    for (size_t i=0; i< matrixSize; i++)
    {
        sum += (*theRateMatrix)[1][i];
    }
    (*theRateMatrix)[1][1] = 0-sum;
    
    sum = 0.0;
    for (size_t i=0; i< matrixSize; i++)
    {
        sum += (*theRateMatrix)[2][i];
    }
    (*theRateMatrix)[2][2] = 0-sum;
    
    sum = 0.0;
    for (size_t i=0; i< matrixSize; i++)
    {
        sum += (*theRateMatrix)[3][i];
    }
    (*theRateMatrix)[3][3] = 0-sum;
    
    for (size_t i=0; i< matrixSize; i++)
    {
        for (size_t j=0; j< matrixSize; j++)
        {
        (*theRateMatrix)[i][j] *= (double) N;
        }
    }

    
    // set the diagonal values
    setDiagonal();
    
    
    
    // rescale
    //rescaleToAverageRate( 1.0 );

    
    //Then we remove the identity matrix
  /*  for (size_t i=0; i< matrixSize; i++)
    {
        (*theRateMatrix)[i][i] = (*theRateMatrix)[i][i] - 1 ;
    }*/
}


double RateMatrix_Pomo::computeEntryFromMoranProcessWithSelection(size_t state1, size_t state2, double& count1){
    //We always assume state1 with count1 is increasing
    double count2 = (double)N-count1;
    //One of state2 alleles is chosen for disappearance
    double result = count2/(double)N; // 1/count2;
    //One of state1 alleles is chosen for replication
    result *= s[state1]*count1 / ( s[state2]*count2 + s[state1]*count1) ;
    return result;
}


/** Calculate the transition probabilities */
void RateMatrix_Pomo::calculateTransitionProbabilities(double t, TransitionProbabilityMatrix& P) const {
   // std::cout << "In calculateTransitionProbabilities: "<< t <<std::endl;
    
    //Now the instantaneous rate matrix has been filled up entirely.
    //We use repeated squaring to quickly obtain exponentials, as in Poujol and Lartillot, Bioinformatics 2014.
    computeExponentialMatrixByRepeatedSquaring(t, P);
    
/*    for (size_t i = 0 ; i<58; ++i) {
      //  for (size_t j = 0 ; j<58; ++j) {
            std::cout << "t: "<< t <<  " Diag "<< i << " : "<< P.getElement(i, i)<<std::endl;
        //}
        
    }*/
    
    return;
}

void RateMatrix_Pomo::computeExponentialMatrixByRepeatedSquaring(double t,  TransitionProbabilityMatrix& P ) const {
    //We use repeated squaring to quickly obtain exponentials, as in Poujol and Lartillot, Bioinformatics 2014.
    //Ideally one should dynamically decide how many squarings are necessary. 
    //For the moment, we arbitrarily do 10 such squarings, as it seems to perform well in practice (N. Lartillot, personal communication).
    //first, multiply the matrix by the right scalar
    //2^10 = 1024
    double tOver2s = t/(1024);
    for ( size_t i = 0; i < matrixSize; i++ ) {
        for ( size_t j = 0; j < matrixSize; j++ ) {
            P[i][j] = (*theRateMatrix)[i][j] * tOver2s; 
        }
    }
    //Add the identity matrix:
     for ( size_t i = 0; i < matrixSize; i++ ) {
         P[i][i] += 1;
     }
     //Now we can do the multiplications
     TransitionProbabilityMatrix P2 (matrixSize);
     squareMatrix (P, P2); //P2 at power 2
     squareMatrix (P2, P); //P at power 4
     squareMatrix (P, P2); //P2 at power 8
     squareMatrix (P2, P); //P at power 16
     squareMatrix (P, P2); //P2 at power 32
     squareMatrix (P2, P); //P at power 64
     squareMatrix (P, P2); //P2 at power 128
     squareMatrix (P2, P); //P at power 256
     squareMatrix (P, P2); //P2 at power 512
     squareMatrix (P2, P); //P at power 1024

     return;
}

inline void RateMatrix_Pomo::squareMatrix( TransitionProbabilityMatrix& P,  TransitionProbabilityMatrix& P2) const {
    //Could probably use boost::ublas here, for the moment we do it ourselves.
    for ( size_t i = 0; i < matrixSize; i++ ) {
        for ( size_t j = 0; j < matrixSize; j++ ) {
            P2.getElement ( i, j ) = 0;
            for ( size_t k = 0; k < matrixSize; k++ ) {
                P2.getElement ( i, j ) += P.getElement ( i, k ) * P.getElement ( k, j );
                }
            }
        }
    }



RateMatrix_Pomo* RateMatrix_Pomo::clone( void ) const {
    return new RateMatrix_Pomo( *this );
}

const std::vector<double>& RateMatrix_Pomo::getStationaryFrequencies( void ) const {
    
    return stationaryFreqs;
}


void RateMatrix_Pomo::updateMatrix( void ) {
    
    if ( needsUpdate )
    {
        buildRateMatrix();
        // clean flags
        needsUpdate = false;
    }
}


void RateMatrix_Pomo::setMutationRates(const std::vector<double>& mr) {

    mu[0][1] = mr[0];
    mu[0][2] = mr[1];
    mu[0][3] = mr[2];
    mu[1][0] = mr[3];
    mu[1][2] = mr[4];
    mu[1][3] = mr[5];
    mu[2][0] = mr[6];
    mu[2][1] = mr[7];
    mu[2][3] = mr[8];
    mu[3][0] = mr[9];
    mu[3][1] = mr[10];
    mu[3][2] = mr[11];
}


void RateMatrix_Pomo::setMutationRates(const RateMatrix& mm) {
    
    mu[0][1] = mm[0][1];
    mu[0][2] = mm[0][2];
    mu[0][3] = mm[0][3];
    mu[1][0] = mm[1][0];
    mu[1][2] = mm[1][2];
    mu[1][3] = mm[1][3];
    mu[2][0] = mm[2][0];
    mu[2][1] = mm[2][1];
    mu[2][3] = mm[2][3];
    mu[3][0] = mm[3][0];
    mu[3][1] = mm[3][1];
    mu[3][2] = mm[3][2];
}


void RateMatrix_Pomo::setSelectionCoefficients(const std::vector<double>& sc){
    s = sc;

}
