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

/*
@header
    secw_access - Handle the access rights on tag for clients in the agent
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

#include <regex>

namespace secw
{
    Access::Access(const std::string & regexClient, AllowedAccess allowAccess):
        m_regexClient(regexClient),
        m_allowedAccess(allowAccess)    
    {}
    
    bool Access::checkAccess(const std::string & client, AccessMethods method) const
    {
        bool allowed = false;
        
        if(std::regex_match(client, std::regex("^"+m_regexClient+"$")))
        {
            AllowedAccess allowedAccess = m_allowedAccess & static_cast<AllowedAccess>(method);

            //log_info("%i = %i | %i", allowedAccess, m_allowedAccess, static_cast<AllowedAccess>(method));

            allowed = (allowedAccess != ____); 
        }
       
        return allowed;
    }

    void operator>>= (const cxxtools::SerializationInfo& si, Access & access)
    {
        si.getMember("regex_client") >>= access.m_regexClient;
        
        std::string strAccess = "";
        si.getMember("access") >>= strAccess;
        
        
        if(strAccess.length() != 4)
        {
            throw std::runtime_error("Bad access value format");
        }
        
        access.m_allowedAccess = ____;
        
        if(strAccess[0] == 'C')
        {
            access.m_allowedAccess |= C___;
        }
        
        if(strAccess[1] == 'R')
        {
            access.m_allowedAccess |= _R__;
        }
        
        if(strAccess[2] == 'U')
        {
            access.m_allowedAccess |= __U_;
        }
        
        if(strAccess[3] == 'D')
        {
            access.m_allowedAccess |= ___D;
        }
    }
    
    TagAccess::TagAccess(const std::string & name):
        m_name(name)
    {}
    
    bool TagAccess::checkAccess(const std::string & client, AccessMethods method) const
    {
        bool allowed = false;
        
        for(const Access & access : m_accessList)
        {
            if(access.checkAccess(client, method))
            {
                allowed = true;
                break;
            }
        }
        
        return allowed;
    }
    
    void operator>>= (const cxxtools::SerializationInfo& si, TagAccess & tagAccess)
    {
        si.getMember("tag_name") >>= tagAccess.m_name;
        si.getMember("access_list") >>= tagAccess.m_accessList;
    }
    
//AllowedAccess operators
    
    AllowedAccess & operator&=(AllowedAccess & a, const AllowedAccess & b)
    {
        a = a & b;
        return a;
    }
    AllowedAccess & operator|=(AllowedAccess & a, const AllowedAccess & b)
    {
        a = a | b;
        return a;
    }
    
    AllowedAccess operator&(const AllowedAccess & a, const AllowedAccess & b)
    {
        AllowedAccess ret = static_cast<AllowedAccess>(static_cast<AllowedAccess_t>(a) & static_cast<AllowedAccess_t>(b));
        return ret;
    }
    
    AllowedAccess operator|(const AllowedAccess & a, const AllowedAccess & b)
    {
        AllowedAccess ret = static_cast<AllowedAccess>( static_cast<AllowedAccess_t>(a) | static_cast<AllowedAccess_t>(b));
        return ret;
    }
} // namespace secw


