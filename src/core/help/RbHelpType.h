#ifndef RbHelpType_H
#define RbHelpType_H

#include "RbHelpArgument.h"
#include "RbHelpReference.h"

#include <string>
#include <vector>

namespace RevBayesCore {
    
    
    /**
     * \brief The help entry for a type.
     *
     * A help entry for a type provides all the help information available on this type, e.g., 
     * about member methods.
     *
     * \copyright (c) Copyright 2009-2013 (GPL version 3)
     * \author The RevBayes Development Core Team (Johan Dunfalk & Sebastian Hoehna)
     * \since Version 1.0, 2014-09-15
     *
     */
    class RbHelpType {
    public:
        
        // getter
        const std::string&                          getAuthor(void) const;
        const std::vector<std::string>&             getDescription(void) const;
        const std::vector<std::string>&             getDetails(void) const;
        const std::string&                          getName(void) const;
        const std::vector<RbHelpReference>&         getReferences(void) const;
        const std::string&                          getTitle(void) const;
        const std::string&                          getUsage(void) const;
        
        // setters
        void                                        setAuthor(const std::string &a);
        void                                        setDescription(const std::vector<std::string> &d);
        void                                        setDetails(const std::vector<std::string> &d);
        void                                        setName(const std::string &n);
        void                                        setReferences(const std::vector<RbHelpReference> &r);
        void                                        setTitle(const std::string &t);
        void                                        setUsage(const std::string &u);
        
        
    private:
        
        std::string                                 name;
        std::string                                 title;
        std::vector<std::string>                    description;
        std::string                                 usage;
//        std::vector<MethodMemberHelpEntry>          methodMembers;
        std::vector<std::string>                    details;
        std::string                                 author;
        std::vector<RbHelpReference>                references;

        
    };
    
    
}

#endif

