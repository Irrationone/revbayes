#include "Clade.h"
#include "ConstantNode.h"
#include "MultispeciesCoalescentInverseGammaPrior.h"
#include "DistributionExponential.h"
#include "DistributionInverseGamma.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include "RbConstants.h"
#include "RbMathCombinatorialFunctions.h"
#include "RbMathFunctions.h"
#include "TopologyNode.h"

#include <algorithm>
#include <cmath>

using namespace RevBayesCore;

MultispeciesCoalescentInverseGammaPrior::MultispeciesCoalescentInverseGammaPrior(const TypedDagNode<Tree> *sp, const std::vector<Taxon> &t) : AbstractMultispeciesCoalescent(sp, t)
{
    
}


MultispeciesCoalescentInverseGammaPrior::~MultispeciesCoalescentInverseGammaPrior()
{
    
}





MultispeciesCoalescentInverseGammaPrior* MultispeciesCoalescentInverseGammaPrior::clone( void ) const
{
    
    return new MultispeciesCoalescentInverseGammaPrior( *this );
}


double MultispeciesCoalescentInverseGammaPrior::computeLnCoalescentProbability(size_t k, const std::vector<double> &times, double begin_age, double end_age, size_t index, bool add_final_interval)
{
    double alpha = shape->getValue();
    double beta =  rate->getValue();
    
    size_t n = times.size();
    double a = k - n;

    double current_time = begin_age;
    double b = 0.0;
    for (size_t i=0; i<n; ++i)
    {
        
        // now we do the computation
        //a is the time between the previous and the current coalescences
        double a = times[i] - current_time;
        
        // get the number j of individuals we had before the current coalescence
        size_t j = k - i;
        double n_pairs = j * (j-1.0) / 2.0;
        
        b += a * n_pairs;
    }
    
    // compute the probability of no coalescent event in the final part of the branch
    // only do this if the branch is not the root branch
    if ( add_final_interval == false )
    {
        double final_interval = end_age - current_time;
        size_t j = k - times.size();
        double n_pairs = j * (j-1.0) / 2.0;
        b += final_interval * n_pairs;
        
    }
    
    double ln_prob_coal = RbConstants::LN2 * a + log(beta) * alpha + log( RbMath::gamma(a+alpha) ) - log( RbMath::gamma(alpha) ) - log(b+beta)*(a+alpha);
    
    return ln_prob_coal;
}


double MultispeciesCoalescentInverseGammaPrior::drawNe( size_t index )
{
    
    double u = RbStatistics::InverseGamma::rv(shape->getValue(), rate->getValue(), *GLOBAL_RNG);
    
    return u;
}



void MultispeciesCoalescentInverseGammaPrior::setShape(TypedDagNode<double>* s)
{
    
    removeParameter( shape );
    
    shape = s;
    
    addParameter( shape );
}



void MultispeciesCoalescentInverseGammaPrior::setRate(TypedDagNode<double>* r)
{
    
    removeParameter( rate );
    
    rate = r;
    
    addParameter( rate );
}


/** Swap a parameter of the distribution */
void MultispeciesCoalescentInverseGammaPrior::swapParameterInternal(const DagNode *oldP, const DagNode *newP)
{
    
    if ( oldP == rate )
    {
        rate = static_cast<const TypedDagNode< double >* >( newP );
    }
    
    if ( oldP == shape )
    {
        shape = static_cast<const TypedDagNode< double >* >( newP );
    }
    AbstractMultispeciesCoalescent::swapParameterInternal(oldP, newP);
    
}
