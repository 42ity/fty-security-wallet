/*  =========================================================================
    secw_tag_description - Description of a tag

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
    secw_tag_description - Description of a tag
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

namespace secw
{

    TagDescription::TagDescription( const std::string & id,
                const std::string & name,
                const std::string & description):
        m_id(id),
        m_name(name),
        m_description(description)
    {}

    void operator>>= (const cxxtools::SerializationInfo& si, TagDescription & tag)
    {
        si.getMember(TAG_ID_ENTRY) >>= tag.m_id;
        si.getMember(TAG_NAME_ENTRY) >>= tag.m_name;
        si.getMember(TAG_DESCRIPTION_ENTRY) >>= tag.m_description;
    }

    void operator<<= (cxxtools::SerializationInfo& si, const TagDescription & tag)
    {
        si.addMember(TAG_ID_ENTRY) <<= tag.getId();
        si.addMember(TAG_NAME_ENTRY) <<= tag.getName();
        si.addMember(TAG_DESCRIPTION_ENTRY) <<= tag.getDescription();
    }

} //namespace secw


