#include "PhyloCTMCSiteHomogeneousDollo.h"
#include "RbMathFunctions.h"

RevBayesCore::PhyloCTMCSiteHomogeneousDollo::PhyloCTMCSiteHomogeneousDollo(const TypedDagNode<Tree> *t, size_t nc, bool c, size_t nSites, bool amb, DolloAscertainmentBias::Coding ty, bool norm) :
    PhyloCTMCSiteHomogeneousConditional<StandardState>(  t, nc, c, nSites, amb, AscertainmentBias::Coding(ty)), integrationFactors(0), normalize(norm)
{
    massNodeOffset = this->num_site_rates*numCorrectionMasks;
    activeMassOffset = this->num_nodes*massNodeOffset;
    perMaskMixtureCorrections = std::vector<double>(2*activeMassOffset, 0.0);

    death_rate = NULL;
    this->addParameter(death_rate);

    survival = std::vector<double>(this->num_site_rates,0.0);
    integrationFactors = std::vector<double>(this->num_site_rates,0.0);
    maskNodeObservationCounts = std::vector<std::vector<size_t> >(numCorrectionMasks, std::vector<size_t>(num_nodes, 0) );

    // set the offsets for easier iteration through the likelihood vector
    // we use an extra site for the integrated likelihood
    siteOffset                  =  num_chars + 1;
    mixtureOffset               =  pattern_block_size*siteOffset;
    nodeOffset                  =  num_site_rates*mixtureOffset;
    activeLikelihoodOffset      =  num_nodes*nodeOffset;

}

RevBayesCore::PhyloCTMCSiteHomogeneousDollo::PhyloCTMCSiteHomogeneousDollo(const PhyloCTMCSiteHomogeneousDollo& n) :
        PhyloCTMCSiteHomogeneousConditional<StandardState>(n),
        integrationFactors(n.integrationFactors),
        maskNodeObservationCounts(n.maskNodeObservationCounts),
        survival(n.survival),
        activeMassOffset(n.activeMassOffset),
        massNodeOffset(n.massNodeOffset),
        normalize(n.normalize),
        death_rate(n.death_rate)
{

}

RevBayesCore::PhyloCTMCSiteHomogeneousDollo* RevBayesCore::PhyloCTMCSiteHomogeneousDollo::clone( void ) const {

    return new PhyloCTMCSiteHomogeneousDollo( *this );
}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::resizeLikelihoodVectors( void )
{
    RevBayesCore::PhyloCTMCSiteHomogeneousConditional<StandardState>::resizeLikelihoodVectors();

    massNodeOffset = num_site_rates*numCorrectionMasks;
    activeMassOffset = num_nodes*massNodeOffset;
    perMaskMixtureCorrections = std::vector<double>(2*activeMassOffset, 0.0);

    maskNodeObservationCounts = std::vector<std::vector<size_t> >(numCorrectionMasks, std::vector<size_t>(num_nodes, 0) );

    integrationFactors = std::vector<double>(num_site_rates, 0.0);
    survival = std::vector<double>(this->num_site_rates, 0.0);

    // set the offsets for easier iteration through the likelihood vector
    siteOffset                  =  num_chars + 1;
    mixtureOffset               =  pattern_block_size*siteOffset;
    nodeOffset                  =  num_site_rates*mixtureOffset;
    activeLikelihoodOffset      =  num_nodes*nodeOffset;

    // only do this if we are in MCMC mode. This will safe memory
    if ( inMcmcMode == true )
    {
        // we resize the partial likelihood vectors to the new dimensions
        delete [] partialLikelihoods;

        partialLikelihoods = new double[2*activeLikelihoodOffset];

        // reinitialize likelihood vectors
        for (size_t i = 0; i < 2*activeLikelihoodOffset; i++)
        {
            partialLikelihoods[i] = 0.0;
        }
    }
}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::updateTransitionProbabilities(size_t nodeIdx, double brlen)
{
    // first, get the rate matrix for this branch
    const RateGenerator *rg = NULL;
    if ( this->branch_heterogeneous_substitution_matrices == true )
    {
        rg = &this->heterogeneous_rate_matrices->getValue()[nodeIdx];
    }
    else if(homogeneous_rate_matrix != NULL)
    {
        rg = dynamic_cast<const RateMatrix *>(&this->homogeneous_rate_matrix->getValue());
    }

    const RateMatrix *rm = dynamic_cast<const RateMatrix *>(rg);
    if ( rm == NULL && rg != NULL)
    {
        throw RbException("Dollo model cannot use RateGenerators that are not RateMatrices");
    }

    // second, get the clock rate for the branch
    double rate = 1.0;
    if ( this->branch_heterogeneous_clock_rates == true )
    {
        rate = this->heterogeneous_clock_rates->getValue()[nodeIdx];
    }
    else if(this->homogeneous_clock_rate != NULL)
    {
        rate = this->homogeneous_clock_rate->getValue();
    }

    // third, get the death rate
    double death = 1.0;
    if(death_rate != NULL)
    {
        death = death_rate->getValue();
    }

    // and finally compute the per site rate transition probability matrix
    const TopologyNode* node = tau->getValue().getNodes()[nodeIdx];

    double endAge = node->getAge();

    // if the tree is not a time tree, then the age will be not a number
    if ( RbMath::isFinite(endAge) == false )
    {
        // we assume by default that the end is at time 0
        endAge = 0.0;
    }
    double startAge = endAge + node->getBranchLength();

    double mu = death * rate;
    double beta = rate;

    if(rm != NULL && normalize)
    {
        double avg = rm->averageRate();

        beta *= avg / (avg + death);
        mu /= (avg + death);
    }

    if ( this->rate_variation_across_sites == true )
    {
        const std::vector<double> &r = this->site_rates->getValue();
        for (size_t i = 0; i < this->num_site_rates; ++i)
        {
            double scaled_mu = r[i] * mu;

            if(node->isRoot())
            {
                integrationFactors[i] = 1.0/scaled_mu;
            }
            else
            {
                survival[i] = exp( - scaled_mu * (startAge - endAge) );
                integrationFactors[i] = (1.0 - survival[i])/scaled_mu;

                if(rm != NULL)
                {
                    rm->calculateTransitionProbabilities( startAge, endAge,  r[i] * beta, this->transition_prob_matrices[i] );

                    for(size_t a = 0; a < num_chars - 1; a++)
                    {
                        for(size_t b = 0; b < num_chars - 1; b++)
                        {
                            this->transition_prob_matrices[i][a][b] *= survival[i];
                        }

                        this->transition_prob_matrices[i][a][num_chars - 1] = 1.0 - survival[i];
                    }

                    this->transition_prob_matrices[i][num_chars - 1][num_chars - 1] = 1.0;
                }
                else
                {
                    this->transition_prob_matrices[i][0][0] = 1.0;
                    this->transition_prob_matrices[i][0][1] = 0.0;
                    this->transition_prob_matrices[i][1][0] = 1.0 - survival[i];
                    this->transition_prob_matrices[i][1][1] = survival[i];
                }
            }
        }
    }
    else
    {
        if(node->isRoot())
        {
            integrationFactors[0] = 1.0/mu;
        }
        else
        {
            survival[0] = exp( - mu * (startAge - endAge) );
            integrationFactors[0] = (1.0 - survival[0])/mu;

            if(rm != NULL)
            {
                rm->calculateTransitionProbabilities( startAge, endAge,  beta, this->transition_prob_matrices[0] );

                for(size_t a = 0; a < num_chars - 1; a++)
                {
                    for(size_t b = 0; b < num_chars - 1; b++)
                    {
                        this->transition_prob_matrices[0][a][b] *= survival[0];
                    }

                    this->transition_prob_matrices[0][a][num_chars - 1] = 1.0 - survival[0];
                }

                this->transition_prob_matrices[0][num_chars - 1][num_chars - 1] = 1.0;
            }
            else
            {
                this->transition_prob_matrices[0][0][0] = 1.0;
                this->transition_prob_matrices[0][0][1] = 0.0;
                this->transition_prob_matrices[0][1][0] = 1.0 - survival[0];
                this->transition_prob_matrices[0][1][1] = survival[0];
            }
        }
    }
}

std::vector<double> RevBayesCore::PhyloCTMCSiteHomogeneousDollo::getStationaryFrequencies( size_t nodeIdx ) const
{
    const TopologyNode* node = tau->getValue().getNodes()[nodeIdx];

    if ( node->isRoot() )
    {
        return getRootFrequencies();
    }

    // first, get the rate matrix for this branch
    const RateGenerator *rg = NULL;
    if ( this->branch_heterogeneous_substitution_matrices == true )
    {
        rg = &this->heterogeneous_rate_matrices->getValue()[nodeIdx];
    }
    else if(this->homogeneous_rate_matrix != NULL)
    {
        rg = &this->homogeneous_rate_matrix->getValue();
    }

    if(rg != NULL)
    {
        const RateMatrix *rm = dynamic_cast<const RateMatrix *>(rg);
        if ( rm != NULL )
        {
            return rm->getStationaryFrequencies();
        }
        else
        {
            throw RbException("Dollo model cannot use RateGenerators that are not RateMatrices");
        }
    }
    else
    {
        return std::vector<double>(1,1.0);
    }

}

std::vector<double> RevBayesCore::PhyloCTMCSiteHomogeneousDollo::getRootFrequencies( void ) const
{
    if(this->root_frequencies != NULL && this->branch_heterogeneous_substitution_matrices == false && this->homogeneous_rate_matrix == NULL)
    {
        throw RbException("Cannot specify root frequencies in Dollo model without first specifying a rate matrix");
    }
    else if ( this->branch_heterogeneous_substitution_matrices == true || this->root_frequencies != NULL )
    {
        if(this->root_frequencies == NULL)
        {
            throw RbException("Using branch-heterogeneous rate matrices, but no root frequencies have been specified");
        }

        std::vector<double> rf = root_frequencies->getValue();

        if(rf.size() != this->num_chars - 1)
        {
            std::stringstream ss;
            ss << "Number of root frequencies (" << rf.size() << ") does not match the number of character states (" << this->num_chars - 1 << ")";
            throw RbException(ss.str());
        }

        return root_frequencies->getValue();
    }
    else if(this->homogeneous_rate_matrix != NULL)
    {
        const RateMatrix *rm = dynamic_cast<const RateMatrix *>(&this->homogeneous_rate_matrix->getValue());
        if ( rm != NULL )
        {
            if(rm->size() != this->num_chars - 1)
            {
                std::stringstream ss;
                ss << "Rate matrix size (" << rm->size() << ") does not match the number of character states (" << this->num_chars - 1 << ")";
                throw RbException(ss.str());
            }

            return rm->getStationaryFrequencies();
        }
        else
        {
            throw RbException("Dollo model cannot use RateGenerators that are not RateMatrices");
        }

    }
    else
    {
        return std::vector<double>(1,1.0);
    }

}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::setDeathRate(const TypedDagNode< double > *r)
{

    // remove the old parameter first
    if ( death_rate != NULL )
    {
        this->removeParameter( death_rate );
        death_rate = NULL;
    }

    // set the value
    death_rate = r;

    // add the new parameter
    this->addParameter( death_rate );

    // redraw the current value
    if ( this->dag_node == NULL || this->dag_node->isClamped() == false )
    {
        this->redrawValue();
    }

}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::computeRootLikelihood( size_t root, size_t left, size_t right)
{
    // compute the transition probability matrix
    updateTransitionProbabilities( root, 0 );

    // get the root frequencies
    const std::vector<double> &f = this->getRootFrequencies();

    // get the pointers to the partial likelihoods of the left and right subtree
          double* p        = partialLikelihoods + activeLikelihood[root]  * activeLikelihoodOffset + root  * nodeOffset;
    const double* p_left   = partialLikelihoods + activeLikelihood[left]  * activeLikelihoodOffset + left  * nodeOffset;
    const double* p_right  = partialLikelihoods + activeLikelihood[right] * activeLikelihoodOffset + right * nodeOffset;

    // get pointers the likelihood for both subtrees
          double*   p_mixture          = p;
    const double*   p_mixture_left     = p_left;
    const double*   p_mixture_right    = p_right;

    size_t n = num_chars - 1;

    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < num_site_rates; ++mixture)
    {
        // get pointers to the likelihood for this mixture category
              double*   p_site_mixture          = p_mixture;
        const double*   p_site_mixture_left     = p_mixture_left;
        const double*   p_site_mixture_right    = p_mixture_right;
        // iterate over all sites
        for (size_t site = 0; site < pattern_block_size; ++site)
        {
            p_site_mixture[n + 1] = 0.0;

            p_site_mixture[n] = p_site_mixture_left[n] * p_site_mixture_right[n];

            if(p_site_mixture[n] == 0)
            {
                for(size_t c = 0; c < n; c++)
                {
                    p_site_mixture[n + 1] += p_site_mixture_left[c] * p_site_mixture_right[c] * f[c];
                }

                p_site_mixture[n + 1] *= integrationFactors[mixture];
            }

            if( !RbSettings::userSettings().getUseScaling() )
            {
                p_site_mixture[n + 1] += ( p_site_mixture_left[n] > 0 ) * p_site_mixture_right[n + 1] + ( p_site_mixture_right[n] > 0 ) * p_site_mixture_left[n + 1];
            }

            // increment the pointers to the next site
            p_site_mixture+=siteOffset; p_site_mixture_left+=siteOffset; p_site_mixture_right+=siteOffset;

        } // end-for over all sites (=patterns)

        // increment the pointers to the next mixture category
        p_mixture+=mixtureOffset; p_mixture_left+=mixtureOffset; p_mixture_right+=mixtureOffset;

    } // end-for over all mixtures (=rate categories)

    computeRootCorrection(root, left, right);
}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::computeRootLikelihood( size_t root, size_t left, size_t right, size_t middle)
{
    // compute the transition probability matrix
    updateTransitionProbabilities( root, 0 );

    // get the root frequencies
    const std::vector<double> &f = this->getRootFrequencies();


    // get the pointers to the partial likelihoods of the left and right subtree
          double* p        = partialLikelihoods + activeLikelihood[root]   * activeLikelihoodOffset + root   * nodeOffset;
    const double* p_left   = partialLikelihoods + activeLikelihood[left]   * activeLikelihoodOffset + left   * nodeOffset;
    const double* p_right  = partialLikelihoods + activeLikelihood[right]  * activeLikelihoodOffset + right  * nodeOffset;
    const double* p_middle = partialLikelihoods + activeLikelihood[middle] * activeLikelihoodOffset + middle * nodeOffset;

    // get pointers the likelihood for both subtrees
          double*   p_mixture          = p;
    const double*   p_mixture_left     = p_left;
    const double*   p_mixture_right    = p_right;
    const double*   p_mixture_middle   = p_middle;

    size_t n = num_chars - 1;

    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < num_site_rates; ++mixture)
    {
        // get pointers to the likelihood for this mixture category
              double*   p_site_mixture          = p_mixture;
        const double*   p_site_mixture_left     = p_mixture_left;
        const double*   p_site_mixture_right    = p_mixture_right;
        const double*   p_site_mixture_middle   = p_mixture_middle;
        // iterate over all sites
        for (size_t site = 0; site < pattern_block_size; ++site)
        {
            p_site_mixture[n + 1] = 0.0;

            p_site_mixture[n] = p_site_mixture_left[n] * p_site_mixture_right[n] * p_site_mixture_middle[n];

            if(p_site_mixture[n] == 0)
            {
                for(size_t c = 0; c < n; c++)
                {
                    p_site_mixture[n + 1] += p_site_mixture_left[c] * p_site_mixture_right[c] * p_site_mixture_middle[c] * f[c];
                }

                p_site_mixture[n + 1] *= integrationFactors[mixture];
            }

            if( !RbSettings::userSettings().getUseScaling() )
            {
                p_site_mixture[n + 1] += ( p_site_mixture_left[n] > 0 )   * ( p_site_mixture_middle[n] > 0 ) * p_site_mixture_right[n + 1]
                                       + ( p_site_mixture_left[n] > 0 )   * ( p_site_mixture_right[n] > 0 )  * p_site_mixture_middle[n + 1]
                                       + ( p_site_mixture_middle[n] > 0 ) * ( p_site_mixture_right[n] > 0 )  * p_site_mixture_left[n + 1];
            }

            // increment the pointers to the next site
            p_site_mixture+=siteOffset; p_site_mixture_left+=siteOffset; p_site_mixture_right+=siteOffset; p_site_mixture_middle+=siteOffset;

        } // end-for over all sites (=patterns)

        // increment the pointers to the next mixture category
        p_mixture+=mixtureOffset; p_mixture_left+=mixtureOffset; p_mixture_right+=mixtureOffset; p_mixture_middle+=mixtureOffset;

    } // end-for over all mixtures (=rate categories)

    computeRootCorrection(root, left, right, middle);
}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::computeInternalNodeLikelihood(const TopologyNode &node, size_t node_index, size_t left, size_t right)
{
    // compute the transition probability matrix
    updateTransitionProbabilities( node_index, node.getBranchLength() );

    // get the root frequencies
    const std::vector<double> &f = getStationaryFrequencies(node_index);

    // get the pointers to the partial likelihoods for this node and the two descendant subtrees
    const double*   p_left  = partialLikelihoods + activeLikelihood[left]*activeLikelihoodOffset + left*nodeOffset;
    const double*   p_right = partialLikelihoods + activeLikelihood[right]*activeLikelihoodOffset + right*nodeOffset;
    double*         p_node  = partialLikelihoods + activeLikelihood[node_index]*activeLikelihoodOffset + node_index*nodeOffset;

    size_t n = num_chars - 1;

    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < num_site_rates; ++mixture)
    {
        // the transition probability matrix for this mixture category
        const TransitionProbabilityMatrix&    pij = this->transition_prob_matrices[mixture];

        // get the pointers to the likelihood for this mixture category
        double*          p_site_mixture          = p_node;
        const double*    p_site_mixture_left     = p_left;
        const double*    p_site_mixture_right    = p_right;
        // compute the per site probabilities
        for (size_t site = 0; site < pattern_block_size ; ++site)
        {
            p_site_mixture[n] = p_site_mixture_left[n] * p_site_mixture_right[n];

            for(size_t from = 0; from < n; from++)
            {
                p_site_mixture[from] = 0.0;

                for(size_t to = 0; to < n + 1; to++)
                {
                    p_site_mixture[from] += p_site_mixture_left[to] * p_site_mixture_right[to] * pij[from][to] ;
                }
            }

            p_site_mixture[n + 1] = 0.0;

            if(p_site_mixture[n] == 0)
            {
                for(size_t c = 0; c < n; c++)
                {
                    p_site_mixture[n + 1] += p_site_mixture_left[c] * p_site_mixture_right[c] * f[c];
                }

                p_site_mixture[n + 1] *= integrationFactors[mixture];
            }

            if( !RbSettings::userSettings().getUseScaling() )
            {
                p_site_mixture[n + 1] += ( p_site_mixture_left[n] > 0 ) * p_site_mixture_right[n + 1] + ( p_site_mixture_right[n] > 0 ) * p_site_mixture_left[n + 1];
            }

            // increment the pointers to the next site
            p_site_mixture_left+=siteOffset; p_site_mixture_right+=siteOffset; p_site_mixture+=siteOffset;

        } // end-for over all sites (=patterns)

        p_left+=mixtureOffset; p_right+=mixtureOffset; p_node+=mixtureOffset;
    } // end-for over all mixtures (=rate-categories)

    computeInternalNodeCorrection(node, node_index, left, right);
}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::computeInternalNodeLikelihood(const TopologyNode &node, size_t node_index, size_t left, size_t right, size_t middle)
{

    // compute the transition probability matrix
    updateTransitionProbabilities( node_index, node.getBranchLength() );

    // get the root frequencies
    const std::vector<double> &f = getStationaryFrequencies(node_index);

    // get the pointers to the partial likelihoods for this node and the two descendant subtrees
    const double*   p_left      = partialLikelihoods + activeLikelihood[left]*activeLikelihoodOffset + left*nodeOffset;
    const double*   p_middle    = partialLikelihoods + activeLikelihood[middle]*activeLikelihoodOffset + middle*nodeOffset;
    const double*   p_right     = partialLikelihoods + activeLikelihood[right]*activeLikelihoodOffset + right*nodeOffset;
    double*         p_node      = partialLikelihoods + activeLikelihood[node_index]*activeLikelihoodOffset + node_index*nodeOffset;

    size_t n = num_chars - 1;

    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < num_site_rates; ++mixture)
    {
        // the transition probability matrix for this mixture category
        const TransitionProbabilityMatrix&    pij = this->transition_prob_matrices[mixture];

        // get the pointers to the likelihood for this mixture category
        double*          p_site_mixture          = p_node;
        const double*    p_site_mixture_left     = p_left;
        const double*    p_site_mixture_middle   = p_middle;
        const double*    p_site_mixture_right    = p_right;
        // compute the per site probabilities
        for (size_t site = 0; site < pattern_block_size ; ++site)
        {
            for(size_t from = 0; from < n; from++)
            {
                p_site_mixture[from] = 0.0;

                for(size_t to = 0; to < n + 1; to++)
                {
                    p_site_mixture[from] += p_site_mixture_left[to] * p_site_mixture_right[to] * p_site_mixture_middle[to] * pij[from][to];
                }
            }

            p_site_mixture[n] = p_site_mixture_left[n] * p_site_mixture_right[n] * p_site_mixture_middle[n];
            p_site_mixture[n + 1] = 0.0;

            if(p_site_mixture[n] == 0)
            {
                for(size_t c = 0; c < n; c++)
                {
                    p_site_mixture[n + 1] += p_site_mixture_left[c] * p_site_mixture_right[c] * p_site_mixture_middle[c] * f[c];
                }

                p_site_mixture[n + 1] *= integrationFactors[mixture];
            }

            if( !RbSettings::userSettings().getUseScaling() )
            {
                p_site_mixture[n + 1] += ( p_site_mixture_left[n] > 0 )   * ( p_site_mixture_middle[n] > 0 ) * p_site_mixture_right[n + 1]
                                       + ( p_site_mixture_left[n] > 0 )   * ( p_site_mixture_right[n] > 0 )  * p_site_mixture_middle[n + 1]
                                       + ( p_site_mixture_middle[n] > 0 ) * ( p_site_mixture_right[n] > 0 )  * p_site_mixture_left[n + 1];
            }

            // increment the pointers to the next site
            p_site_mixture_left+=siteOffset; p_site_mixture_middle+=siteOffset; p_site_mixture_right+=siteOffset; p_site_mixture+=siteOffset;

        } // end-for over all sites (=patterns)

        p_left+=mixtureOffset; p_middle+=mixtureOffset; p_right+=mixtureOffset; p_node+=mixtureOffset;

    } // end-for over all mixtures (=rate-categories)

    computeInternalNodeCorrection(node, node_index, left, right, middle);
}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::computeTipLikelihood(const TopologyNode &node, size_t node_index)
{

    double* p_node = partialLikelihoods + activeLikelihood[node_index]*activeLikelihoodOffset + node_index*nodeOffset;

    const std::vector<bool> &gap_node = gap_matrix[node_index];
    const std::vector<unsigned long> &char_node = char_matrix[node_index];
    const std::vector<RbBitSet> &amb_char_node = this->ambiguous_char_matrix[node_index];

    // compute the transition probabilities
    updateTransitionProbabilities( node_index, node.getBranchLength() );

    // get the root frequencies
    const std::vector<double> &f = getStationaryFrequencies(node_index);

    double*   p_mixture      = p_node;

    size_t n = num_chars - 1;

    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < num_site_rates; ++mixture)
    {
        // the transition probability matrix for this mixture category
        const TransitionProbabilityMatrix&    pij = this->transition_prob_matrices[mixture];

        // get the pointer to the likelihoods for this site and mixture category
        double*     p_site_mixture      = p_mixture;

        // iterate over all sites
        for (size_t site = 0; site != pattern_block_size; ++site)
        {

            // is this site a gap?
            if ( gap_node[site] )
            {
                for(size_t c = 0; c < n + 1; c++)
                {
                    p_site_mixture[c] = 1.0;
                }

                p_site_mixture[n + 1] = integrationFactors[mixture];
            }
            else // we have observed a character
            {

                if ( this->using_ambiguous_characters == true )
                {
                    // compute the likelihood that we had a transition from state c1 to the observed state org_val
                    // note, the observed state could be ambiguous!
                    const RbBitSet &val = amb_char_node[site];

                    for(size_t c = 0; c < n + 1; c++)
                    {
                        double tmp = 0.0;

                        for( size_t i=0; i<val.size(); ++i )
                        {
                            // check whether we observed this state
                            if ( val.isSet(i) == true )
                            {
                                tmp += pij[c][i];
                            }
                        } // end-while over all observed states for this character

                        // store the likelihood
                        p_site_mixture[c] = tmp;
                    }

                    p_site_mixture[n + 1] = 0.0;

                    for( size_t i=1; i<val.size(); ++i )
                    {
                        if ( val.isSet(i) == true )
                        {
                            // check whether we observed this state
                            p_site_mixture[n + 1] += f[i];
                        }
                    } // end-while over all observed states for this character

                    p_site_mixture[n + 1] *= integrationFactors[mixture];

                }
                else
                {
                    // get the original character
                    // shift the characters so that a zero state is the (n-1)th state
                    unsigned long org_val = char_node[site] == 0 ? n : char_node[site] - 1;

                    // store the branch likelihoods and integrated node likelihood
                    for(size_t c = 0; c < n + 1; c++)
                    {
                        p_site_mixture[c] = pij[c][org_val];
                    }

                    p_site_mixture[n + 1] = (org_val == n) ? 0.0 : f[org_val] * integrationFactors[mixture];
                }


            } // end-if a gap state

            // increment the pointers to next site
            p_site_mixture+=siteOffset;

        } // end-for over all sites/patterns in the sequence

        // increment the pointers to next mixture category
        p_mixture+=mixtureOffset;

    } // end-for over all mixture categories

    computeTipCorrection(node, node_index);
}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::computeTipCorrection(const TopologyNode &node, size_t node_index)
{
    std::vector<double>::iterator p_node = correctionLikelihoods.begin() + this->activeLikelihood[node_index]*this->activeCorrectionOffset + node_index*correctionNodeOffset;
    std::vector<double>::iterator c_node = perMaskMixtureCorrections.begin() + this->activeLikelihood[node_index]*activeMassOffset + node_index*massNodeOffset;

    std::vector<std::vector<std::vector<double> > > partialNodeCorrections = std::vector<std::vector<std::vector<double> > >(num_chars, std::vector<std::vector<double> >(num_chars, std::vector<double>(numCorrectionPatterns, 0.0)));

    // iterate over correction masks
    for(size_t mask = 0; mask < numCorrectionMasks; mask++)
    {
        bool gap = correctionMaskMatrix[mask][node_index];

        maskNodeObservationCounts[mask][node_index] = !gap;

        std::vector<double>::iterator                 c_mask_mixture = c_node   + num_site_rates*mask;

        // iterate over all mixture categories
        for (size_t mixture = 0; mixture < this->num_site_rates; ++mixture)
        {
            const TransitionProbabilityMatrix&    pij = this->transition_prob_matrices[mixture];

            // iterate over ancestral (non-autapomorphic) states
            for(size_t a = 0; a < num_chars; a++)
            {
                size_t offset = mixture*correctionMixtureOffset + mask*correctionMaskOffset + a*correctionOffset;

                std::vector<double>::iterator                 u = p_node   + offset;

                // iterate over constant/autapomorphic states
                for(size_t i = 0; i < num_chars; i++)
                {
                    if(i > 0 && numCorrectionPatterns == 1)
                        break;

                    // get bit pattern for this constant/autapomorphic state
                    size_t c = ( i == 0 ? 0 : pow(2, i - 1) );

                    std::vector<double>::iterator         uc = u  + c*num_chars;

                    // if we have a gap, then fill with ones for constant patterns
                    // or zero for other patterns
                    if(gap)
                    {
                        std::fill(uc, uc + num_chars, i == 0);
                    }
                    // if this is a constant pattern, fill with transition prob from ci to a
                    else if(c == 0)
                    {
                        // iterate over initial states
                        for(size_t ci = 0; ci < num_chars; ci++)
                        {
                            uc[ci] = pij[ci][a];
                        }

                        partialNodeCorrections[a][a][c] = 1.0;
                    }
                    // if this is an autapomorphic state cj, fill with transition prob from ci to cj
                    else
                    {
                        // get autamorphic state
                        size_t cj = (i-1 >= a ? i : i-1);

                        // iterate over initial states
                        for(size_t ci = 0; ci < num_chars; ci++)
                        {
                            uc[ci] = pij[ci][cj];
                        }

                        partialNodeCorrections[cj][a][c] = 1.0;
                    }
                }
            }

            c_mask_mixture[mixture] = computeIntegratedNodeCorrection(partialNodeCorrections, node_index, mask, mixture);
        }
    }
}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::computeInternalNodeCorrection(const TopologyNode &node, size_t node_index, size_t left, size_t right, size_t middle)
{
    // get the pointers to the partial likelihoods for this node and the two descendant subtrees
    std::vector<double>::const_iterator   p_left   = correctionLikelihoods.begin() + this->activeLikelihood[left]*activeCorrectionOffset + left*correctionNodeOffset;
    std::vector<double>::const_iterator   p_right  = correctionLikelihoods.begin() + this->activeLikelihood[right]*activeCorrectionOffset + right*correctionNodeOffset;
    std::vector<double>::const_iterator   p_middle = correctionLikelihoods.begin() + this->activeLikelihood[middle]*activeCorrectionOffset + middle*correctionNodeOffset;
    std::vector<double>::iterator         p_node   = correctionLikelihoods.begin() + this->activeLikelihood[node_index]*activeCorrectionOffset + node_index*correctionNodeOffset;

    std::vector<double>::iterator c_node   = perMaskMixtureCorrections.begin() + this->activeLikelihood[node_index]*activeMassOffset + node_index*massNodeOffset;

    std::vector<std::vector<std::vector<double> > > partialNodeCorrections = std::vector<std::vector<std::vector<double> > >(num_chars, std::vector<std::vector<double> >(num_chars, std::vector<double>(numCorrectionPatterns, 0.0)));

    // iterate over correction masks
    for(size_t mask = 0; mask < numCorrectionMasks; mask++)
    {
        maskNodeObservationCounts[mask][node_index] = maskNodeObservationCounts[mask][left] + maskNodeObservationCounts[mask][right] + maskNodeObservationCounts[mask][middle];

        std::vector<double>::iterator                 c_mask_mixture = c_node   + num_site_rates*mask;

        // iterate over all mixture categories
        for (size_t mixture = 0; mixture < this->num_site_rates; ++mixture)
        {
            const TransitionProbabilityMatrix&    pij = this->transition_prob_matrices[mixture];

            // iterate over ancestral (non-autapomorphic) states
            for(size_t a = 0; a < this->num_chars; a++)
            {
                size_t offset = mixture*correctionMixtureOffset + mask*correctionMaskOffset + a*correctionOffset;

                std::vector<double>::iterator                 u = p_node   + offset;
                std::vector<double>::const_iterator         u_l = p_left   + offset;
                std::vector<double>::const_iterator         u_r = p_right  + offset;
                std::vector<double>::const_iterator         u_m = p_middle + offset;

                // iterate over combinations of autapomorphic states
                for(size_t c = 0; c < numCorrectionPatterns; c++)
                {
                    std::vector<double>::iterator         uc = u  + c*this->num_chars;

                    std::fill(uc, uc + this->num_chars, 0.0);

                    // iterate over partitions of c
                    for(size_t p1 = 0; p1 <= c; p1++)
                    {
                        if( (p1 | c) == c)
                        {
                            size_t p_tmp = p1 ^ c;

                            // iterate over partitions of p_tmp
                            for(size_t p2 = 0; p2 <= p_tmp; p2++)
                            {
                                if( (p2 | p_tmp) == p_tmp)
                                {
                                    size_t p3 = p2 ^ p_tmp;

                                    std::vector<double>::const_iterator         lc = u_l  + p1*this->num_chars;
                                    std::vector<double>::const_iterator         rc = u_r  + p2*this->num_chars;
                                    std::vector<double>::const_iterator         mc = u_m  + p3*this->num_chars;

                                    // iterate over ending states
                                    for(size_t cj = 0; cj < this->num_chars; cj++)
                                    {
                                        double pj = lc[cj] * rc[cj] * mc[cj];

                                        partialNodeCorrections[cj][a][c] += pj;

                                        // iterate over initial states
                                        for(size_t ci = 0; ci < this->num_chars; ci++)
                                        {
                                            uc[ci] += pij[ci][cj] * pj;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            c_mask_mixture[mixture] = computeIntegratedNodeCorrection(partialNodeCorrections, node_index, mask, mixture);
        }
    }
}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::computeInternalNodeCorrection(const TopologyNode &node, size_t node_index, size_t left, size_t right)
{
    // get the pointers to the partial likelihoods for this node and the two descendant subtrees
    std::vector<double>::const_iterator   p_left   = correctionLikelihoods.begin() + this->activeLikelihood[left]*activeCorrectionOffset + left*correctionNodeOffset;
    std::vector<double>::const_iterator   p_right  = correctionLikelihoods.begin() + this->activeLikelihood[right]*activeCorrectionOffset + right*correctionNodeOffset;
    std::vector<double>::iterator         p_node   = correctionLikelihoods.begin() + this->activeLikelihood[node_index]*activeCorrectionOffset + node_index*correctionNodeOffset;

    std::vector<double>::iterator c_node   = perMaskMixtureCorrections.begin() + this->activeLikelihood[node_index]*activeMassOffset + node_index*massNodeOffset;

    std::vector<std::vector<std::vector<double> > > partialNodeCorrections = std::vector<std::vector<std::vector<double> > >(num_chars, std::vector<std::vector<double> >(num_chars, std::vector<double>(numCorrectionPatterns, 0.0)));

    // iterate over correction masks
    for(size_t mask = 0; mask < numCorrectionMasks; mask++)
    {
        maskNodeObservationCounts[mask][node_index] = maskNodeObservationCounts[mask][left] + maskNodeObservationCounts[mask][right];

        std::vector<double>::iterator                 c_mask_mixture = c_node   + num_site_rates*mask;

        // iterate over all mixture categories
        for (size_t mixture = 0; mixture < this->num_site_rates; ++mixture)
        {
            const TransitionProbabilityMatrix&    pij = this->transition_prob_matrices[mixture];

            // iterate over ancestral (non-autapomorphic) states
            for(size_t a = 0; a < this->num_chars; a++)
            {
                size_t offset = mixture*correctionMixtureOffset + mask*correctionMaskOffset + a*correctionOffset;

                std::vector<double>::iterator                 u = p_node   + offset;
                std::vector<double>::const_iterator         u_l = p_left   + offset;
                std::vector<double>::const_iterator         u_r = p_right  + offset;

                // iterate over combinations of autapomorphic states
                for(size_t c = 0; c < numCorrectionPatterns; c++)
                {
                    std::vector<double>::iterator         uc = u  + c*this->num_chars;

                    std::fill(uc, uc + this->num_chars, 0.0);

                    // iterate over partitions of c
                    for(size_t p1 = 0; p1 <= c; p1++)
                    {
                        if( (p1 | c) == c)
                        {
                            size_t p2 = p1 ^ c;

                            std::vector<double>::const_iterator         lc = u_l  + p1*this->num_chars;
                            std::vector<double>::const_iterator         rc = u_r  + p2*this->num_chars;

                            // iterate over ending states
                            for(size_t cj = 0; cj < this->num_chars; cj++)
                            {
                                double pj = lc[cj] * rc[cj];

                                partialNodeCorrections[cj][a][c] += pj;

                                // iterate over initial states
                                for(size_t ci = 0; ci < this->num_chars; ci++)
                                {
                                    uc[ci] += pij[ci][cj] * pj;
                                }
                            }
                        }
                    }
                }
            }

            c_mask_mixture[mixture] = computeIntegratedNodeCorrection(partialNodeCorrections, node_index, mask, mixture);
        }
    }
}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::computeRootCorrection( size_t root, size_t left, size_t right, size_t middle)
{
    const std::vector<double> &f = this->getRootFrequencies();

    // get the pointers to the partial likelihoods for this node and the two descendant subtrees
    std::vector<double>::iterator         p_node   = correctionLikelihoods.begin() + this->activeLikelihood[root]*activeCorrectionOffset + root*correctionNodeOffset;
    std::vector<double>::const_iterator   p_left   = correctionLikelihoods.begin() + this->activeLikelihood[left]*activeCorrectionOffset + left*correctionNodeOffset;
    std::vector<double>::const_iterator   p_right  = correctionLikelihoods.begin() + this->activeLikelihood[right]*activeCorrectionOffset + right*correctionNodeOffset;
    std::vector<double>::const_iterator   p_middle = correctionLikelihoods.begin() + this->activeLikelihood[middle]*activeCorrectionOffset + middle*correctionNodeOffset;

    std::vector<double>::iterator c_node   = perMaskMixtureCorrections.begin() + this->activeLikelihood[root]*activeMassOffset + root*massNodeOffset;

    std::vector<std::vector<std::vector<double> > > partialNodeCorrections = std::vector<std::vector<std::vector<double> > >(num_chars, std::vector<std::vector<double> >(num_chars, std::vector<double>(numCorrectionPatterns, 0.0)));

    // iterate over correction masks
    for(size_t mask = 0; mask < numCorrectionMasks; mask++)
    {
        maskNodeObservationCounts[mask][root] = maskNodeObservationCounts[mask][left] + maskNodeObservationCounts[mask][right] + maskNodeObservationCounts[mask][middle];

        std::vector<double>::iterator                 c_mask_mixture = c_node   + num_site_rates*mask;

        // iterate over all mixture categories
        for (size_t mixture = 0; mixture < this->num_site_rates; ++mixture)
        {
            // iterate over ancestral (non-autapomorphic) states
            for(size_t a = 0; a < this->num_chars; a++)
            {
                size_t offset = mixture*correctionMixtureOffset + mask*correctionMaskOffset + a*correctionOffset;

                std::vector<double>::iterator                 u = p_node   + offset;
                std::vector<double>::const_iterator         u_l = p_left   + offset;
                std::vector<double>::const_iterator         u_r = p_right  + offset;
                std::vector<double>::const_iterator         u_m = p_middle + offset;

                // iterate over combinations of autapomorphic states
                for(size_t c = 0; c < numCorrectionPatterns; c++)
                {
                    std::vector<double>::iterator         uc = u  + c*this->num_chars;

                    std::fill(uc, uc + this->num_chars, 0.0);

                    // iterate over partitions of c
                    for(size_t p1 = 0; p1 <= c; p1++)
                    {
                        if( (p1 | c) == c)
                        {
                            size_t p_tmp = p1 ^ c;

                            // iterate over partitions of p_tmp
                            for(size_t p2 = 0; p2 <= p_tmp; p2++)
                            {
                                if( (p2 | p_tmp) == p_tmp)
                                {
                                    size_t p3 = p2 ^ p_tmp;

                                    std::vector<double>::const_iterator         lc = u_l  + p1*this->num_chars;
                                    std::vector<double>::const_iterator         rc = u_r  + p2*this->num_chars;
                                    std::vector<double>::const_iterator         mc = u_m  + p3*this->num_chars;

                                    // iterate over ending states
                                    for(size_t cj = 0; cj < this->num_chars - 1; cj++)
                                    {
                                        double pj = lc[cj] * rc[cj] * mc[cj];

                                        partialNodeCorrections[cj][a][c] += pj;

                                        uc[cj] += f[cj] * pj;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            c_mask_mixture[mixture] = computeIntegratedNodeCorrection(partialNodeCorrections, root, mask, mixture);
        }
    }
}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::computeRootCorrection( size_t root, size_t left, size_t right)
{
    const std::vector<double> &f = this->getRootFrequencies();

    // get the pointers to the partial likelihoods for this node and the two descendant subtrees
    std::vector<double>::iterator         p_node   = correctionLikelihoods.begin() + this->activeLikelihood[root]*activeCorrectionOffset + root*correctionNodeOffset;
    std::vector<double>::const_iterator   p_left   = correctionLikelihoods.begin() + this->activeLikelihood[left]*activeCorrectionOffset + left*correctionNodeOffset;
    std::vector<double>::const_iterator   p_right  = correctionLikelihoods.begin() + this->activeLikelihood[right]*activeCorrectionOffset + right*correctionNodeOffset;

    std::vector<double>::iterator c_node   = perMaskMixtureCorrections.begin() + this->activeLikelihood[root]*activeMassOffset + root*massNodeOffset;

    std::vector<std::vector<std::vector<double> > > partialNodeCorrections = std::vector<std::vector<std::vector<double> > >(num_chars, std::vector<std::vector<double> >(num_chars, std::vector<double>(numCorrectionPatterns, 0.0)));

    // iterate over correction masks
    for(size_t mask = 0; mask < numCorrectionMasks; mask++)
    {
        maskNodeObservationCounts[mask][root] = maskNodeObservationCounts[mask][left] + maskNodeObservationCounts[mask][right];

        std::vector<double>::iterator                 c_mask_mixture = c_node   + num_site_rates*mask;

        // iterate over all mixture categories
        for (size_t mixture = 0; mixture < this->num_site_rates; ++mixture)
        {
            // iterate over ancestral (non-autapomorphic) states
            for(size_t a = 0; a < this->num_chars; a++)
            {
                size_t offset = mixture*correctionMixtureOffset + mask*correctionMaskOffset + a*correctionOffset;

                std::vector<double>::iterator                 u = p_node   + offset;
                std::vector<double>::const_iterator         u_l = p_left   + offset;
                std::vector<double>::const_iterator         u_r = p_right  + offset;

                // iterate over combinations of autapomorphic states
                for(size_t c = 0; c < numCorrectionPatterns; c++)
                {
                    std::vector<double>::iterator         uc = u  + c*this->num_chars;

                    std::fill(uc, uc + this->num_chars, 0.0);

                    // iterate over partitions of c
                    for(size_t p1 = 0; p1 <= c; p1++)
                    {
                        if( (p1 | c) == c)
                        {
                            size_t p2 = p1 ^ c;

                            // iterate over partitions of p_tmp
                            std::vector<double>::const_iterator         lc = u_l  + p1*this->num_chars;
                            std::vector<double>::const_iterator         rc = u_r  + p2*this->num_chars;

                            // iterate over ending states
                            for(size_t cj = 0; cj < this->num_chars - 1; cj++)
                            {
                                double pj = lc[cj] * rc[cj];

                                partialNodeCorrections[cj][a][c] += pj;

                                uc[cj] += f[cj] * pj;
                            }
                        }
                    }
                }
            }

            c_mask_mixture[mixture] = computeIntegratedNodeCorrection(partialNodeCorrections, root, mask, mixture);
        }
    }
}


double RevBayesCore::PhyloCTMCSiteHomogeneousDollo::computeIntegratedNodeCorrection(const std::vector<std::vector<std::vector<double> > >& partials, size_t node_index, size_t mask, size_t mixture )
{
    /*
     * partials[a][b][c] contains the probability of observing:
     * autapomorphy pattern c
     * on background state b
     * conditioned on node state a
     */

    double prob = 0.0;

    // get the root frequencies
    const std::vector<double> &f = getStationaryFrequencies(node_index);

    // iterate over background (non-autapomorphic) states
    for(size_t a = 0; a < this->num_chars; a++)
    {
        // iterate over combinations of autapomorphic states
        for(size_t c = 0; c < numCorrectionPatterns; c++)
        {
            if(c == 0 && (coding == DolloAscertainmentBias::VARIABLE || (a == num_chars - 1 && coding == DolloAscertainmentBias::NOABSENCESITES) ))
            {
                // iterate over initial states
                for(size_t ci = 0; ci < this->num_chars - 1; ci++)
                {
                    prob += partials[ci][a][c] * f[ci];
                }
            }
            else if(coding == DolloAscertainmentBias::INFORMATIVE)
            {
                // get number of autapomorphic states
                size_t popcount = std::bitset<sizeof(size_t)*CHAR_BIT>(c).count();

               /* if the number of observations is <= the number of autapomorphies then
                * 1. don't count patterns with zero prob (num obs < num auto)
                * 2. don't double-count patterns (num obs == num auto)
                *
                * if num obs == num auto + 1
                * then don't double count the all-autapomorphies pattern
                */
                if( (maskNodeObservationCounts[mask][node_index] == maskObservationCounts[mask] &&
                    ((maskNodeObservationCounts[mask][node_index] == popcount + 1 && a == 0) ||
                     maskNodeObservationCounts[mask][node_index] > popcount + 1))
                        ||
                    (maskNodeObservationCounts[mask][node_index] != maskObservationCounts[mask] &&
                    ((maskNodeObservationCounts[mask][node_index] == popcount + 1 && a == 0) ||
                     (maskNodeObservationCounts[mask][node_index] > popcount + 1 && a == num_chars - 1)))
                     )
                {
                    // iterate over initial states
                    for(size_t ci = 0; ci < this->num_chars - 1; ci++)
                    {
                        prob += partials[ci][a][c] * f[ci];
                    }
                }
            }
        }
    }

    // impose a per-mixture boundary
    if(prob < 0.0 || prob > 1.0)
    {
        prob = RbConstants::Double::nan;
    }

    if(prob == 0.0 && !this->tau->getValue().getNode(node_index).isTip())
    {
        //prob = RbConstants::Double::nan;
    }

    prob = integrationFactors[mixture]*(1.0 - prob);

    return prob;
}

double RevBayesCore::PhyloCTMCSiteHomogeneousDollo::sumRootLikelihood( void )
{
    // get the root node
    const TopologyNode &root = tau->getValue().getRoot();

    // get the index of the root node
    size_t root_index = root.getIndex();

    const double*   p_root  = this->partialLikelihoods + this->activeLikelihood[root_index] * this->activeLikelihoodOffset + root_index*nodeOffset;

    // create a vector for the per mixture likelihoods
    // we need this vector to sum over the different mixture likelihoods
    std::vector<double> per_mixture_Likelihoods = std::vector<double>(pattern_block_size,0.0);

    const double*   p_site_root = p_root;

    // iterate over all mixture categories
    for (size_t site = 0; site < pattern_block_size; ++site)
    {
        if ( RbSettings::userSettings().getUseScaling() )
        {
            std::vector<double> integrated_likelihoods;

            double max = getScaledNodeWeights(root, site, integrated_likelihoods);

            per_mixture_Likelihoods[site] = RbMath::log_sum_exp(integrated_likelihoods, max);
        }
        else
        {
            const double*   p_site_mixture_root = p_site_root;

            for (size_t mixture = 0; mixture < this->num_site_rates; ++mixture)
            {
                per_mixture_Likelihoods[site] += p_site_mixture_root[num_chars];

                p_site_mixture_root += mixtureOffset;
            } // end-for over all sites (=patterns)

            p_site_root += siteOffset;
        }
    } // end-for over all mixtures (=rate categories)

    // sum the log-likelihoods for all sites together
    double sumPartialProbs = 0.0;

    std::vector< size_t >::const_iterator patterns = this->pattern_counts.begin();
    for (size_t site = 0; site < pattern_block_size; ++site, ++patterns)
    {
        if ( RbSettings::userSettings().getUseScaling() )
        {
            sumPartialProbs += ( per_mixture_Likelihoods[site] - log(this->num_site_rates ) ) * *patterns;
        }
        else
        {
            sumPartialProbs += log( per_mixture_Likelihoods[site] / this->num_site_rates ) * *patterns;
        }

    }


#ifdef RB_MPI

    // we only need to send message if there is more than one process
    if ( num_processes > 1 )
    {

        // send the likelihood from the helpers to the master
        if ( process_active == false )
        {
            // send from the workers the log-likelihood to the master
            MPI::COMM_WORLD.Send(&sumPartialProbs, 1, MPI::DOUBLE, active_PID, 0);
        }

        // receive the likelihoods from the helpers
        if ( process_active == true )
        {
            for (size_t i=active_PID+1; i<active_PID+num_processes; ++i)
            {
                double tmp = 0;
                MPI::COMM_WORLD.Recv(&tmp, 1, MPI::DOUBLE, int(i), 0);
                sumPartialProbs += tmp;
            }
        }

        // now send back the combined likelihood to the helpers
        if ( process_active == true )
        {
            for (size_t i=active_PID+1; i<active_PID+num_processes; ++i)
            {
                MPI::COMM_WORLD.Send(&sumPartialProbs, 1, MPI::DOUBLE, int(i), 0);
            }
        }
        else
        {
            MPI::COMM_WORLD.Recv(&sumPartialProbs, 1, MPI::DOUBLE, active_PID, 0);
        }
    
    }

#endif

    std::vector<double> perMaskCorrections = std::vector<double>(numCorrectionMasks, 0.0);

    // iterate over each correction mask
    for(size_t mask = 0; mask < numCorrectionMasks; mask++)
    {
        for (size_t node = 0; node < num_nodes; ++node)
        {
            std::vector<double>::iterator c_node_mixture   = perMaskMixtureCorrections.begin() + this->activeLikelihood[node]*activeMassOffset + node*massNodeOffset;

            // iterate over all mixture categories
            for (size_t mixture = 0; mixture < num_site_rates; ++mixture)
            {
                perMaskCorrections[mask] += c_node_mixture[mixture];
            }
        }
        
        if(perMaskCorrections[mask] <= 0.0)
            perMaskCorrections[mask] = RbConstants::Double::nan;

        perMaskCorrections[mask] = log(perMaskCorrections[mask]) - log(num_site_rates);

        // apply the correction for this correction mask
        sumPartialProbs -= perMaskCorrections[mask]*correctionMaskCounts[mask];
    }

    // num_sites is poisson
    sumPartialProbs -= log(num_sites);

    return sumPartialProbs;
}

double RevBayesCore::PhyloCTMCSiteHomogeneousDollo::getScaledNodeWeights(const TopologyNode &node, size_t pattern, std::vector<double>& weights)
{
    //reset the per site likelihood
    double max = RbConstants::Double::neginf;

    size_t node_index = node.getIndex();

    const double* p_node  = partialLikelihoods + activeLikelihood[node_index] * activeLikelihoodOffset  + node_index*nodeOffset + pattern*siteOffset;

    double logScalingFactor = perNodeSiteLogScalingFactors[activeLikelihood[node_index]][node_index][pattern];

    //otherwise, it is an ancestral node so we add the integrated likelihood
    for (size_t mixture = 0; mixture < num_site_rates; ++mixture)
    {
        double prob = log(p_node[num_chars]) - logScalingFactor;

        max = std::max(prob, max);

        weights.push_back( prob );

        p_node += mixtureOffset;
    }

    if(node.isTip())
        return max;

    std::vector<TopologyNode*> children = node.getChildren();

    TopologyNode* child;
    size_t num_with_descendants = 0;

    for(size_t i = 0; i < children.size(); i++)
    {
        size_t child_index = children[i]->getIndex();
        const double* p_child  = partialLikelihoods + activeLikelihood[child_index]  * activeLikelihoodOffset + child_index*nodeOffset  + pattern*siteOffset;

        // does this child have descendants?
        if(p_child[num_chars - 1] == 0)
        {
            child = children[i];
            num_with_descendants++;
        }
    }

    //if more than one child has observed descendants, then this is the last ancestral node
    if(num_with_descendants == 1)
    {
        max = std::max(max, getScaledNodeWeights(*child, pattern, weights));
    }

    return max;
}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::scale( size_t node_index)
{

    double* p_node = this->partialLikelihoods + this->activeLikelihood[node_index]*this->activeLikelihoodOffset + node_index*this->nodeOffset;

    if ( RbSettings::userSettings().getUseScaling() == true && node_index % RbSettings::userSettings().getScalingDensity() == 0 )
    {
        // iterate over all mixture categories
        for (size_t site = 0; site < this->pattern_block_size ; ++site)
        {

            // the max probability
            double max = 0.0;

            // compute the per site probabilities
            for (size_t mixture = 0; mixture < this->num_site_rates; ++mixture)
            {
                // get the pointers to the likelihood for this mixture category
                size_t offset = mixture*this->mixtureOffset + site*this->siteOffset;

                double*          p_site_mixture          = p_node + offset;

                for ( size_t i=0; i<num_chars-1; ++i)
                {
                    if ( p_site_mixture[i] > max )
                    {
                        max = p_site_mixture[i];
                    }
                }

            }

            this->perNodeSiteLogScalingFactors[this->activeLikelihood[node_index]][node_index][site] = -log(max);


            // compute the per site probabilities
            for (size_t mixture = 0; mixture < this->num_site_rates; ++mixture)
            {
                // get the pointers to the likelihood for this mixture category
                size_t offset = mixture*this->mixtureOffset + site*this->siteOffset;

                double*          p_site_mixture          = p_node + offset;

                for ( size_t i=0; i<num_chars; ++i)
                {
                    p_site_mixture[i] /= max;
                }

            }

        }
    }
    else if ( RbSettings::userSettings().getUseScaling() == true )
    {
        // iterate over all mixture categories
        for (size_t site = 0; site < this->pattern_block_size ; ++site)
        {
            this->perNodeSiteLogScalingFactors[this->activeLikelihood[node_index]][node_index][site] = 0;
        }

    }
}


void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::scale( size_t node_index, size_t left, size_t right )
{

    double* p_node = this->partialLikelihoods + this->activeLikelihood[node_index]*this->activeLikelihoodOffset + node_index*this->nodeOffset;

    if ( RbSettings::userSettings().getUseScaling() == true && node_index % RbSettings::userSettings().getScalingDensity() == 0 && node_index < num_nodes -1)
    {
        // iterate over all mixture categories
        for (size_t site = 0; site < this->pattern_block_size ; ++site)
        {

            // the max probability
            double max = 0.0;

            // compute the per site probabilities
            for (size_t mixture = 0; mixture < this->num_site_rates; ++mixture)
            {
                // get the pointers to the likelihood for this mixture category
                size_t offset = mixture*this->mixtureOffset + site*this->siteOffset;

                double*          p_site_mixture          = p_node + offset;

                for ( size_t i=0; i<num_chars-1; ++i)
                {
                    if ( p_site_mixture[i] > max )
                    {
                        max = p_site_mixture[i];
                    }
                }

            }

            this->perNodeSiteLogScalingFactors[this->activeLikelihood[node_index]][node_index][site] = this->perNodeSiteLogScalingFactors[this->activeLikelihood[left]][left][site] + this->perNodeSiteLogScalingFactors[this->activeLikelihood[right]][right][site] - log(max);


            // compute the per site probabilities
            for (size_t mixture = 0; mixture < this->num_site_rates; ++mixture)
            {
                // get the pointers to the likelihood for this mixture category
                size_t offset = mixture*this->mixtureOffset + site*this->siteOffset;

                double*          p_site_mixture          = p_node + offset;

                for ( size_t i=0; i<num_chars; ++i)
                {
                    p_site_mixture[i] /= max;
                }

            }

        }
    }
    else if ( RbSettings::userSettings().getUseScaling() == true )
    {
        // iterate over all mixture categories
        for (size_t site = 0; site < this->pattern_block_size ; ++site)
        {
            this->perNodeSiteLogScalingFactors[this->activeLikelihood[node_index]][node_index][site] = this->perNodeSiteLogScalingFactors[this->activeLikelihood[left]][left][site] + this->perNodeSiteLogScalingFactors[this->activeLikelihood[right]][right][site];
        }

    }
}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::scale( size_t node_index, size_t left, size_t right, size_t middle )
{

    double* p_node   = this->partialLikelihoods + this->activeLikelihood[node_index]*this->activeLikelihoodOffset + node_index*this->nodeOffset;

    if ( RbSettings::userSettings().getUseScaling() == true && node_index % RbSettings::userSettings().getScalingDensity() == 0 && node_index < num_nodes -1)
    {
        // iterate over all mixture categories
        for (size_t site = 0; site < this->pattern_block_size ; ++site)
        {

            // the max probability
            double max = 0.0;

            // compute the per site probabilities
            for (size_t mixture = 0; mixture < this->num_site_rates; ++mixture)
            {
                // get the pointers to the likelihood for this mixture category
                size_t offset = mixture*this->mixtureOffset + site*this->siteOffset;

                double*          p_site_mixture          = p_node + offset;

                for ( size_t i=0; i<num_chars-1; ++i)
                {
                    if ( p_site_mixture[i] > max )
                    {
                        max = p_site_mixture[i];
                    }
                }

            }

            this->perNodeSiteLogScalingFactors[this->activeLikelihood[node_index]][node_index][site] = this->perNodeSiteLogScalingFactors[this->activeLikelihood[left]][left][site] + this->perNodeSiteLogScalingFactors[this->activeLikelihood[right]][right][site] + this->perNodeSiteLogScalingFactors[this->activeLikelihood[middle]][middle][site] - log(max);


            // compute the per site probabilities
            for (size_t mixture = 0; mixture < this->num_site_rates; ++mixture)
            {
                // get the pointers to the likelihood for this mixture category
                size_t offset = mixture*this->mixtureOffset + site*this->siteOffset;

                double*          p_site_mixture          = p_node + offset;

                for ( size_t i=0; i<num_chars; ++i)
                {
                    p_site_mixture[i] /= max;
                }

            }

        }
    }
    else if ( RbSettings::userSettings().getUseScaling() == true )
    {
        // iterate over all mixture categories
        for (size_t site = 0; site < this->pattern_block_size ; ++site)
        {
            this->perNodeSiteLogScalingFactors[this->activeLikelihood[node_index]][node_index][site] = this->perNodeSiteLogScalingFactors[this->activeLikelihood[left]][left][site] + this->perNodeSiteLogScalingFactors[this->activeLikelihood[right]][right][site] + this->perNodeSiteLogScalingFactors[this->activeLikelihood[middle]][middle][site];
        }

    }
}

/** Swap a parameter of the distribution */
void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::swapParameterInternal(const DagNode *oldP, const DagNode *newP)
{

    if (oldP == death_rate)
    {
        death_rate = static_cast<const TypedDagNode< double >* >( newP );
    }
    else
    {
        PhyloCTMCSiteHomogeneousConditional<StandardState>::swapParameterInternal(oldP,newP);
    }

}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::redrawValue( void ) {

    // delete the old value first
    delete this->value;

    // create a new character data object
    this->value = new HomologousDiscreteCharacterData<StandardState>();

    size_t numTips = this->tau->getValue().getNumberOfTips();
    size_t num_nodes = this->tau->getValue().getNumberOfNodes();

    RandomNumberGenerator* rng = GLOBAL_RNG;

    const TopologyNode &root = this->tau->getValue().getRoot();
    size_t rootIndex = this->tau->getValue().getRoot().getIndex();

    updateCorrections(root, rootIndex);

    std::vector< DiscreteTaxonData<StandardState> > taxa = std::vector< DiscreteTaxonData<StandardState> >(numTips, DiscreteTaxonData<StandardState>( Taxon("") ) );

    std::vector<size_t> rateIndices;
    std::vector<size_t> birthNodes;

    double total = 0.0;
    for ( size_t i = 0; i < this->num_nodes; ++i )
        for ( size_t j = 0; j < this->num_site_rates; ++j )
            total += perMaskMixtureCorrections[i*massNodeOffset + j];

    for ( size_t i = 0; i < num_sites; i++ )
    {
        double u = rng->uniform01()*total;
        double tmp = 0.0;

        // simulate a birth and rate index for this character
        // by sampling nodes in proportion to the per node survival probs
        size_t birthNode = 0;
        size_t rateIndex = 0;
        while(tmp < u)
        {
            tmp += perMaskMixtureCorrections[birthNode*massNodeOffset + rateIndex];

            if(tmp < u)
            {
                rateIndex++;
                if(rateIndex == num_site_rates)
                {
                    rateIndex = 0;
                    birthNode++;
                }
            }
        }

        rateIndices.push_back(rateIndex);
        birthNodes.push_back(birthNode);
    }

    const std::vector< double > &freqs = this->getRootFrequencies();

    // then sample site-patterns using rejection sampling,
    // rejecting those that match the unobservable ones.
    for ( size_t i = 0; i < this->num_sites; i++ )
    {
        size_t rateIndex = rateIndices[i];
        size_t birthIndex = birthNodes[i];

        std::vector<StandardState> siteData(num_nodes, StandardState());

        // create the character
        StandardState &c = siteData[birthIndex];
        c.setStateByIndex(1);
        // draw the state
        double u = rng->uniform01();
        size_t stateIndex = 0;
        while ( true )
        {
            u -= freqs[stateIndex];
            ++stateIndex;

            if ( u > 0.0 && stateIndex < this->num_chars-1)
            {
                ++c;
            }
            else
            {
                break;
            }

        }

        const TopologyNode& birthNode = tau->getValue().getNode(birthIndex);

        // recursively simulate the sequences
        std::map<size_t, size_t> charCounts;
        simulate( birthNode, siteData, rateIndex, charCounts);

        if( !isSitePatternCompatible(charCounts) )
        {
            i--;
            continue;
        }

        // add the taxon data to the character data
        for (size_t t = 0; t < numTips; ++t)
        {
            taxa[t].addCharacter(siteData[t]);
        }
    }

    // add the taxon data to the character data
    for (size_t i = 0; i < this->tau->getValue().getNumberOfTips(); ++i)
    {
        taxa[i].setTaxon( this->tau->getValue().getNode(i).getTaxon() );
        this->value->addTaxonData( taxa[i] );
    }

    // compress the data and initialize internal variables
    this->reInitialized();

    for (std::vector<bool>::iterator it = this->dirty_nodes.begin(); it != this->dirty_nodes.end(); ++it)
    {
        (*it) = true;
    }

    // flip the active likelihood pointers
    for (size_t index = 0; index < this->changed_nodes.size(); ++index)
    {
        if ( this->changed_nodes[index] == false )
        {
            this->activeLikelihood[index] = (this->activeLikelihood[index] == 0 ? 1 : 0);
            this->changed_nodes[index] = true;
        }
    }

}

void RevBayesCore::PhyloCTMCSiteHomogeneousDollo::simulate( const TopologyNode &node, std::vector<StandardState> &data, size_t rateIndex, std::map<size_t, size_t>& charCounts) {

    // get the children of the node
    const std::vector<TopologyNode*>& children = node.getChildren();

    // get the sequence of this node
    size_t node_index = node.getIndex();
    StandardState &parentState = data[ node_index ];

    // simulate the sequence for each child
    RandomNumberGenerator* rng = GLOBAL_RNG;
    for (std::vector< TopologyNode* >::const_iterator it = children.begin(); it != children.end(); ++it)
    {
        const TopologyNode &child = *(*it);

        // update the transition probability matrix
        this->updateTransitionProbabilities( child.getIndex(), child.getBranchLength() );

        double u = rng->uniform01();

        if(u < survival[rateIndex])
        {
            unsigned long cp = parentState.getStateIndex() - 1;

            double *freqs = this->transition_prob_matrices[ rateIndex ][ cp ];

            // create the character
            StandardState &c = data[ child.getIndex() ];
            c.setStateByIndex(1);
            // draw the state
            double u = rng->uniform01();
            size_t stateIndex = 0;
            while ( true )
            {
                u -= *freqs;
                ++stateIndex;

                if ( u > 0.0 && stateIndex < this->num_chars -1)
                {
                    ++c;
                    ++freqs;
                }
                else
                {
                    break;
                }

            }

            if(child.isTip())
                charCounts[c.getStateIndex()]++;
            else
                simulate( child, data, rateIndex, charCounts);
        }
    }

}
