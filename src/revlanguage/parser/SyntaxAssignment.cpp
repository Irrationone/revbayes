#include "RbException.h"
#include "RbUtil.h"
#include "RbOptions.h"
#include "SyntaxAssignment.h"
#include "Workspace.h"

#include <iostream>
#include <list>
#include <sstream>

using namespace RevLanguage;

/** Basic constructor from lef-hand side and right-hand side expressions */
SyntaxAssignment::SyntaxAssignment( SyntaxElement* lhsExpr, SyntaxElement* rhsExpr ) :SyntaxElement(),
    lhsExpression( lhsExpr ),
    rhsExpression( rhsExpr )
{
    
}


/** Deep copy constructor */
SyntaxAssignment::SyntaxAssignment( const SyntaxAssignment& x ) : SyntaxElement( x ),
    lhsExpression( x.lhsExpression->clone() ),
    rhsExpression( x.rhsExpression->clone() )
{
    
}


/** Destructor deletes operands */
SyntaxAssignment::~SyntaxAssignment( void )
{
    delete lhsExpression;
    delete rhsExpression;
}


/** Assignment operator */
SyntaxAssignment& SyntaxAssignment::operator=( const SyntaxAssignment& x )
{
    if ( this != &x )
    {
        
        delete lhsExpression;
        delete rhsExpression;
        
        lhsExpression = x.lhsExpression->clone();
        rhsExpression = x.rhsExpression->clone();
    }
    
    return (*this);
}


/**
 * Get semantic value. The semantic value of a constant assignment is the same
 * independent of grammatical context, so we only need one evaluate function.
 * We first evaluate the lhs content of the lhs expression. If it is a variable,
 * and the variable does not exist in the workspace, the variable will be created
 * and inserted into the workspace (the environment) before being returned as the
 * content of the lhs expression. Other expressions will typically return temporary
 * variables that will be lost after the assignment, but it could be used in some
 * contexts. For instance, it might be used in a chain assignment or in passing a
 * variable to a function.
 */
RevPtr<Variable> SyntaxAssignment::evaluateContent( Environment& env, bool dynamic )
{
#ifdef DEBUG_PARSER
    printf( "Evaluating constant assignment\n" );
#endif
    
    // Get the rhs expression wrapped and executed into a variable.
    RevPtr<Variable> theVariable = rhsExpression->evaluateContent( env, isDynamic() );
    
    // Get variable slot from lhs
    RevPtr<Variable> theSlot = lhsExpression->evaluateLHSContent( env, theVariable->getRevObject().getType() );
    
    // let us remove all potential indexed variables
    removeElementVariables(env, theSlot);
    
    // now we delegate to the derived class
    assign(theSlot, theVariable);
    
    // We return the rhs variable itself as the semantic value of the
    // assignment statement. It can be used in further assignments.
    return theSlot;
}


/** This is an assignment, return true. */
bool SyntaxAssignment::isAssignment( void ) const
{
    return true;
}


/** Should the rhs be evaluated dynamically. By default, this is false. */
bool SyntaxAssignment::isDynamic( void )
{
    return false;
}






/**
 * Is the syntax element safe for use in a function (as
 * opposed to a procedure)? The assignment is safe
 * if its lhs and rhs expressions are safe, and the
 * assignment is not to an external variable.
 */
bool SyntaxAssignment::isFunctionSafe( const Environment& env, std::set<std::string>& localVars ) const
{
    // Check lhs and rhs expressions
    if ( !lhsExpression->isFunctionSafe( env, localVars ) || !rhsExpression->isFunctionSafe( env, localVars ) )
        return false;
    
    // Check whether assignment is to external variable (not function safe)
    if ( lhsExpression->retrievesExternVar( env, localVars, true ) )
        return false;
    
    // All tests passed
    return true;
}



/** Print info about the syntax element */
void SyntaxAssignment::printValue( std::ostream& o ) const
{
    o << "SyntaxAssignment:" << std::endl;
    o << "lhsExpression = ";
    lhsExpression->printValue( o );
    o << std::endl;
    o << "rhsExpression = ";
    rhsExpression->printValue( o );
    o << std::endl;
}


/** 
 * Removing all element variables from this variable.
 * First, we need to check if this is a vector variable, 
 * and then we perform the remove element recursively.
 */
void SyntaxAssignment::removeElementVariables(Environment &env, RevPtr<Variable> &theVar)
{
    // check if the variable is a vector variable
    if ( theVar->isVectorVariable() == true )
    {
        int min = theVar->getMinIndex();
        int max = theVar->getMaxIndex();
        if ( min > max )
        {
            throw RbException("Cannot remove a vector variable with name '" + theVar->getName() + "' because it doesn't have elements.");
        }
        // iterate over all elements
        for (int i=min; i<=max; ++i)
        {
            std::ostringstream s;
            s << theVar->getName() << "[" << i << "]";
            std::string elementIdentifier = s.str();
            RevPtr<Variable>& elementVar = env.getVariable( elementIdentifier );
            
            // recursively remove the element
            removeElementVariables( env, elementVar );
            
            // now we remove the elementVar from the workspace
            env.eraseVariable( elementIdentifier );
        }
        
        theVar->setVectorVariableState( false );
    }
    
}


