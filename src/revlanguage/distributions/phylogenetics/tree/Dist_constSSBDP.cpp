#include "ArgumentRule.h"
#include "ArgumentRules.h"
#include "Clade.h"
#include "ConstantRateSerialSampledBirthDeathProcess.h"
#include "Dist_constSSBDP.h"
#include "ModelVector.h"
#include "Natural.h"
#include "OptionRule.h"
#include "Probability.h"
#include "Real.h"
#include "RealPos.h"
#include "RlClade.h"
#include "RlString.h"
#include "RlTaxon.h"
#include "RlTimeTree.h"

using namespace RevLanguage;

/**
 * Default constructor.
 *
 * The default constructor does nothing except allocating the object.
 */
Dist_constSSBDP::Dist_constSSBDP() : BirthDeathProcess()
{
    
}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */
Dist_constSSBDP* Dist_constSSBDP::clone( void ) const
{
    return new Dist_constSSBDP(*this);
}


/**
 * Create a new internal distribution object.
 *
 * This function simply dynamically allocates a new internal distribution object that can be
 * associated with the variable. The internal distribution object is created by calling its
 * constructor and passing the distribution-parameters (other DAG nodes) as arguments of the
 * constructor. The distribution constructor takes care of the proper hook-ups.
 *
 * \return A new internal distribution object.
 */
RevBayesCore::ConstantRateSerialSampledBirthDeathProcess* Dist_constSSBDP::createDistribution( void ) const
{
    
    // get the parameters
    
    // the start age
    RevBayesCore::TypedDagNode<double>* sa       = static_cast<const RealPos &>( startAge->getRevObject() ).getDagNode();
    // speciation rate
    RevBayesCore::TypedDagNode<double>* s       = static_cast<const RealPos &>( lambda->getRevObject() ).getDagNode();
    // extinction rate
    RevBayesCore::TypedDagNode<double>* e       = static_cast<const RealPos &>( mu->getRevObject() ).getDagNode();
    // fossilization rate
    RevBayesCore::TypedDagNode<double>* p       = static_cast<const RealPos &>( psi->getRevObject() ).getDagNode();
    // sampling probability
    RevBayesCore::TypedDagNode<double>* r       = static_cast<const Probability &>( rho->getRevObject() ).getDagNode();
    // the start condition
    bool uo = ( startCondition == "originAge" ? true : false );
    
    // sampling condition
    const std::string& cond                     = static_cast<const RlString &>( condition->getRevObject() ).getValue();
    
    // get the taxa to simulate either from a vector of rev taxon objects or a vector of names
    std::vector<RevBayesCore::Taxon> t = static_cast<const ModelVector<Taxon> &>( taxa->getRevObject() ).getValue();
    
    // create the internal distribution object
    RevBayesCore::ConstantRateSerialSampledBirthDeathProcess* d = new RevBayesCore::ConstantRateSerialSampledBirthDeathProcess(sa, s, e, p, r, uo, cond, t);
    
    return d;
}


/**
 * Get Rev type of object
 *
 * \return The class' name.
 */
const std::string& Dist_constSSBDP::getClassType( void )
{
    
    static std::string rev_type = "Dist_constSSBDP";
    
    return rev_type;
}


/**
 * Get class type spec describing type of an object from this class (static).
 *
 * \return TypeSpec of this class.
 */
const TypeSpec& Dist_constSSBDP::getClassTypeSpec( void )
{
    
    static TypeSpec rev_type_spec = TypeSpec( getClassType(), new TypeSpec( BirthDeathProcess::getClassTypeSpec() ) );
    
    return rev_type_spec;
}


/**
 * Get the alternative Rev names (aliases) for the constructor function.
 *
 * \return Rev aliases of constructor function.
 */
std::vector<std::string> Dist_constSSBDP::getDistributionFunctionAliases( void ) const
{
    // create alternative constructor function names variable that is the same for all instance of this class
    std::vector<std::string> a_names;
    a_names.push_back( "SSBDP" );
    
    return a_names;
}


/**
 * Get the Rev name for the distribution.
 * This name is used for the constructor and the distribution functions,
 * such as the density and random value function
 *
 * \return Rev name of constructor function.
 */
std::string Dist_constSSBDP::getDistributionFunctionName( void ) const
{
    // create a distribution name variable that is the same for all instance of this class
    std::string d_name = "SerialSampledBirthDeath";
    
    return d_name;
}


/**
 * Get the member rules used to create the constructor of this object.
 *
 * The member rules of the constant-rate birth-death process are:
 * (1) the speciation rate lambda which must be a positive real.
 * (2) the extinction rate mu that must be a positive real.
 * (3) all member rules specified by BirthDeathProcess.
 *
 * \return The member rules.
 */
const MemberRules& Dist_constSSBDP::getParameterRules(void) const
{
    
    static MemberRules dist_member_rules;
    static bool rules_set = false;
    
    if ( !rules_set )
    {
        
        std::vector<std::string> aliases;
        aliases.push_back("rootAge");
        aliases.push_back("originAge");
        dist_member_rules.push_back( new ArgumentRule( aliases,   RealPos::getClassTypeSpec(), "The start age of the process, either the root age or the origin time.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
        dist_member_rules.push_back( new ArgumentRule( "lambda",  RealPos::getClassTypeSpec(), "The constant speciation rate.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
        dist_member_rules.push_back( new ArgumentRule( "mu",      RealPos::getClassTypeSpec(), "The constant extinction rate.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY, new RealPos(0.0) ) );
        dist_member_rules.push_back( new ArgumentRule( "psi",     RealPos::getClassTypeSpec(), "The constant sampling rate.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY, new RealPos(0.0) ) );
        
        // add the rules from the base class
        const MemberRules &parentRules = BirthDeathProcess::getParameterRules();
        for (size_t i = 0; i < parentRules.size(); i++)
        {
            if ( parentRules[i].getArgumentLabel() == "rho" )
            {
                ArgumentRule* tmp = parentRules[i].clone();
                dist_member_rules.push_back( tmp );
                std::vector<std::string> optionsCondition;
                
            }
            else if ( parentRules[i].getArgumentLabel() != "rootAge" )
            {
                ArgumentRule* tmp = parentRules[i].clone();
                dist_member_rules.push_back( tmp );
            }
        }
        
        rules_set = true;
    }
    
    return dist_member_rules;
}


/**
 * Get type-specification on this object (non-static).
 *
 * \return The type spec of this object.
 */
const TypeSpec& Dist_constSSBDP::getTypeSpec( void ) const
{
    
    static TypeSpec ts = getClassTypeSpec();
    
    return ts;
}


/**
 * Set a member variable.
 *
 * Sets a member variable with the given name and store the pointer to the variable.
 * The value of the variable might still change but this function needs to be called again if the pointer to
 * the variable changes. The current values will be used to create the distribution object.
 *
 * \param[in]    name     Name of the member variable.
 * \param[in]    var      Pointer to the variable.
 */
void Dist_constSSBDP::setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var)
{
    if (name == "rootAge" || name == "originAge")
    {
        startCondition = name;
        startAge = var;
    }
    else if ( name == "lambda" )
    {
        lambda = var;
    }
    else if ( name == "mu" )
    {
        mu = var;
    }
    else if ( name == "psi" )
    {
        psi = var;
    }
    else
    {
        BirthDeathProcess::setConstParameter(name, var);
    }
    
}
