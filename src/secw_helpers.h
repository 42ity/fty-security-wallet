/*  =========================================================================
    secw_helpers - List of helper functions use a bit everywhere

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

#ifndef SECW_HELPERS_H_INCLUDED
#define SECW_HELPERS_H_INCLUDED

namespace secw
{
  cxxtools::SerializationInfo deserialize(const std::string & json);
  
  std::string serialize(const cxxtools::SerializationInfo & si);

  bool hasCommonUsageIds( const std::set<std::string> &  usages1, const std::set<std::string> &  usages2);

} //namespace secw

#endif
