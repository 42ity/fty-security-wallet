/*  =========================================================================
    secw_openssl_wrapper - Openssl wrapper for crypto

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

#ifndef SECW_OPENSSL_WRAPPER_H_INCLUDED
#define SECW_OPENSSL_WRAPPER_H_INCLUDED

#include <string>
#include <vector>

namespace secw
{
    using Byte = unsigned char;
    using ByteField = std::vector<Byte>;

    static constexpr size_t IV_SIZE = 128;
    static constexpr size_t IV_BYTE_SIZE = (IV_SIZE + 7) / 8;
    static constexpr size_t IV_BASE64_SIZE = ((IV_SIZE + 5) / 6 + 3) / 4 * 4; // Base64 digit encodes 6 bits


    // String to byte field conversion
    ByteField strToBytes(const std::string & str);
    std::string bytesToStr(const ByteField & data);

    // Security to avoid sensitive data remains in heap after deallocation
    void clean(ByteField & data);
    void clean(std::string & str);

    // Random free-size byte field generation
    ByteField randomVector(size_t nbBytes);

    //base64 tools
    std::string base64Encode(const ByteField & data);
    ByteField base64Decode(const std::string & encodedData, size_t off, size_t count = -1);

    //digests
    ByteField generateMD5Digest(const ByteField & data);
    ByteField generateSHA256Digest(const ByteField & data);

    //AES 256 cbc
    ByteField Aes256cbcEncrypt(const ByteField & data, const ByteField & key, const ByteField & iv);
    ByteField Aes256cbcDecrypt(const ByteField & cipherData, const ByteField & key, const ByteField & iv);

} //secw

#endif
