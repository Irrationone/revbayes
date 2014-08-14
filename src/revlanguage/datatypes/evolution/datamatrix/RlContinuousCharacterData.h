//
//  RlContinuousCharacterData.h
//  revbayes
//
//  Created by Nicolas Lartillot on 2014-03-27.
//  Copyright (c) 2014 revbayes team. All rights reserved.
//

#ifndef __revbayes__RlContinuousCharacterData__
#define __revbayes__RlContinuousCharacterData__

#include <iostream>


#include "ContinuousCharacterData.h"
#include "ModelObject.h"
#include "TypedDagNode.h"

#include <ostream>
#include <string>

namespace RevLanguage {
    
    class ContinuousCharacterData : public ModelObject<RevBayesCore::ContinuousCharacterData> {
        
    public:
        ContinuousCharacterData(void);                                                                          //!< Default constructor
        ContinuousCharacterData(const RevBayesCore::ContinuousCharacterData *d);                                //!< Copy constructor
        
        // Operators
        
        // Basic utility functions
        ContinuousCharacterData*        clone(void) const;                                                      //!< Clone object
        RevObject*                      convertTo(const TypeSpec& type) const;                                  //!< Convert to type
        static const std::string&       getClassType(void);                                                     //!< Get Rev type
        static const TypeSpec&          getClassTypeSpec(void);                                                 //!< Get class type spec
        const TypeSpec&                 getTypeSpec(void) const;                                                //!< Get language type of the object
        bool                            isConvertibleTo(const TypeSpec& type, bool once) const;                 //!< Is convertible to type?
        
        // Member method functions
        const MethodTable&              getMethods(void) const;                                                 //!< Get member methods
        
    };
    
}

#endif /* defined(__revbayes__RlContinuousCharacterData__) */
