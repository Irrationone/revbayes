#ifndef SyntaxDecrement_H
#define SyntaxDecrement_H

#include "SyntaxElement.h"
#include "SyntaxVariable.h"

#include <iostream>
#include <list>
#include <string>

namespace RevLanguage {
    
    /**
     * @brief Decrement operator ('a--' and '--a')
     *
     * The decrement operator subtracts one from the value of the variable.
     * We do not distinguish between post- and pre-decrements, as decrement
     * statements cannot occur inside the specification of a for loop.
     * Instead, we use the R-like in-expression syntax for for loops.
     *
     * Note that we respect the control variable status of the variable.
     * In other words, the decrement statement 'a--' or '--a' would be
     * equivalent to
     *
     *    a <- a - 1
     *
     * if 'a' were a constant variable, and to
     *
     *    a <<- a - 1
     *
     * if 'a' were a control variable.
     */
    class SyntaxDecrement : public SyntaxElement {
        
    public:
        SyntaxDecrement(SyntaxVariable* v);                                                                 //!< Basic constructor
        SyntaxDecrement(const SyntaxDecrement& x);                                                          //!< Copy constructor

	    virtual                            ~SyntaxDecrement();                                              //!< Destructor
        
        // Assignment operator
        SyntaxDecrement&                    operator=(const SyntaxDecrement& x);                            //!< Assignment operator
        
        // Basic utility functions
        SyntaxDecrement*                    clone() const;                                                  //!< Clone object
        void                                printValue(std::ostream& o) const;                              //!< Print info about object
        
        // Regular functions
        RevPtr<Variable>                    evaluateContent(Environment& env);                              //!< Get semantic value
        bool                                isFunctionSafe(const Environment&       env,
                                                           std::set<std::string>&   localVars) const;       //!< Is this element safe in a function?
        
    protected:
        SyntaxVariable*                     variable;                                                       //!< The variable expression
        
    };
    
}

#endif

