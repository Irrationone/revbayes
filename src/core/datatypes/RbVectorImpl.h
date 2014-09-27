/**
 * @file
 * This file contains the declaration of the RbVector class.
 * The RbVector is our implementation of the stl vector, actually a wrapper class,
 * that internally stores the values in a stl-vector of pointers so that vectors
 * of abstract base classes can be used as well.
 *
 *
 * @brief Declaration of the RbVector class
 *
 * (c) Copyright 2009-
 * @date Last modified: $Date$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @since Version 1.0, 2012-07-18
 *
 * $Id$
 */

#ifndef RbVectorImpl_H
#define RbVectorImpl_H

#include "Cloner.h"
#include "Cloneable.h"
#include "IsDerivedFrom.h"
#include "RbIterator.h"
#include "RbConstIterator.h"

#include <vector>
#include <iostream>

namespace RevBayesCore {
    
    template <class valueType, int indicator>
    // general case: T is not abstract
    // use actual objects
    class RbVectorImpl : public Cloneable {
        
    public:
        // constructor(s)
        RbVectorImpl() {  }
        RbVectorImpl(size_t n) { values = std::vector<valueType>(n, valueType() ); }
        RbVectorImpl(size_t n, const valueType &v) { for (size_t i = 0; i < n; ++i) values.push_back( v ); }
        RbVectorImpl(const RbVectorImpl<valueType,indicator> &v) { size_t n=v.size(); for (size_t i = 0; i < n; ++i) values.push_back( v[i] ); }
        virtual                                            ~RbVectorImpl<valueType,indicator>(void) { clear(); }
        
        typedef typename std::vector<valueType>::iterator           iterator;
        typedef typename std::vector<valueType>::const_iterator     const_iterator;

        
        // public member functions
        virtual RbVectorImpl<valueType, indicator>*         clone(void) const = 0;                                                                      //!< Create an independent clone
        
        // public (stl-like) vector functions
        valueType&                                          operator[](size_t i) { return values[i]; }
        const valueType&                                    operator[](size_t i) const { return values[i]; }
        void                                                clear(void) { values.clear(); }
        void                                                insert(size_t i, const valueType &v) { values[i] = v; }
        void                                                push_back(const valueType &v) { values.push_back( v ); }
        virtual size_t                                      size(void) const = 0;

    protected:
        
        // private members
        std::vector<valueType>                              values;
    };
    
    template <typename valueType>
    // T is abstract
    // uses pointers
    class RbVectorImpl<valueType,1> : public Cloneable {
        
    public:
        // constructor(s)
        RbVectorImpl() {  }
        RbVectorImpl(size_t n) { values = std::vector<valueType*>(n, NULL); }
        RbVectorImpl(size_t n, const valueType &v) { for (size_t i = 0; i < n; ++i) values.push_back( Cloner<valueType, IsDerivedFrom<valueType, Cloneable>::Is >::createClone( v ) ); }
        RbVectorImpl(const RbVectorImpl<valueType,1> &v) : values() { size_t n=v.size(); for (size_t i = 0; i < n; ++i) values.push_back( Cloner<valueType, IsDerivedFrom<valueType, Cloneable>::Is >::createClone( v[i] ) ); }
        virtual                                            ~RbVectorImpl(void) { clear(); }
        
        typedef typename std::vector<valueType*>::iterator           iterator;
        typedef typename std::vector<valueType*>::const_iterator     const_iterator;

        
        // public member functions
        virtual RbVectorImpl<valueType, 1>*                 clone(void) const = 0;                                                                      //!< Create an independent clone
        
        // public (stl-like) vector functions
        valueType&                                          operator[](size_t i) { return *values[i]; }
        const valueType&                                    operator[](size_t i) const { return *values[i]; }
        void                                                clear(void) {
            size_t n = values.size();
            for (size_t i = 0; i < n; ++i)
            {
                valueType* v = values[i];
                delete v;
            }
            
            values.clear();
        }
        void                                                insert(size_t i, const valueType &v) { delete values[i]; values[i] = Cloner<valueType, IsDerivedFrom<valueType, Cloneable>::Is >::createClone( v ); }
        void                                                push_back(const valueType &v) { values.push_back( Cloner<valueType, IsDerivedFrom<valueType, Cloneable>::Is >::createClone( v ) ); }
        virtual size_t                                      size(void) const = 0;
        
    protected:
        
        // private members
        std::vector<valueType*>                             values;
    };

    
}


#endif

