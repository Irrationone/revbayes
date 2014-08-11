//
//  Func_exp.cpp
//  RevBayesCore
//
//  Created by Sebastian Hoehna on 8/7/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#include "Func_treeAssembly.h"
#include "ModelVector.h"
#include "Real.h"
#include "RealPos.h"
#include "RlBranchLengthTree.h"
#include "RlDeterministicNode.h"
#include "RlTopology.h"
#include "Topology.h"
#include "TreeAssemblyFunction.h"
#include "TypedDagNode.h"

using namespace RevLanguage;

/** default constructor */
Func_treeAssembly::Func_treeAssembly( void ) : Function( ) {
    
}


/** Clone object */
Func_treeAssembly* Func_treeAssembly::clone( void ) const {
    
    return new Func_treeAssembly( *this );
}


RevPtr<Variable> Func_treeAssembly::execute() {
    
    RevBayesCore::TypedDagNode<RevBayesCore::Topology>* tau = static_cast<const Topology&>( this->args[0].getVariable()->getRevObject() ).getDagNode();
    RevBayesCore::TypedDagNode<std::vector<double> >* brlens = static_cast<const ModelVector<RealPos> &>( this->args[1].getVariable()->getRevObject() ).getDagNode();
    RevBayesCore::TreeAssemblyFunction* f = new RevBayesCore::TreeAssemblyFunction( tau, brlens );

    DeterministicNode<RevBayesCore::BranchLengthTree> *detNode = new DeterministicNode<RevBayesCore::BranchLengthTree>("", f, this->clone());
    
    BranchLengthTree* value = new BranchLengthTree( detNode );
    
    return new Variable( value );
}


/* Get argument rules */
const ArgumentRules& Func_treeAssembly::getArgumentRules( void ) const {
    
    static ArgumentRules argumentRules = ArgumentRules();
    static bool          rulesSet = false;
    
    if ( !rulesSet ) {
        
        argumentRules.push_back( new ArgumentRule( "topology", true, Topology::getClassTypeSpec() ) );
        argumentRules.push_back( new ArgumentRule( "brlens", true, ModelVector<RealPos>::getClassTypeSpec() ) );
        
        rulesSet = true;
    }
    
    return argumentRules;
}


const std::string& Func_treeAssembly::getClassType(void) { 
    
    static std::string revType = "Func_treeAssembly";
    
	return revType; 
}

/* Get class type spec describing type of object */
const TypeSpec& Func_treeAssembly::getClassTypeSpec(void) { 
    
    static TypeSpec revTypeSpec = TypeSpec( getClassType(), new TypeSpec( Function::getClassTypeSpec() ) );
    
	return revTypeSpec; 
}


/* Get return type */
const TypeSpec& Func_treeAssembly::getReturnType( void ) const {
    
    static TypeSpec returnTypeSpec = BranchLengthTree::getClassTypeSpec();
    
    return returnTypeSpec;
}


const TypeSpec& Func_treeAssembly::getTypeSpec( void ) const {
    
    static TypeSpec typeSpec = getClassTypeSpec();
    
    return typeSpec;
}
