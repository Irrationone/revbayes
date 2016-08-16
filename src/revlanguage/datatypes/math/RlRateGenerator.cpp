//
//  RlRateGenerator.cpp
//  revbayes-proj
//
//  Created by Michael Landis on 3/17/15.
//  Copyright (c) 2015 Michael Landis. All rights reserved.
//

#include "ArgumentRule.h"
#include "ModelVector.h"
#include "Natural.h"
#include "Real.h"
#include "RealPos.h"
#include "RlBoolean.h"
#include "RlRateGenerator.h"
#include "RlRateMatrix.h"

using namespace RevLanguage;

RateGenerator::RateGenerator(void) : ModelObject<RevBayesCore::RateGenerator>()
{
    initMethods();
}


RateGenerator::RateGenerator( const RevBayesCore::RateGenerator &v) : ModelObject<RevBayesCore::RateGenerator>( v.clone() )
{
    initMethods();
}


RateGenerator::RateGenerator( RevBayesCore::RateGenerator *v) : ModelObject<RevBayesCore::RateGenerator>( v )
{
    initMethods();
}


RateGenerator::RateGenerator( RevBayesCore::TypedDagNode<RevBayesCore::RateGenerator> *m) : ModelObject<RevBayesCore::RateGenerator>( m )
{
    initMethods();
}


RateGenerator* RateGenerator::clone() const
{
    return new RateGenerator( *this );
}


/* Map calls to member methods */
RevPtr<RevVariable> RateGenerator::executeMethod(std::string const &name, const std::vector<Argument> &args, bool &found)
{
    
//    if (name == "[]")
//    {
//        found = true;
//        int n = 0; //(int)this->dagNode->getValue().getNumberOfStates();
//        return new RevVariable( new Natural(n) );
////        throw RbException("Currently deprecated. Blame Michael (or Sebastian)!");
//        
//    }
//    else
    if (name == "size")
    {
        found = true;
        
        int n = 0; //(int)this->dagNode->getValue().getNumberOfStates();
        return new RevVariable( new Natural(n) );
    }
    
    return ModelObject<RevBayesCore::RateGenerator>::executeMethod( name, args, found );
}


/* Get Rev type of object */
const std::string& RateGenerator::getClassType(void) {
    
    static std::string revType = "RateGenerator";
    
	return revType;
}

/* Get class type spec describing type of object */
const TypeSpec& RateGenerator::getClassTypeSpec(void) {
    
    static TypeSpec revTypeSpec = TypeSpec( getClassType(), new TypeSpec( RevObject::getClassTypeSpec() ) );
    
	return revTypeSpec;
}


/** Get the type spec of this class. We return a member variable because instances might have different element types. */
const TypeSpec& RateGenerator::getTypeSpec(void) const {
    
    static TypeSpec typeSpec = getClassTypeSpec();
    return typeSpec;
}

void RateGenerator::initMethods(void) {
    
    // add method for call "x[]" as a function
//    ArgumentRules* squareBracketArgRules = new ArgumentRules();
//    squareBracketArgRules->push_back( new ArgumentRule( "index" , Natural::getClassTypeSpec(), "The index of the row.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
//    methods.addFunction( new MemberProcedure( "[]", ModelVector<RealPos>::getClassTypeSpec(), squareBracketArgRules) );
    
    
    // add method for call "x[]" as a function
    ArgumentRules* sizeArgRules = new ArgumentRules();
    methods.addFunction( new MemberProcedure( "size", Natural::getClassTypeSpec(), sizeArgRules) );
}
