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

namespace secw
{
  ClientAccessor::ClientAccessor(const ClientId & clientId,
                  uint32_t timeout,
                  const std::string & endPoint):
    m_clientId(clientId),
    m_timeout(timeout),
    m_endPoint(endPoint)
  {}

  ClientAccessor:: ~ClientAccessor()
  {
    //1. Set the handlers to null
    {
      //lock the mutex to set the handler
      std::unique_lock<std::mutex> lock(m_handlerFunctionLock);

      m_updatedCallback = nullptr;
      m_createdCallback = nullptr;
      m_deletedCallback = nullptr;
      m_startedCallback = nullptr;
    }

    //2. Update thread
    updateNotificationThread();
  }

  std::vector<std::string> ClientAccessor::sendCommand(const std::string & command, const std::vector<std::string> & frames) const
  {
    mlm_client_t * client = mlm_client_new();

    if(client == NULL)
    {
      mlm_client_destroy(&client);
      throw SecwMalamuteClientIsNullException();
    }

    //create a unique sender id: <clientId>.[thread id in hexa]
    pid_t threadId = gettid();
    
    std::stringstream ss;
    ss << m_clientId << "." << std::setfill('0') << std::setw(sizeof(pid_t)*2) << std::hex << threadId;

    std::string uniqueId = ss.str();
    
    int rc = mlm_client_connect (client, m_endPoint.c_str(), m_timeout, uniqueId.c_str());
    
    if (rc != 0)
    {
      mlm_client_destroy(&client);
      throw SecwMalamuteConnectionFailedException();
    }

    //Prepare the request:
    zmsg_t *request = zmsg_new();
    ZuuidGuard  zuuid(zuuid_new ());
    zmsg_addstr (request, zuuid_str_canonical (zuuid));

    //add the command
    zmsg_addstr (request, command.c_str());

    //add all the extra frames
    for(const std::string & frame : frames )
    {
      zmsg_addstr (request, frame.c_str());
    }

    if(zsys_interrupted)
    {
      zmsg_destroy(&request);
      mlm_client_destroy(&client);
      throw SecwMalamuteInterruptedException();
    }

    //send the message
    mlm_client_sendto (client, SECURITY_WALLET_AGENT, "REQUEST", NULL, m_timeout, &request);

    if(zsys_interrupted)
    {
      zmsg_destroy(&request);
      mlm_client_destroy(&client);
      throw SecwMalamuteInterruptedException();
    }

    //Get the reply
    ZmsgGuard recv(mlm_client_recv (client));
    mlm_client_destroy(&client);

    //Get number of frame all the frame
    size_t numberOfFrame = zmsg_size(recv);

    if(numberOfFrame < 2)
    {
      throw SecwProtocolErrorException("Wrong number of frame");
    }

    //Check the message
    ZstrGuard str(zmsg_popstr (recv));
    if(!streq (str, zuuid_str_canonical (zuuid)))
    {
      throw SecwProtocolErrorException("Mismatch correlation id");
    }

    std::vector<std::string> receivedFrames;

    //we unstack all the other frame starting by the 2rd one.
    for(size_t index = 1; index < numberOfFrame; index++)
    {
      ZstrGuard frame( zmsg_popstr(recv) );
      receivedFrames.push_back( std::string(frame.get()) );
    }

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

  void ClientAccessor::notificationHandler()
  {
    std::cerr << "notificationHandler: init..." << std::endl;
    mlm_client_t *client = mlm_client_new ();

    if(client == NULL)
    {
        mlm_client_destroy(&client);
        m_handlerFunctionStarting.unlock();
        throw SecwMalamuteClientIsNullException();
    }

    //create a unique id: <m_clientId>-SECW_NOTIFICATIONS.[thread id in hexa]
    pid_t threadId = gettid();

    std::stringstream ss;
    ss << m_clientId  << "-" << SECW_NOTIFICATIONS << "." << std::setfill('0') << std::setw(sizeof(pid_t)*2) << std::hex << threadId;

    std::string uniqueId = ss.str();

    int rc = mlm_client_connect (client, m_endPoint.c_str(), 1000, uniqueId.c_str());

    if (rc != 0)
    {
        mlm_client_destroy(&client);
        m_handlerFunctionStarting.unlock();
        throw SecwMalamuteConnectionFailedException();
    }

    rc = mlm_client_set_consumer (client, SECW_NOTIFICATIONS, ".*");
    if (rc != 0)
    {
        mlm_client_destroy (&client);
        m_handlerFunctionStarting.unlock();
        throw SecwMalamuteInterruptedException();
    }

    zpoller_t *poller = zpoller_new (mlm_client_msgpipe (client), NULL);

    std::cerr << "notificationHandler: init... Done." << std::endl;

    m_handlerFunctionStarting.unlock();

    while (!zsys_interrupted)
    {
      void *which = zpoller_wait (poller, -1);
      if (which == mlm_client_msgpipe (client))
      {
        //check if we need to leave the loop
        if(m_stopRequested)
        {
          std::cerr << "notificationHandler: Stopping..." << std::endl;
          break;
        }

        ZmsgGuard msg (mlm_client_recv (client));

        //Get number of frame all the frame
        size_t numberOfFrame = zmsg_size(msg);

        try
        {
          //treat only the notification
          if (numberOfFrame == 1)
          {
            ZstrGuard notification (zmsg_popstr (msg));

            std::cerr << "notificationHandler: Notification received." << std::endl;

            cxxtools::SerializationInfo si = deserialize(std::string(notification.get()));
            
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
          
        }
        catch(const std::exception& e)
        {
          log_error("Error during notification processing: %s", e.what());
        }
        catch (...) //Show Must Go On => Log errors and continue
        {
          log_error("Error during notification processing: unknown error");
        }

      }
    }

    zpoller_destroy(&poller);
    mlm_client_destroy(&client);

    std::cerr << "notificationHandler: Stopping... Done." << std::endl;
      
  }

  //function which start or stop the thread if needed
  void ClientAccessor::updateNotificationThread()
  {
    bool shouldBeRunning = false;

    //1. Get to know if we should run or not
    {
      std::unique_lock<std::mutex> lock(m_handlerFunctionLock);
      shouldBeRunning = ((m_createdCallback) || (m_updatedCallback) || (m_deletedCallback) || (m_startedCallback));
    }

    //2. update the thread
    if(shouldBeRunning && (!m_notificationThread.joinable()))
    {
      std::cerr << "Start the callback handler ...." << std::endl;

      //start
      m_stopRequested = false;
      m_handlerFunctionStarting.lock(); //the notification handler will release the mutex when the init will be finish.

      m_notificationThread = std::thread(std::bind(&ClientAccessor::notificationHandler, this));

      //wait until it's started
      m_handlerFunctionStarting.lock();
      m_handlerFunctionStarting.unlock();
      std::cerr << "Start the callback handler .... Done." << std::endl;
    }
    else if((!shouldBeRunning) && m_notificationThread.joinable())
    {
      //stop
      m_stopRequested = true;

      std::cerr << "Stop the callback handler ...." << std::endl;

      //send a signal to the bus
      try
      {
        mlm_client_t * client = mlm_client_new();

        if(client == NULL)
        {
          mlm_client_destroy(&client);
          throw SecwMalamuteClientIsNullException();
        }

        //create a unique id: <m_clientId>+SECW_NOTIFICATIONS.[thread id in hexa]
        pid_t threadId = gettid();

        std::stringstream ss;
        ss << m_clientId  << "+" << SECW_NOTIFICATIONS << "." << std::setfill('0') << std::setw(sizeof(pid_t)*2) << std::hex << threadId;

        std::string uniqueId = ss.str();

        int rc = mlm_client_connect (client, m_endPoint.c_str(), 1000, uniqueId.c_str());

        if (rc != 0)
        {
          mlm_client_destroy(&client);
          throw SecwMalamuteConnectionFailedException();
        }

        rc = mlm_client_set_producer (client, SECW_NOTIFICATIONS);
        if (rc != 0)
        {
            mlm_client_destroy (&client);
            throw SecwMalamuteInterruptedException();
        }

        zmsg_t *notification = zmsg_new ();
        zmsg_addstr (notification, "");
        rc = mlm_client_send (client, "SYNC", &notification);

        if (rc != 0)
        {
          zmsg_destroy(&notification);
          mlm_client_destroy(&client);
          throw SecwMalamuteInterruptedException();
        }

        mlm_client_destroy (&client);
      }
      catch(const std::exception& e)
      {
        throw std::runtime_error("Impossible to stop the notification handler: "+std::string(e.what()));
      }

      //wait until the thread finish
      m_notificationThread.join();

      std::cerr << "Stop the callback handler .... Done." << std::endl;
    }

  }

  void ClientAccessor::setCallbackOnUpdate(UpdatedCallback updatedCallback)
  {
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
    std::cerr << "Set callback CREATED" << std::endl;

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
    std::cerr << "Set callback DELETED" << std::endl;

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
    std::cerr << "Set callback STARTED" << std::endl;

    //1. Set the handler
    {
      //lock the mutex to set the handler
      std::unique_lock<std::mutex> lock(m_handlerFunctionLock);
      m_startedCallback = startedCallback;
    }

    //2. Update thread if needed
    updateNotificationThread();
  }

} //namespace secw
