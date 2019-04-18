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

#ifndef SECW_CONFIGURATION_H_INCLUDED
#define SECW_CONFIGURATION_H_INCLUDED

#include "secw_document.h"

using UsageId	= std::string;
using Type 		= std::string;

namespace secw
{
  class Usage
  {
  public:
    Usage(){}
    UsageId getUsageId() const;
    std::set<Type> getTypes() const;

    friend void operator>>= (const cxxtools::SerializationInfo& si, Usage & usage);

  private:
    UsageId m_usageId;
    std::set<Type> m_types;
  };

  void operator>>= (const cxxtools::SerializationInfo& si, Usage & usage);

  class SecwConfiguration
  {
  public:
    explicit SecwConfiguration(const cxxtools::SerializationInfo & si);

    Usage getUsage( const UsageId & usageId ) const;

    std::set<UsageId> getAllUsageId() const;
    std::set<UsageId> getUsageIdForConsummer( const ClientId & clientId ) const;
    std::set<UsageId> getUsageIdForProducer( const ClientId & clientId ) const;

    std::set<Type> getSupportedTypes() const;

  private:
    std::map<UsageId, Usage> m_usages;
    std::set<Type> m_supportedTypes;
  };


  /*class Producer
  {
  public:

  private:

  };

  class Consumer
  {
  public:

  private:

  };*/

} // namespace secw

#endif
