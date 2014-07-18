#include "ArgumentRule.h"
#include "Ellipsis.h"
#include "AbstractMemberFunction.h"
#include "RbException.h"
#include "RbUtil.h"
#include "TypeSpec.h"

#include <sstream>

using namespace RevLanguage;

/** Constructor */
AbstractMemberFunction::AbstractMemberFunction(const TypeSpec retType, ArgumentRules* argRules) : Function(), 
    argumentRules(argRules), 
    object(NULL), 
    returnType(retType) 
{
    
}


/** Execute function: call the object's internal implementation through executeOperation */
RevObject* AbstractMemberFunction::execute( void ) 
{
//    
//    RevBayesCore::TypedDagNode<RevBayesCore::TimeTree>* tau = static_cast<const TimeTree&>( this->args[0].getVariable()->getRevObject() ).getDagNode();
//    const RevBayesCore::Clade& c = static_cast<const Clade &>( this->args[1].getVariable()->getRevObject() ).getValue();
//    RevBayesCore::TmrcaStatistic* f = new RevBayesCore::TmrcaStatistic( tau, c );
//    
//    DeterministicNode<double> *detNode = new DeterministicNode<double>("", f, this->clone());
//    
//    RealPos* value = new RealPos( detNode );
    
    return object->getRevObject().executeMethod( funcName, args );
    
}


/** Get class name of object */
const std::string& AbstractMemberFunction::getClassName(void) { 
    
    static std::string rbClassName = "AbstractMemberFunction";
    
	return rbClassName; 
}

/** Get class type spec describing type of object */
const TypeSpec& AbstractMemberFunction::getClassTypeSpec(void) { 
    
    static TypeSpec rbClass = TypeSpec( getClassName(), new TypeSpec( Function::getClassTypeSpec() ) );
    
	return rbClass; 
}

/** Get type spec */
const TypeSpec& AbstractMemberFunction::getTypeSpec( void ) const {
    
    static TypeSpec typeSpec = getClassTypeSpec();
    
    return typeSpec;
}


/** Get argument rules */
const ArgumentRules& AbstractMemberFunction::getArgumentRules(void) const {
    
    return *argumentRules;
}


/** Get return type */
const TypeSpec& AbstractMemberFunction::getReturnType(void) const {
    
    return returnType;
}


void AbstractMemberFunction::setMemberObject( const RevPtr<Variable> &obj) {
    
    // we do not own the object itself because one object can have multiple member functions
    object = obj;
}



void AbstractMemberFunction::setMethodName(std::string const &name) {
 
    funcName = name;
}

