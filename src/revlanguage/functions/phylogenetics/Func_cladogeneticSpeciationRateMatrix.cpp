//
//  Func_cladogeneticSpeciationRateMatrix.cpp
//
//  Created by Will Freyman on 8/1/17.
//

#include "CladogeneticSpeciationRateMatrix.h"
#include "CladogeneticSpeciationRateMatrixFunction.h"
#include "Func_cladogeneticSpeciationRateMatrix.h"
#include "RealPos.h"
#include "ModelVector.h"

using namespace RevLanguage;

/** default constructor */
Func_cladogeneticSpeciationRateMatrix::Func_cladogeneticSpeciationRateMatrix( void ) : TypedFunction<CladogeneticSpeciationRateMatrix>( ) {
    
}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */
Func_cladogeneticSpeciationRateMatrix* Func_cladogeneticSpeciationRateMatrix::clone( void ) const {
    
    return new Func_cladogeneticSpeciationRateMatrix( *this );
}


RevBayesCore::TypedFunction< RevBayesCore::CladogeneticSpeciationRateMatrix >* Func_cladogeneticSpeciationRateMatrix::createFunction( void ) const
{
   
    RevBayesCore::TypedDagNode< RevBayesCore::RbVector<RevBayesCore::RbVector<int> > >* events = static_cast<const ModelVector<ModelVector<Natural> > &>( this->args[0].getVariable()->getRevObject() ).getDagNode();
    RevBayesCore::TypedDagNode<RevBayesCore::RbVector<double> >* spec_rates = static_cast<const ModelVector<RealPos> &>( this->args[1].getVariable()->getRevObject() ).getDagNode();
    int n_states = static_cast<const Natural &>( this->args[2].getVariable()->getRevObject() ).getValue();
    
    RevBayesCore::CladogeneticSpeciationRateMatrixFunction* f = new RevBayesCore::CladogeneticSpeciationRateMatrixFunction( events, spec_rates, n_states );
    
    return f;
}


/* Get argument rules */
const ArgumentRules& Func_cladogeneticSpeciationRateMatrix::getArgumentRules( void ) const
{
    
    static ArgumentRules argumentRules = ArgumentRules();
    static bool          rules_set = false;
    
    if ( !rules_set )
    {
        
        argumentRules.push_back( new ArgumentRule( "cladogenetic_events", ModelVector<ModelVector<Natural> >::getClassTypeSpec(), "A vector of cladogenetic event types. Each type is in the form [ancestral_state, daughter1_state, daughter2_state].", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
        argumentRules.push_back( new ArgumentRule( "speciation_rates", ModelVector<RealPos>::getClassTypeSpec() , "The speciation rates that correspond to the different cladogenetic event types.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
        argumentRules.push_back( new ArgumentRule( "num_states", Natural::getClassTypeSpec(), "The number of states.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
        
        rules_set = true;
    }
    
    return argumentRules;
}


const std::string& Func_cladogeneticSpeciationRateMatrix::getClassType(void)
{
    
    static std::string rev_type = "Func_cladogeneticSpeciationRateMatrix";
    
    return rev_type;
}

/* Get class type spec describing type of object */
const TypeSpec& Func_cladogeneticSpeciationRateMatrix::getClassTypeSpec(void)
{
    
    static TypeSpec rev_type_spec = TypeSpec( getClassType(), new TypeSpec( Function::getClassTypeSpec() ) );
    
    return rev_type_spec;
}


/**
 * Get the primary Rev name for this function.
 */
std::string Func_cladogeneticSpeciationRateMatrix::getFunctionName( void ) const
{
    // create a name variable that is the same for all instance of this class
    std::string f_name = "fnCladogeneticSpeciationRateMatrix";
    
    return f_name;
}


const TypeSpec& Func_cladogeneticSpeciationRateMatrix::getTypeSpec( void ) const
{
    
    static TypeSpec type_spec = getClassTypeSpec();
    
    return type_spec;
}
