/**
 * @file
 * This file contains the declaration of Distribution, which is
 * the interface and abstract base class for RevLanguage distribution objects.
 *
 * @brief Declaration of Distribution
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2012-08-03 17:51:49 +0200 (Fri, 03 Aug 2012) $
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @interface Distribution
 * @package distributions
 * @since Version 1.0, 2012-08-06
 *
 * $Id: Function.h 1734 2012-08-03 15:51:49Z hoehna $
 */

#ifndef RlContinuousDistribution_H
#define RlContinuousDistribution_H

#include "ContinuousDistribution.h"
#include "Real.h"
#include "RlTypedDistribution.h"

namespace RevLanguage {
    
    class ContinuousDistribution : public TypedDistribution<Real> {
        
    public:
        virtual                                         ~ContinuousDistribution(void);                                                                  //!< Destructor
        ContinuousDistribution(const ContinuousDistribution &x);                                                                //!< Copy constuctor
        
        virtual Real*                                   createRandomVariable(void) const;                                                   //!< Create a random variable from this distribution        
        
        // Basic utility functions you have to override
        virtual ContinuousDistribution*                 clone(void) const = 0;                                                              //!< Clone object
        static const std::string&                       getClassName(void);                                                                 //!< Get class name
        static const TypeSpec&                          getClassTypeSpec(void);                                                             //!< Get class type spec
        
        
        // Distribution functions you have to override
        virtual RevBayesCore::ContinuousDistribution*   createDistribution(void) const = 0;                                                 //!< Create a random variable from this distribution
        
        
    protected:
        ContinuousDistribution(void);                                                                                                 //!< Basic constructor
        
    };
    
}

#endif

