/*  =========================================================================
    secw_access - Handle the access rights on tag for clients in the agent

    Copyright (C) 2019 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

#ifndef SECW_ACCESS_H_INCLUDED
#define SECW_ACCESS_H_INCLUDED

#include "cxxtools/serializationinfo.h"

namespace secw
{
    typedef int AllowedAccess_t;
    typedef enum : AllowedAccess_t
    {
        ____ = 0x0,
        C___ = 0x1,
        _R__ = 0x2,
        CU__ = C___ | _R__,        
        __U_ = 0x4,
        C_U_ = C___ | __U_,
        _RU_ = _R__ | __U_,
        CRU_ = C___ | _R__ | __U_,
        ___D = 0x8,
        C__D = C___ | ___D,
        _R_D = _R__ | ___D,
        CR_D = C___ | _R__ | ___D,
        __UD = __U_ | ___D,
        C_UD = C___ | __U_ | ___D,
        _RUD = _R__ | __U_ | ___D,
        CRUD = C___ | _R__| __U_ | ___D,
                
    } AllowedAccess;
    
    typedef AllowedAccess_t AccessMethods_t;
    typedef enum : AccessMethods_t
    {
        CREATE_ACCESS   = C___,
        READ_ACCESS     = _R__,
        UPDATE_ACCESS   = __U_,
        DELETE_ACCESS   = ___D,
    } AccessMethods;
    
    AllowedAccess & operator&=(AllowedAccess & a, const AllowedAccess & b);
    AllowedAccess & operator|=(AllowedAccess & a, const AllowedAccess & b);
    AllowedAccess operator&(const AllowedAccess & a, const AllowedAccess & b);
    AllowedAccess operator|(const AllowedAccess & a, const AllowedAccess & b);
    
    
    
    class Access
    {
    public:
        Access(const std::string & regexClient = "", AllowedAccess allowAccess = ____);
        bool checkAccess(const std::string & client, AccessMethods method) const;
        
        friend void operator>>= (const cxxtools::SerializationInfo& si, Access & access);
        
    private:
        std::string m_regexClient;
        AllowedAccess m_allowedAccess;
    };
    
    void operator>>= (const cxxtools::SerializationInfo& si, Access & access);
    
    class TagAccess
    {
    public:
        explicit TagAccess(const std::string & name = "");
        bool checkAccess(const std::string & client, AccessMethods method) const;
        
        std::string getName() const { return m_name; }
        
        friend void operator>>= (const cxxtools::SerializationInfo& si, TagAccess & access);
        
    private:
        std::string m_name;
        std::vector<Access> m_accessList;
    };
    
    void operator>>= (const cxxtools::SerializationInfo& si, TagAccess & tagAccess);
} // namespace secw


#endif
