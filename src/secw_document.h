/*  =========================================================================
    secw_document - Document parsers

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

#ifndef SECW_DOCUMENT_H_INCLUDED
#define SECW_DOCUMENT_H_INCLUDED

/**
 * \brief Document
 */
class Document
{
public:
    virtual std::string getName(){return m_name;};
    virtual std::string getType(){return m_type;};
    virtual std::string getId(){return m_id;};
    virtual std::string getPublicPart(){return m_content_public;};
    virtual std::string getPrivatePart(){return m_content_private;};
    virtual std::string toJSON_withSecret();
    virtual std::string toJSON_withoutSecret();

protected:
    std::string m_name,m_type,m_id;
    std::string m_content_public,m_content_private;    
};

/**
 * \brief Dummy Document
 */
class DummyDocument  final : public Document
{
public:
    DummyDocument(std::string id, std::string name,  std::string public_part, std::string private_part){
        m_name=name;
        m_id=id;
        m_type="dummy";
        m_content_public=public_part;
        m_content_private=private_part;
    };

};
#endif
