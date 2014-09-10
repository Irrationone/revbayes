//
//  AminoAcidRateMapFunction.cpp
//  rb_mlandis
//
//  Created by Michael Landis on 4/3/14.
//  Copyright (c) 2014 Michael Landis. All rights reserved.
//

#include "AminoAcidRateMapFunction.h"
#include "RateMatrix_Blosum62.h"
#include "RateMap_AminoAcid.h"
#include "ConstantNode.h"
#include "RbException.h"

using namespace RevBayesCore;

AminoAcidRateMapFunction::AminoAcidRateMapFunction(size_t nc) : TypedFunction<RateMap>( new RateMap_AminoAcid(nc) )
{
    homogeneousRateMatrix               = new ConstantNode<RateMatrix>("homogeneousRateMatrix", new RateMatrix_Blosum62());
    heterogeneousRateMatrices           = NULL;
    homogeneousClockRate                = new ConstantNode<double>("clockRate", new double(1.0) );
    heterogeneousClockRates             = NULL;
    rootFrequencies                     = new ConstantNode<std::vector<double> >("rootFrequencies", new std::vector<double>(20,0.05));
    
    branchHeterogeneousClockRates       = false;
    branchHeterogeneousRateMatrices    = false;
    
    this->addParameter(homogeneousRateMatrix);
    this->addParameter(homogeneousClockRate);
    this->addParameter(rootFrequencies);
    
    update();
}


AminoAcidRateMapFunction::AminoAcidRateMapFunction(const AminoAcidRateMapFunction &n) : TypedFunction<RateMap>( n )
{
    homogeneousRateMatrix = n.homogeneousRateMatrix;
    heterogeneousRateMatrices = n.heterogeneousRateMatrices;
    homogeneousClockRate = n.homogeneousClockRate;
    heterogeneousClockRates = n.heterogeneousClockRates;
    rootFrequencies = n.rootFrequencies;
    
    branchHeterogeneousClockRates = n.branchHeterogeneousClockRates;
    branchHeterogeneousRateMatrices = n.branchHeterogeneousRateMatrices;
}


AminoAcidRateMapFunction::~AminoAcidRateMapFunction( void ) {
    // We don't delete the parameters, because they might be used somewhere else too. The model needs to do that!
}



AminoAcidRateMapFunction* AminoAcidRateMapFunction::clone( void ) const {
    return new AminoAcidRateMapFunction( *this );
}


void AminoAcidRateMapFunction::update( void ) {
    
    // set the gainLossRate
    if (branchHeterogeneousRateMatrices)
    {
        const RbVector<RateMatrix>& rm = heterogeneousRateMatrices->getValue();
        static_cast<RateMap_AminoAcid*>(value)->setHeterogeneousRateMatrices(rm);
    }
    else
    {
        const RateMatrix& rm = homogeneousRateMatrix->getValue();
        static_cast<RateMap_AminoAcid*>(value)->setHomogeneousRateMatrix(&rm);
    }
    
    if (branchHeterogeneousClockRates)
    {
        const std::vector<double>& r = heterogeneousClockRates->getValue();
        static_cast< RateMap_AminoAcid* >(value)->setHeterogeneousClockRates(r);
    }
    else
    {
        const double& r = homogeneousClockRate->getValue();
        static_cast< RateMap_AminoAcid* >(value)->setHomogeneousClockRate(r);
    }
    
    value->updateMap();
}

void AminoAcidRateMapFunction::setRateMatrix(const TypedDagNode<RateMatrix>* r)
{
    // remove the old parameter first
    if ( homogeneousRateMatrix != NULL )
    {
        this->removeParameter( homogeneousRateMatrix );
        homogeneousRateMatrix = NULL;
    }
    else
    {
        this->removeParameter( heterogeneousRateMatrices );
        heterogeneousRateMatrices = NULL;
    }
    
    // set the value
    branchHeterogeneousRateMatrices = false;
    homogeneousRateMatrix = r;
    
    // add the parameter
    this->addParameter( homogeneousRateMatrix );
}

void AminoAcidRateMapFunction::setClockRate(const TypedDagNode< double > *r) {
    
    // remove the old parameter first
    if ( homogeneousClockRate != NULL )
    {
        this->removeParameter( homogeneousClockRate );
        homogeneousClockRate = NULL;
    }
    else // heterogeneousClockRate != NULL
    {
        this->removeParameter( heterogeneousClockRates );
        heterogeneousClockRates = NULL;
    }
    
    // set the value
    branchHeterogeneousClockRates = false;
    homogeneousClockRate = r;
    
    // add the parameter
    this->addParameter( homogeneousClockRate );
}

void AminoAcidRateMapFunction::setClockRate(const TypedDagNode< std::vector< double > > *r) {
    
    // remove the old parameter first
    if ( homogeneousClockRate != NULL )
    {
        this->removeParameter( homogeneousClockRate );
        homogeneousClockRate = NULL;
    }
    else // heterogeneousClockRate != NULL
    {
        this->removeParameter( heterogeneousClockRates );
        heterogeneousClockRates = NULL;
    }
    
    // set the value
    branchHeterogeneousClockRates = true;
    heterogeneousClockRates = r;
    
    // add the parameter
    this->addParameter( heterogeneousClockRates );
    
}

void AminoAcidRateMapFunction::setRootFrequencies(const TypedDagNode<std::vector<double> > *f)
{
    if (rootFrequencies != NULL)
    {
        this->removeParameter(rootFrequencies);
        rootFrequencies = NULL;
    }
    
    rootFrequencies = f;
    this->addParameter(rootFrequencies);
}

void AminoAcidRateMapFunction::swapParameterInternal(const DagNode *oldP, const DagNode *newP)
{
    if (oldP == homogeneousRateMatrix)
    {
        homogeneousRateMatrix = static_cast<const TypedDagNode<RateMatrix>* >( newP );
    }
    else if (oldP == heterogeneousRateMatrices)
    {
        heterogeneousRateMatrices = static_cast<const TypedDagNode<RbVector<RateMatrix> >* >( newP );
    }
    else if (oldP == homogeneousClockRate)
    {
        homogeneousClockRate = static_cast<const TypedDagNode< double >* >( newP );
    }
    else if (oldP == heterogeneousClockRates)
    {
        heterogeneousClockRates = static_cast<const TypedDagNode< std::vector< double > >* >( newP );
    }
    else if (oldP == rootFrequencies)
    {
        rootFrequencies = static_cast<const TypedDagNode<std::vector<double> >* >( newP );
    }
}

//std::ostream& operator<<(std::ostream& o, const std::vector<std::vector<double> >& x)
//{
//    o << "";
//    return o;
//}
