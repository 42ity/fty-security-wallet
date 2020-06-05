/*  =========================================================================
    secw_configuration - Handle the configuration of a security wallet

    Copyright (C) 2019 - 2020 Eaton

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

#include <regex>

namespace secw
{
/*-----------------------------------------------------------------------------*/
/*   PortfolioConfiguration                                                         */
/*-----------------------------------------------------------------------------*/
  PortfolioConfiguration::PortfolioConfiguration(const cxxtools::SerializationInfo & si)
  {
    si.getMember("portfolio_name") >>= m_portfolioName;

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

    si.getMember("consumers") >>= m_consumers;
    si.getMember("producers") >>= m_producers;

    //TODO
    //check that the types are all in the system
    /*for( const Type & type : m_supportedTypes )
    {
      if()
    }*/
  }

  std::set<UsageId> PortfolioConfiguration::getAllUsageId() const
  {
    std::set<UsageId> usageIds;

    for( const auto & item : m_usages)
    {
      usageIds.insert(item.first);
    }

    return usageIds;
  }

  std::string PortfolioConfiguration::getPortfolioName() const
  {
    return m_portfolioName;
  }

  Usage PortfolioConfiguration::getUsage( const UsageId & usageId ) const
  {
    //TODO check error if none existing usageId
    return m_usages.at(usageId);
  }

  std::set<UsageId> PortfolioConfiguration::getUsageIdsForConsummer( const ClientId & clientId ) const
  {
    std::set<UsageId> usages;

    for(const Consumer & consumer : m_consumers)
    {
      //if it match we add all the usage id into the set
      if(consumer.isMatchingClient(clientId))
      {
        const std::set<UsageId> & consumerUsages(consumer.getUsageIds());

        std::copy(consumerUsages.begin(), consumerUsages.end(),
          std::inserter(usages, usages.end()));
      }
    }
    return usages;
  }

  std::set<UsageId> PortfolioConfiguration::getUsageIdsForProducer( const ClientId & clientId ) const
  {
    std::set<UsageId> usages;

    for(const Producer & producer : m_producers)
    {
      //if it match we add all the usage id into the set
      if(producer.isMatchingClient(clientId))
      {
        const std::set<UsageId> & producerUsages(producer.getUsageIds());

        std::copy(producerUsages.begin(), producerUsages.end(),
          std::inserter(usages, usages.end()));
      }
    }
    return usages;
  }

/*-----------------------------------------------------------------------------*/
/*   Usage                                                                     */
/*-----------------------------------------------------------------------------*/
//public

  const UsageId & Usage::getUsageId() const
  {
    return m_usageId;
  }

  const std::set<Type> & Usage::getTypes() const
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

/*-----------------------------------------------------------------------------*/
/*   Client                                                                    */
/*-----------------------------------------------------------------------------*/
//public
  bool Client::isMatchingClient( const ClientId & clientId) const
  {
    log_debug(" Client='%s' try to match with regex '%s'", clientId.c_str(), m_clientRegex.c_str());

    return std::regex_match(clientId, std::regex("^"+m_clientRegex+"$"));
  }

  const std::set<UsageId> & Client::getUsageIds() const
  {
    return m_usages;
  }

  void operator>>= (const cxxtools::SerializationInfo& si, Client & client)
  {
    si.getMember("client_regex") >>= client.m_clientRegex;
    si.getMember("usages") >>= client.m_usages;
  }

/*-----------------------------------------------------------------------------*/
/*   Others                                                                    */
/*-----------------------------------------------------------------------------*/
  std::set<UsageId> differenceBetween2UsagesIdSet(const std::set<UsageId> & a, const std::set<UsageId> & b)
  {
      std::set<UsageId> diff;

      //check if all a are in b
      for(const UsageId & usage : a)
      {
          if(b.count(usage) == 0)
          {
              diff.insert(usage);
          }
      }

      //check if all b are in a
      for(const UsageId & usage : b)
      {
        if(a.count(usage) == 0)
        {
            diff.insert(usage);
        }
      }

      return diff;
  }

} // namespace secw

