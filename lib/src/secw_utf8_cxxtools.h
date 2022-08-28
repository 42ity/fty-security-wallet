/*  =========================================================================
  secw_utf8_cxxtools.h - handle std::string utf-8 in/out with cxxtools

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

#pragma once

#include <cxxtools/string.h>
#include <cxxtools/utf8codec.h>
#include <cxxtools/jsonserializer.h>
#include <string>
#include <sstream>

// encode std::string (utf-8) to cxxtools::String
inline std::string CxxStringToStdString(const cxxtools::String& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// decode cxxtools::String to std::string (utf-8)
inline cxxtools::String StdStringToCxxString(const std::string& value)
{
    cxxtools::String cxxStr = cxxtools::Utf8Codec::decode(value);
    return cxxStr;
}

// helper, get std::string (utf-8) from SerializationInfo& member
inline std::string GetSiMemberCxxString(const cxxtools::SerializationInfo& si, const std::string& memberName)
{
    cxxtools::String cxxStr;
    si.getMember(memberName) >>= cxxStr;
    std::string stdStr = CxxStringToStdString(cxxStr);
    return stdStr; // utf-8
}
