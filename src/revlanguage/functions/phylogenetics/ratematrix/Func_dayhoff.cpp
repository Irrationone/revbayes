#include "DeterministicNode.h"
#include "Func_dayhoff.h"
#include "Natural.h"
#include "RateMatrix_Dayhoff.h"
#include "Real.h"
#include "RealPos.h"
#include "RlRateMatrix.h"
#include "RlSimplex.h"
#include "TypedDagNode.h"

using namespace RevLanguage;

/** default constructor */
Func_dayhoff::Func_dayhoff( void ) : Procedure( )
{
    
}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */
Func_dayhoff* Func_dayhoff::clone( void ) const
{
    
    return new Func_dayhoff( *this );
}


RevPtr<RevVariable> Func_dayhoff::execute()
{
    
    
    RevBayesCore::RateMatrix_Dayhoff *rmj = new RevBayesCore::RateMatrix_Dayhoff();
    RateMatrix* value = new RateMatrix( rmj );
    
    return new RevVariable( value );
}


/* Get argument rules */
const ArgumentRules& Func_dayhoff::getArgumentRules( void ) const
{
    
    static ArgumentRules argumentRules = ArgumentRules();
    
    return argumentRules;
}


const std::string& Func_dayhoff::getClassType(void)
{
    
    static std::string rev_type = "Func_dayhoff";
    
	return rev_type; 
}

/* Get class type spec describing type of object */
const TypeSpec& Func_dayhoff::getClassTypeSpec(void)
{
    
    static TypeSpec rev_type_spec = TypeSpec( getClassType(), new TypeSpec( Function::getClassTypeSpec() ) );
    
	return rev_type_spec; 
}


/**
 * Get the primary Rev name for this function.
 */
std::string Func_dayhoff::getFunctionName( void ) const
{
    // create a name variable that is the same for all instance of this class
    std::string f_name = "fnDayhoff";
    
    return f_name;
}


/* Get return type */
const TypeSpec& Func_dayhoff::getReturnType( void ) const
{
    
    static TypeSpec returnTypeSpec = RateGenerator::getClassTypeSpec();
    
    return returnTypeSpec;
}


const TypeSpec& Func_dayhoff::getTypeSpec( void ) const
{
    
    static TypeSpec type_spec = getClassTypeSpec();
    
    return type_spec;
}
