#include "LognormalDistribution.h"
#include "DistributionLognormal.h"
#include "RandomNumberFactory.h"
#include "RbConstants.h"

using namespace RevBayesCore;

LognormalDistribution::LognormalDistribution(const TypedDagNode<double> *m, const TypedDagNode<double> *s) : ContinuousDistribution( new double( 1.0 ) ), mean( m ), sd( s ) {
    // add the parameters to the parents set
    addParameter( mean );
    addParameter( sd );
    
    *value = RbStatistics::Lognormal::rv(mean->getValue(), sd->getValue(), *GLOBAL_RNG);
}


LognormalDistribution::~LognormalDistribution( void ) {
    // We don't delete the parameters, because they might be used somewhere else too. The model needs to do that!
}


double LognormalDistribution::cdf( void ) const {
    return RbStatistics::Lognormal::cdf(mean->getValue(), sd->getValue(), *value);
}


LognormalDistribution* LognormalDistribution::clone( void ) const {
    return new LognormalDistribution( *this );
}


double LognormalDistribution::computeLnProbability( void ) {
    return RbStatistics::Lognormal::lnPdf(mean->getValue(), sd->getValue(), *value);
}


double LognormalDistribution::getMax( void ) const {
    return RbConstants::Double::inf;
}


double LognormalDistribution::getMin( void ) const {
    return 0.0;
}


double LognormalDistribution::quantile(double p) const {
    return RbStatistics::Lognormal::quantile(mean->getValue(), sd->getValue(), p);
}


void LognormalDistribution::redrawValue( void ) {
    *value = RbStatistics::Lognormal::rv(mean->getValue(), sd->getValue(), *GLOBAL_RNG);
}


void LognormalDistribution::swapParameter(const DagNode *oldP, const DagNode *newP) {
    if (oldP == mean) {
        mean = static_cast<const TypedDagNode<double>* >( newP );
    }
    if (oldP == sd) {
        sd = static_cast<const TypedDagNode<double>* >( newP );
    }
}
