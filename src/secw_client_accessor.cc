/*  =========================================================================
    secw_client_accessor - Accessor to return documents from the agent

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

/*
@header
    secw_client_accessor - Accessor to return documents from the agent
@discuss
@end
*/

#include "fty_security_wallet_classes.h"
#include "secw_client_accessor.h"

#include <sys/types.h>
#include <gnu/libc-version.h>
#include <unistd.h>

//gettid() is available since glibc 2.30
#if ((__GLIBC__ < 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ < 30))
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

#include <iomanip>
#include <sstream>

#include "secw_helpers.h"


using namespace std::placeholders;

namespace secw
{
  ClientAccessor::ClientAccessor( fty::SyncClient & requestClient)
   : m_requestClient(requestClient), m_ptrStreamClient(nullptr)
  {}

  ClientAccessor::ClientAccessor( fty::SyncClient & requestClient, fty::StreamSubscriber & subscriberClient)
   : m_requestClient(requestClient), m_ptrStreamClient(&subscriberClient)
  {}

  ClientAccessor:: ~ClientAccessor()
  {
    //unsubscribe if needed
    if(m_isRegistered && m_ptrStreamClient)
    {
      m_ptrStreamClient->unsubscribe(m_registrationId);
    }
  }

  std::vector<std::string> ClientAccessor::sendCommand(const std::string & command, const std::vector<std::string> & frames) const
  {
    std::vector<std::string> payload = {command};
    std::copy(frames.begin(), frames.end(), back_inserter(payload));

    std::vector<std::string> receivedFrames = m_requestClient.syncRequestWithReply(payload);

    //check if the first frame we get is an error
    if(receivedFrames[0] == "ERROR")
    {
      //It's an error and we will throw directly the exceptions
      if(receivedFrames.size() == 2)
      {
        SecwException::throwSecwException(receivedFrames.at(1));
      }
      else
      {
        throw SecwProtocolErrorException("Missing data for error");
      }

    }
    return receivedFrames;
  }

  void ClientAccessor::updateNotificationThread()
  {
    bool shouldBeRunning = false;

    //1. Get to know if we should run or not
    {
      std::unique_lock<std::mutex> lock(m_handlerFunctionLock);
      shouldBeRunning = ((m_createdCallback) || (m_updatedCallback) || (m_deletedCallback) || (m_startedCallback));
    }

    //check if we need to subscribe
    if(shouldBeRunning && !m_isRegistered)
    {
      m_registrationId = m_ptrStreamClient->subscribe(std::bind(&ClientAccessor::callbackHandler, this, _1));
      m_isRegistered = true;
    }

    //check if we need to unsubscribe
    if(shouldBeRunning && !m_isRegistered)
    {
      m_ptrStreamClient->unsubscribe(m_registrationId);
      m_isRegistered = false;
    }

  }

  void ClientAccessor::setCallbackOnUpdate(UpdatedCallback updatedCallback)
  {
    if(!m_ptrStreamClient) return; // no stream listener

    std::cerr << "Set callback UPDATED" << std::endl;
    //1. Set the handler
    {
      //lock the mutex to set the handler
      std::unique_lock<std::mutex> lock(m_handlerFunctionLock);
      m_updatedCallback = updatedCallback;
    }

    //2. Update thread if needed
    updateNotificationThread();

  }

  void ClientAccessor::setCallbackOnCreate(CreatedCallback createdCallback)
  {
    if(!m_ptrStreamClient) return; // no stream listener

    //1. Set the handler
    {
      //lock the mutex to set the handler
      std::unique_lock<std::mutex> lock(m_handlerFunctionLock);
      m_createdCallback = createdCallback;
    }

    //2. Update thread if needed
    updateNotificationThread();

  }

  void ClientAccessor::setCallbackOnDelete(DeletedCallback deletedCallback)
  {
    if(!m_ptrStreamClient) return; // no stream listener

    //1. Set the handler
    {
      //lock the mutex to set the handler
      std::unique_lock<std::mutex> lock(m_handlerFunctionLock);
      m_deletedCallback = deletedCallback;
    }

    //2. Update thread if needed
    updateNotificationThread();
  }

  void ClientAccessor::setCallbackOnStart(StartedCallback startedCallback)
  {
    if(!m_ptrStreamClient) return; // no stream listener

    //1. Set the handler
    {
      //lock the mutex to set the handler
      std::unique_lock<std::mutex> lock(m_handlerFunctionLock);
      m_startedCallback = startedCallback;
    }

    //2. Update thread if needed
    updateNotificationThread();
  }

  void ClientAccessor::callbackHandler( const std::vector<std::string> & payload)
  {
       try
        {
          //treat only the payload with good format (1 frame)
          if (payload.size() == 1)
          {
            cxxtools::SerializationInfo si = deserialize(payload[0]);

            std::string action = "";
            si.getMember ("action") >>= action;


            if (action == "CREATED")
            {
              //lock the mutex and check if we have a handler
              std::unique_lock<std::mutex> lock(m_handlerFunctionLock);

              if(m_createdCallback) // we have an handler, we extract the data
              {
                std::string portfolio;
                DocumentPtr new_data;

                si.getMember ("portfolio") >>= portfolio;
                si.getMember ("new_data") >>= new_data;

                m_createdCallback (portfolio, new_data);
              }

            }
            else if (action == "UPDATED")
            {
              //lock the mutex and check if we have a handler
              std::unique_lock<std::mutex> lock(m_handlerFunctionLock);

              if(m_updatedCallback) // we have an handler, we extract the data
              {
                std::string portfolio;
                DocumentPtr new_data, old_data;

                si.getMember ("portfolio") >>= portfolio;
                si.getMember ("old_data") >>= old_data;
                si.getMember ("new_data") >>= new_data;

                m_updatedCallback (portfolio, old_data, new_data);
              }
            }
            else if (action == "DELETED")
            {
              //lock the mutex and check if we have a handler
              std::unique_lock<std::mutex> lock(m_handlerFunctionLock);

              if(m_deletedCallback) // we have an handler, we extract the data
              {
                std::string portfolio;
                DocumentPtr new_data, old_data;

                si.getMember("portfolio") >>= portfolio;
                si.getMember("old_data") >>= old_data;

                m_deletedCallback(portfolio, old_data);
              }
            }
            else if (action == "STARTED")
            {
              //lock the mutex and check if we have a handler
              std::unique_lock<std::mutex> lock(m_handlerFunctionLock);

              if(m_startedCallback) // we have an handler
              {
                m_startedCallback();
              }
            }

            //end of handlers
          }
          else
          {
            log_warning("Unknown payload received");
          }

        }
        catch(const std::exception& e)
        {
          log_error("Error during security wallet notification processing: %s", e.what());
        }
        catch (...) //Show Must Go On => Log errors and continue
        {
          log_error("Error during security wallet notification processing: unknown error");
        }
  }

} //namespace secw
