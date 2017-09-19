//
//  CladogeneticSpeciationRateMatrixFunction.h
//
//  Created by Will Freyman on 8/1/17.
//

#ifndef CladogeneticSpeciationRateMatrixFunction__
#define CladogeneticSpeciationRateMatrixFunction__

#include "AbstractCladogenicStateFunction.h"
#include "CladogeneticSpeciationRateMatrix.h"
#include "RbVector.h"
#include "TypedDagNode.h"
#include "TypedFunction.h"

#include <sstream>
#include <vector>
#include <map>

namespace RevBayesCore {
    
    class CladogeneticSpeciationRateMatrixFunction : public AbstractCladogenicStateFunction, public TypedFunction<CladogeneticSpeciationRateMatrix> {
        
    public:
        
        //CladogeneticSpeciationRateMatrixFunction( const TypedDagNode< RbVector< double > >* sr, unsigned mc );
        CladogeneticSpeciationRateMatrixFunction( const TypedDagNode< RevBayesCore::RbVector<RevBayesCore::RbVector<long> > >* events, const TypedDagNode<RevBayesCore::RbVector<double> >* spec_rates, int n_states );
        virtual                                                                         ~CladogeneticSpeciationRateMatrixFunction(void);
        
        // public member functions
        CladogeneticSpeciationRateMatrixFunction*                                       clone(void) const;
        std::map< std::vector<unsigned>, double >                                       getEventMap(double t=0.0);
        const std::map< std::vector<unsigned>, double >&                                getEventMap(double t=0.0) const;
        void                                                                            update(void);
        
    protected:
        
        void                                                                            swapParameterInternal(const DagNode *oldP, const DagNode *newP);
        
    private:
        
//        void                                                                            buildEventMap(void);
        void                                                                            updateSpeciationRates(void);
        
        // members
        const TypedDagNode< RevBayesCore::RbVector<RevBayesCore::RbVector<long> > >*     cladogenetic_events;
        std::map< std::vector<unsigned>, double >                                       event_map;
        unsigned                                                                        num_states;
        const TypedDagNode< RbVector<double> >*                                         speciation_rates;
        
    };
    
}

#endif
