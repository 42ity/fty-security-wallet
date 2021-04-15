/*  =========================================================================
	secw_helpers - List of helper functions use a bit everywhere

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
	secw_helpers - List of helper functions use a bit everywhere
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

#include <memory>
#include <sstream>
#include <iostream>
#include <cxxtools/jsonserializer.h>
#include <cxxtools/jsondeserializer.h>

namespace secw
{
  cxxtools::SerializationInfo deserialize(const std::string & json)
  {
    cxxtools::SerializationInfo si;

    try
    {
      std::stringstream input;
      input << json;
      cxxtools::JsonDeserializer deserializer(input);
      deserializer.deserialize(si);
    }
    catch(const std::exception& e)
    {
      throw SecwProtocolErrorException("Error in the json from server: "+std::string(e.what()));
    }

    return si;

  }

  std::string serialize(const cxxtools::SerializationInfo & si)
  {
    std::string returnData("");

    try
    {
      std::stringstream output;
      cxxtools::JsonSerializer serializer(output);
      serializer.serialize(si);

      returnData = output.str();
    }
    catch(const std::exception& e)
    {
      throw SecwException("Error while creating json "+std::string(e.what()));
    }

    return returnData;
  }

  std::string encrypt(const std::string & plainData, const std::string & passphrase)
  {
    // Convert data to binary
    ByteField plainBinary = strToBytes(plainData);
    // Generate key and initial vector
    ByteField key = generateSHA256Digest(strToBytes(passphrase));
    ByteField iv = randomVector(IV_BYTE_SIZE);
    std::string cyphered = base64Encode(iv);
    cyphered += ":";
    cyphered += base64Encode(Aes256cbcEncrypt(plainBinary, key, iv));
    // CAUTION: need to clean memory to avoid key and init vector stay in the
    // heap after deallocation
    clean(iv);
    clean(key);
    // Return cyphered string
    return cyphered;
  }

  std::string decrypt(const std::string & encryptedData, const std::string & passphrase)
  {
    // If input is empty string do not try to decypher and return an empty string
    if( encryptedData.empty() )
    {
      return "";
    }

    // Ensure there is a ':' after initial vector
    if ((encryptedData.length() <= IV_BASE64_SIZE) || (encryptedData[IV_BASE64_SIZE] != ':'))
    {
      throw std::invalid_argument("Invalid cyphered format");
    }
    // Compute key from passphrase
    ByteField key = generateSHA256Digest(strToBytes(passphrase));
    // Extract init vector
    ByteField iv = base64Decode(encryptedData, 0, IV_BASE64_SIZE);
    // Decrypt
    ByteField plainBinary = Aes256cbcDecrypt(base64Decode(encryptedData, IV_BASE64_SIZE + 1), key, iv);
    // CAUTION: need to clean memory to avoid key and init vector stay in the
    // heap after deallocation
    clean(iv);
    clean(key);
    // Convert binary to string
    return bytesToStr(plainBinary);

  }

  bool hasCommonUsageIds( const std::set<std::string> &  usages1, const std::set<std::string> &  usages2)
  {
    bool matching = false;

    for(const auto &  usage1 : usages1)
    {
      if(usages2.count(usage1) != 0)
      {
        matching = true;
        break;
      }
    }
    return matching;
  }

} //namespace secw

