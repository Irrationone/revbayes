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

#ifndef RlPositiveContinuousDistribution_H
#define RlPositiveContinuousDistribution_H

#include "ContinuousDistribution.h"
#include "RealPos.h"
#include "RlTypedDistribution.h"

namespace RevLanguage {
    
    class PositiveContinuousDistribution : public TypedDistribution<RealPos> {
        
    public:
        virtual                                         ~PositiveContinuousDistribution(void);                                                                  //!< Destructor
        PositiveContinuousDistribution(const PositiveContinuousDistribution &x);                                                                //!< Copy constuctor
        
        virtual RealPos*                                createRandomVariable(void) const;                                                   //!< Create a random variable from this distribution        
        
        // Basic utility functions you have to override
        virtual PositiveContinuousDistribution*         clone(void) const = 0;                                                              //!< Clone object
        static const std::string&                       getClassType(void);                                                                 //!< Get class name
        static const TypeSpec&                          getClassTypeSpec(void);                                                             //!< Get class type spec
        
        
        // Distribution functions you have to override
        virtual RevBayesCore::ContinuousDistribution*   createDistribution(void) const = 0;                                                 //!< Create a random variable from this distribution
        
        
    protected:
        PositiveContinuousDistribution(void);                                                                                                 //!< Basic constructor
        
    };
    
}

#endif

