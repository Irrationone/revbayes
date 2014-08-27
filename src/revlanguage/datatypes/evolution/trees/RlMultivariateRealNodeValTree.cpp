/* 
 * File:   RlMultivariateRealNodeValTree.cpp
 * Author: nl
 * 
 * Created on 16 juillet 2014, 19:54
 */

#include "RlMultivariateRealNodeValTree.h"

#include "Natural.h"
#include "RbUtil.h"
#include "MemberProcedure.h"
#include "ModelVector.h"
#include "RlAbstractCharacterData.h"
#include "RlMemberFunction.h"
#include "RlString.h"
#include "RealPos.h"
#include "TypeSpec.h"
#include "RlString.h"

#include <sstream>

using namespace RevLanguage;

/** Default constructor */
MultivariateRealNodeValTree::MultivariateRealNodeValTree(void) : ModelObject<RevBayesCore::MultivariateRealNodeContainer>() {
    
}

/** Construct from bool */
MultivariateRealNodeValTree::MultivariateRealNodeValTree(RevBayesCore::MultivariateRealNodeContainer *t) : ModelObject<RevBayesCore::MultivariateRealNodeContainer>( t ) {
    
}

/** Construct from bool */
MultivariateRealNodeValTree::MultivariateRealNodeValTree(const RevBayesCore::MultivariateRealNodeContainer &t) : ModelObject<RevBayesCore::MultivariateRealNodeContainer>( new RevBayesCore::MultivariateRealNodeContainer( t ) ) {
    
}

/** Construct from bool */
MultivariateRealNodeValTree::MultivariateRealNodeValTree(RevBayesCore::TypedDagNode<RevBayesCore::MultivariateRealNodeContainer> *n) : ModelObject<RevBayesCore::MultivariateRealNodeContainer>( n ) {
    
}



/** Construct from bool */
MultivariateRealNodeValTree::MultivariateRealNodeValTree(const MultivariateRealNodeValTree &t) : ModelObject<RevBayesCore::MultivariateRealNodeContainer>( t ) {
    
}


/** Clone object */
MultivariateRealNodeValTree* MultivariateRealNodeValTree::clone(void) const {
    
	return new MultivariateRealNodeValTree(*this);
}


/* Map calls to member methods */
RevLanguage::RevPtr<Variable> MultivariateRealNodeValTree::executeMethod(std::string const &name, const std::vector<Argument> &args) {
    
    if (name == "newick") {        
        RevBayesCore::TypedDagNode< int >* k = static_cast<const Integer &>( args[0].getVariable()->getRevObject() ).getDagNode();
        std::string newick = this->dagNode->getValue().getNewick(k->getValue());
        return new Variable( new RlString( newick ) );
    }
    else if ( name == "clampAt" )
    {
        RevBayesCore::TypedDagNode< RevBayesCore::AbstractCharacterData >* data = static_cast<const AbstractCharacterData &>( args[0].getVariable()->getRevObject() ).getDagNode();
        RevBayesCore::TypedDagNode< int >* k = static_cast<const Integer &>( args[1].getVariable()->getRevObject() ).getDagNode();
        RevBayesCore::TypedDagNode< int >* l = static_cast<const Integer &>( args[2].getVariable()->getRevObject() ).getDagNode();
        RevBayesCore::AbstractCharacterData* d = & data->getValue();
        RevBayesCore::ContinuousCharacterData* c = static_cast<RevBayesCore::ContinuousCharacterData*>(d);
        
        this->dagNode->getValue().clampAt(c, k->getValue(), l->getValue());   
        return new Variable( new Real( 0 ) );
    }

    return ModelObject<RevBayesCore::MultivariateRealNodeContainer>::executeMethod( name, args );
}


/** Get class name of object */
const std::string& MultivariateRealNodeValTree::getClassType(void) { 
    
    static std::string revClassType = "MultivariateRealNodeValTree";
    
	return revClassType; 
}

/** Get class type spec describing type of object */
const TypeSpec& MultivariateRealNodeValTree::getClassTypeSpec(void) { 
    
    static TypeSpec revClassTypeSpec = TypeSpec( getClassType(), new TypeSpec( RevObject::getClassTypeSpec() ) );
    
	return revClassTypeSpec; 
}


/**
 * Get member methods. We construct the appropriate static member
 * function table here.
 */
const RevLanguage::MethodTable& MultivariateRealNodeValTree::getMethods( void ) const
{
    static MethodTable  myMethods   = MethodTable();
    static bool         methodsSet  = false;
    
    if ( !methodsSet )
    {
        myMethods = makeMethods();
        methodsSet = true;
    }
    
    return myMethods;
}


/** Get type spec */
const TypeSpec& MultivariateRealNodeValTree::getTypeSpec( void ) const {
    
    static TypeSpec typeSpec = getClassTypeSpec();
    
    return typeSpec;
}


/* Make member methods for this class */
RevLanguage::MethodTable MultivariateRealNodeValTree::makeMethods(void) const
{
    
    MethodTable methods = MethodTable();
    
    ArgumentRules* argRules = new ArgumentRules();
    argRules->push_back(new ArgumentRule("index", false, Natural::getClassTypeSpec()));
    
    methods.addFunction("mean", new MemberFunction<MultivariateRealNodeValTree,Real>( this, argRules ) );
    
    methods.addFunction("tipMean", new MemberFunction<MultivariateRealNodeValTree,Real>( this, argRules ) );
    
    methods.addFunction("stdev", new MemberFunction<MultivariateRealNodeValTree,RealPos>(  this, argRules ) );
    
    methods.addFunction("rootVal", new MemberFunction<MultivariateRealNodeValTree,RealPos>(  this, argRules ) );
    
    methods.addFunction("newick", new MemberProcedure(RlString::getClassTypeSpec(), argRules ) );
    
    ArgumentRules* clampArgRules = new ArgumentRules();
    clampArgRules->push_back(new ArgumentRule("data", false, AbstractCharacterData::getClassTypeSpec()));
    clampArgRules->push_back(new ArgumentRule("processIndex", false, Natural::getClassTypeSpec()));
    clampArgRules->push_back(new ArgumentRule("dataIndex", false, Natural::getClassTypeSpec()));
    methods.addFunction("clampAt", new MemberProcedure(MultivariateRealNodeValTree::getClassTypeSpec(), clampArgRules ) );
    
    // Insert inherited methods
    methods.insertInheritedMethods( ModelObject<RevBayesCore::MultivariateRealNodeContainer>::makeMethods() );
    
    return methods;
}


/** Print value for user */
void MultivariateRealNodeValTree::printValue(std::ostream &o) const {

    long previousPrecision = o.precision();
    std::ios_base::fmtflags previousFlags = o.flags();
    
    std::fixed( o );
    o.precision( 3 );
    
    dagNode->printValue( o, "" );
    
    o.setf( previousFlags );
    o.precision( previousPrecision );

}


