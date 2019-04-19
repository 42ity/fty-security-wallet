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

#include "secw_helpers.h"

namespace secw
{
  ClientAccessor::ClientAccessor(const ClientId & clientId,
                  uint32_t timeout,
                  const std::string & endPoint):
    m_clientId(clientId),
    m_timeout(timeout),
    m_client(mlm_client_new())
  {
    mlm_client_connect(m_client, endPoint.c_str(), m_timeout, m_clientId.c_str());
  }

  ClientAccessor:: ~ClientAccessor()
  {
    mlm_client_destroy(&m_client);
  }

  std::vector<std::string> ClientAccessor::sendCommand(const std::string & command, const std::vector<std::string> & frames) const
  {
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

    //send the message
    mlm_client_sendto (m_client, SECURITY_WALLET_AGENT, "REQUEST", NULL, m_timeout, &request);

    //Get the reply
    ZmsgGuard recv(mlm_client_recv (m_client));

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

} //namespace secw
