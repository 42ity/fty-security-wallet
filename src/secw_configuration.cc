/*  =========================================================================
    secw_configuration - Handle the configuration of a security wallet

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
    secw_configuration - Handle the configuration of a security wallet
@discuss
@end
*/

#include "fty_security_wallet_classes.h"
namespace secw
{
/*-----------------------------------------------------------------------------*/
/*   SecwConfiguration                                                         */
/*-----------------------------------------------------------------------------*/
  SecwConfiguration::SecwConfiguration(const cxxtools::SerializationInfo & si)
  {
    std::vector<Usage> usages;
    si.getMember("usages") >>= usages;

    for(const Usage & usage : usages)
    {
      m_usages[usage.getUsageId()] = usage;

      for(const Type & type : usage.getTypes())
      {
        m_supportedTypes.insert(type);
      }
    }

    //TODO
    //check that the types are all in the system
    /*for( const Type & type : m_supportedTypes )
    {
      if()
    }*/
  }

  std::set<UsageId> SecwConfiguration::getAllUsageId() const
  {
    std::set<UsageId> usageIds;

    for( const auto & item : m_usages)
    {
      usageIds.insert(item.first);
    }

    return usageIds;
  }

  Usage SecwConfiguration::getUsage( const UsageId & usageId ) const
  {
    //TODO check error if none existing usageId
    return m_usages.at(usageId);
  }

  std::set<UsageId> SecwConfiguration::getUsageIdForConsummer( const ClientId & /*clientId*/ ) const
  {
    return getAllUsageId();
  }

  std::set<UsageId> SecwConfiguration::getUsageIdForProducer( const ClientId & /*clientId*/ ) const
  {
    return getAllUsageId();
  }

/*-----------------------------------------------------------------------------*/
/*   Usage                                                                     */
/*-----------------------------------------------------------------------------*/
//public

  UsageId Usage::getUsageId() const
  {
    return m_usageId;
  }

  std::set<Type> Usage::getTypes() const
  {
    return m_types;
  }

  void operator>>= (const cxxtools::SerializationInfo& si, Usage & usage)
  {
    si.getMember("usage_id") >>= usage.m_usageId;

    std::vector<Type> types;
    si.getMember("supported_types") >>= types;

    std::copy(types.begin(), types.end(),
        std::inserter(usage.m_types, usage.m_types.end()));

    if(usage.m_usageId.empty()) throw std::runtime_error("usage_id cannot be empty");
  }

} // namespace secw

