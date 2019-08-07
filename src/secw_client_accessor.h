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
#include <mutex>
#include <functional>
#include <condition_variable>

namespace secw
{
  using ClientId = std::string;

  using CreatedCallback = std::function<void(const std::string&, DocumentPtr)> ;
  using UpdatedCallback = std::function<void(const std::string&, DocumentPtr, DocumentPtr)> ;
  using DeletedCallback = std::function<void(const std::string&, DocumentPtr)> ;
  using StartedCallback = std::function<void()>;

  class ClientAccessor
  {
  public:
    explicit ClientAccessor(const ClientId & clientId,
                uint32_t timeout,
                const std::string & endPoint);

    ~ClientAccessor();

    std::vector<std::string> sendCommand(const std::string & command, const std::vector<std::string> & frames) const;

    void setCallbackOnUpdate(UpdatedCallback updatedCallback = nullptr);
    void setCallbackOnCreate(CreatedCallback createdCallback= nullptr);
    void setCallbackOnDelete(DeletedCallback deletedCallback= nullptr);
    void setCallbackOnStart(StartedCallback startedCallback= nullptr);

  private:
    ClientId m_clientId;
    uint32_t m_timeout;
    std::string m_endPoint;

    //callbacks
    UpdatedCallback m_updatedCallback;
    CreatedCallback m_createdCallback;
    DeletedCallback m_deletedCallback;
    StartedCallback m_startedCallback;

    //thread which handle notification and call the correct callback.
    std::thread m_notificationThread;
    bool m_stopRequested = false;
    void notificationHandler();
    std::mutex m_handlerFunctionStarting;
    std::condition_variable m_handlerFunctionThreadStarted;
    std::mutex m_handlerFunctionLock;

    //funtions to start or stop the thread
    void updateNotificationThread();

  };
} //namespace secw

//  @interface
std::vector<std::pair<std::string,bool>> secw_client_accessor_test();

#endif
