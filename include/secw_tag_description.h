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

#ifndef SECW_TAG_DESCRIPTION_H_INCLUDED
#define SECW_TAG_DESCRIPTION_H_INCLUDED

#include "cxxtools/serializationinfo.h"

namespace secw
{
    /**
     * Some key definition for serialization
     * 
     */
    static constexpr const char* TAG_ID_ENTRY = "tag_id";
    static constexpr const char* TAG_NAME_ENTRY = "tag_name";
    static constexpr const char* TAG_DESCRIPTION_ENTRY = "tag_description";


    class TagDescription
    {
    public:
        explicit TagDescription( const std::string & id,
                const std::string & name,
                const std::string & description);

        std::string getName() const { return m_name; }
        std::string getId() const { return m_id; }
        std::string getDescription() const { return m_description; }

        friend void operator>>= (const cxxtools::SerializationInfo& si, TagDescription & tag);
        
    private:
        std::string m_id;
        std::string m_name;
        std::string m_description;
    };

    void operator>>= (const cxxtools::SerializationInfo& si, TagDescription & tag);
    void operator<<= (cxxtools::SerializationInfo& si, const TagDescription & tag);

} //namespace secw

#endif
