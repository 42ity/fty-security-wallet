/*  =========================================================================
    cam_helpers - List of helper functions use a bit everywhere

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

#include "cam_helpers.h"
#include "cam_exception.h"
#include <fty_common_json.h>
#include <cxxtools/serializationinfo.h>
#include <iostream>
#include <sstream>

namespace cam {
cxxtools::SerializationInfo deserialize(const std::string& json)
{
    cxxtools::SerializationInfo si;

    try {
        JSON::readFromString(json, si);
    } catch (const std::exception& e) {
        throw CamProtocolErrorException("Error in the json from server: " + std::string(e.what()));
    }

    return si;
}

std::string serialize(const cxxtools::SerializationInfo& si)
{
    try {
        cxxtools::SerializationInfo si2{si}; //deconst
        return JSON::writeToString(si2, false);
    } catch (const std::exception& e) {
        throw CamException("Error while creating json " + std::string(e.what()));
    }
}

} // namespace cam
