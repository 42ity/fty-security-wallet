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

/*
@header
    secw_document - Document parsers
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

std::string Document::toJSON_withSecret()
{
    std::string result="{";
    result +="\"secw_doc_type\" : \"" + m_type + "\",";
    result +="\"secw_doc_id\" : \"" + m_id + "\",";
    result += "\"secw_doc_name\" : \"" + m_name + "\",";
    result +="\"secw_doc_public\": { ";
    result +="\"" + m_type + "\" : { " + m_content_public + "}}";
    result +="\"secw_doc_secret\" :{";
    result +="\"" + m_type + "\" : { " + m_content_private + "}}";
    result +="}";
    return result;
}

std::string Document::toJSON_withoutSecret()
{
    std::string result="{";
    result +="\"secw_doc_type\" : \"" + m_type + "\",";
    result +="\"secw_doc_id\" : \"" + m_id + "\",";
    result += "\"secw_doc_name\" : \"" + m_name + "\",";
    result +="\"secw_doc_public\": { ";
    result +="\"" + m_type + "\" : { " + m_content_public + "}}";
    result +="}";
    return result;
}

