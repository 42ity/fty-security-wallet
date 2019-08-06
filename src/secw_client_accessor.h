/*  =========================================================================
    secw_client_accessor - Accessor to return documents from the agent

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

#ifndef SECW_CLIENT_ACCESSOR_H_INCLUDED
#define SECW_CLIENT_ACCESSOR_H_INCLUDED

#include "secw_document.h"
#include "secw_exception.h"

#include <cxxtools/serializationinfo.h>
#include <thread>

namespace secw
{
  using ClientId = std::string;

  class ClientAccessor
  {
  public:
    explicit ClientAccessor(const ClientId & clientId,
                uint32_t timeout,
                const std::string & endPoint);

    ~ClientAccessor();

    std::vector<std::string> sendCommand(const std::string & command, const std::vector<std::string> & frames) const;

    //void setCallbackOnUpdate (std::function<void(const std::string&, DocumentPtr, DocumentPtr)>& updateCallback=nullptr);
    //{ m_updateCallback = updateCallback; }
    void setCallbackOnCreate (std::function<void(const std::string&, DocumentPtr)>& createCallback=nullptr);
    //{ m_createCallback = createCallback; }
    //void setCallbackOnDelete (std::function<void(const std::string&, DocumentPtr)>& deleteCallback=nullptr);
    //{ m_deleteCallback = deleteCallback; }
    //void setCallbackOnStart (std::function<void(void)>& startCallback=nullptr);
    //{ m_startCallback = startCallback; }

  private:
    ClientId m_clientId;
    uint32_t m_timeout;
    std::string m_endPoint;

    //callbacks
    std::function<void(const std::string&, DocumentPtr, DocumentPtr)> m_updateCallback;
    std::function<void(const std::string&, DocumentPtr)> m_createCallback;
    std::function<void(const std::string&, DocumentPtr)> m_deleteCallback;
    std::function<void(void)> m_startCallback;

    std::thread m_notificationThread;
    void notificationListener(void) noexcept;

  };
} //namespace secw

//  @interface
std::vector<std::pair<std::string,bool>> secw_client_accessor_test();

#endif
