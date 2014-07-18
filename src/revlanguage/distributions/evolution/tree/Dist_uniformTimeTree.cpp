#include "ArgumentRule.h"
#include "ArgumentRules.h"
#include "Clade.h"
#include "Dist_uniformTimeTree.h"
#include "ModelVector.h"
#include "Natural.h"
#include "OptionRule.h"
#include "Real.h"
#include "RealPos.h"
#include "RlClade.h"
#include "RlString.h"
#include "RlTimeTree.h"
#include "StochasticNode.h"
#include "UniformTimeTreeDistribution.h"

using namespace RevLanguage;


Dist_uniformTimeTree::Dist_uniformTimeTree() : TypedDistribution<TimeTree>() {
    
}


Dist_uniformTimeTree::~Dist_uniformTimeTree() {

}



Dist_uniformTimeTree* Dist_uniformTimeTree::clone( void ) const {

    return new Dist_uniformTimeTree( *this );
}


RevBayesCore::UniformTimeTreeDistribution* Dist_uniformTimeTree::createDistribution( void ) const {

    // Get the parameters
    RevBayesCore::TypedDagNode<double>* originT = static_cast<const RealPos &>( originTime->getRevObject() ).getDagNode();
    const std::vector<std::string>      &names  = static_cast<const ModelVector<RlString> &>( taxonNames->getRevObject() ).getDagNode()->getValue();

    RevBayesCore::UniformTimeTreeDistribution*   d = new RevBayesCore::UniformTimeTreeDistribution( originT, names );

    return d;
}



/* Get class name of object */
const std::string& Dist_uniformTimeTree::getClassType(void) {
    
    static std::string revType = "Dist_uniformTimeTree";
    
	return revType;
}


/* Get class type spec describing type of object. TODO: Check if the correct parent is TypedDistribution or Distribution */
const TypeSpec& Dist_uniformTimeTree::getClassTypeSpec(void) {
    
    static TypeSpec revTypeSpec = TypeSpec( getClassType(), new TypeSpec( TypedDistribution<TimeTree>::getClassTypeSpec() ) );
    
	return revTypeSpec;
}


/* Return member rules */
const MemberRules& Dist_uniformTimeTree::getMemberRules(void) const {
    
    static MemberRules distUTTDMemberRules;
    static bool rulesSet = false;
    
    if ( !rulesSet ) {

        distUTTDMemberRules.push_back( new ArgumentRule( "originTime", true, RealPos::getClassTypeSpec() ) );
        distUTTDMemberRules.push_back( new ArgumentRule( "taxonNames", true, ModelVector<RlString>::getClassTypeSpec() ) );
        
        rulesSet = true;
    }
    
    return distUTTDMemberRules;
}


const TypeSpec& Dist_uniformTimeTree::getTypeSpec( void ) const {
    
    static TypeSpec ts = getClassTypeSpec();
    
    return ts;
}


/** Set a member variable */
void Dist_uniformTimeTree::setConstMemberVariable(const std::string& name, const RevPtr<const Variable> &var) {
    
    if ( name == "originTime" ) {
        originTime = var;
    }
    else if ( name == "taxonNames" ) {
        taxonNames = var;
    }
    else {
        Distribution::setConstMemberVariable(name, var);
    }
}

