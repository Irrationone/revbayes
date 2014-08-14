#ifndef Simplex_H
#define Simplex_H


#include "ModelVector.h"
#include "RealPos.h"
#include "TypedDagNode.h"
#include "TypeSpec.h"

#include <iostream>
#include <vector>

namespace RevLanguage {
    
    /**
     * @brief Rev simplex variable class
     *
     * This class is derived from ModelVector<RealPos> and inherits all
     * its indexing and type conversion functionality. The only changes
     * we need to introduce here are to override some vector functions
     * that are not applicable to simplices (sort, unique) and to override
     * the two functions providing assignment or external modification
     * abilities to the parser (findOrCreateElement, getElement).
     */
    
    class Simplex : public ModelVector<RealPos> {
        
    public:
        Simplex(void);                                                                                  //!< Construct empty simplex
        Simplex(const std::vector<double>& v);                                                          //!< Construct simplex from double (real) vector
        Simplex(RevBayesCore::TypedDagNode<std::vector<double> >* c);                                   //!< Construct simplex from DAG node

        virtual                                    ~Simplex(void);                                                      //!< Destructor        

        // Basic utility functions you have to override
        virtual Simplex*                            clone(void) const;                                                  //!< Clone object
        static const std::string&                   getClassType(void);                                                 //!< Get Rev type
        static const TypeSpec&                      getClassTypeSpec(void);                                             //!< Get class type spec
        virtual const TypeSpec&                     getTypeSpec(void) const;                                            //!< Get language type of the object
        
        // ModelVector functions overridden here to protect from assignment or external modification
        RevPtr<Variable>                            findOrCreateElement(const std::vector<size_t>& oneOffsetIndices);   //!< Find or create element variable
        RevPtr<Variable>                            getElement(size_t oneOffsetIndex);                                  //!< Get element variable

        // ModelVector functions that we override here to stop inappropriate actions
        void                                        sort(void);                                                         //!< Sort vector
        void                                        unique(void);                                                       //!< Remove consecutive duplicates
        
        // Member methods
        const MethodTable&                          getMethods(void) const;                                             //!< Get member methods

    private:
        std::vector<double>*                        makeNormalizedValue(const std::vector<double>& v);                  //!< Help function
        
    };
    
}


#endif

