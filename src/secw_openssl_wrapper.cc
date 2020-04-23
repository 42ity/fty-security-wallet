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

/*
@header
    secw_openssl_wrapper - Openssl wrapper for crypto
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

#include <vector>

#include <cstdio>
#include <cstring>
#include <memory>

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace secw
{
    ByteField strToBytes(const std::string & str)
    {
      ByteField data(str.begin(), str.end());
      return data;
    }

    std::string bytesToStr(const ByteField & data)
    {
      std::string str(data.begin(), data.end());

      return str;
    }

    void clean(ByteField & data)
    {
      OPENSSL_cleanse(&data[0], data.capacity() * sizeof(data[0]));
    }

    void clean(std::string & str)
    {
      OPENSSL_cleanse(const_cast<void*>(static_cast<const void *>(str.c_str())), str.capacity() * sizeof(*str.c_str()));
      str.resize(0);
    }

    ByteField randomVector(size_t nbBytes)
    {
      ByteField rndVector(nbBytes);
      if (RAND_bytes(&rndVector[0], nbBytes) == 0)
      {
        throw std::runtime_error("Unable to generate a strong random bytes array");
      }

      return rndVector;
    }

    std::string base64Encode(const ByteField & data)
    {
      if(data.empty())
      {
        return "";
      }

      BIO *b64 = BIO_new(BIO_f_base64()); // create BIO to perform base64
      BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

      BIO *mem = BIO_new(BIO_s_mem()); // create BIO that holds the result
      BIO_push(b64, mem);

      BIO_write(b64, &data[0], data.size());
      (void)BIO_flush(b64);

      // get a pointer to mem's data
      BUF_MEM *bptr;
      BIO_get_mem_ptr(b64, &bptr);

      // Convert to string
      std::string returnVal(bptr->data, bptr->length);

      BIO_free_all(b64);

      return returnVal;
    }

    ByteField base64Decode(const std::string & encodedData, const size_t off, const size_t count)
    {
      BIO *b64, *bmem;

      ByteField base64( encodedData.length() );

      b64 = BIO_new(BIO_f_base64());
      bmem = BIO_new_mem_buf( const_cast<char*>(encodedData.c_str() + off), count);
      bmem = BIO_push(b64, bmem);

      BIO_set_flags(bmem, BIO_FLAGS_BASE64_NO_NL);
      size_t sizeData = BIO_read(bmem, base64.data(), encodedData.length());

      BIO_free_all(b64);

      base64.resize(sizeData);
      return base64;
    }

    ByteField generateDigest(const ByteField & data, const EVP_MD * pEvpMd)
    {
      // Build the returned vector
      ByteField digest(EVP_MAX_MD_SIZE);
      unsigned int digestSize = 0;
      
      // Compute the digest
      EVP_MD_CTX * md5ctx = EVP_MD_CTX_create();
      EVP_DigestInit_ex(md5ctx, pEvpMd, NULL);
      EVP_DigestUpdate(md5ctx, &data[0], data.size());
      EVP_DigestFinal_ex(md5ctx, &digest[0], &digestSize);
      EVP_MD_CTX_destroy(md5ctx);

      // Adjust the returned vector size with the digest size
      digest.resize(digestSize);
      return digest;
    }

    ByteField generateMD5Digest(const ByteField & data)
    {
      return generateDigest(data, EVP_md5());
    }

    ByteField generateSHA256Digest(const ByteField & data)
    {
      return generateDigest(data, EVP_sha256());
    }

    ByteField Aes256cbcEncrypt(const ByteField & data, const ByteField & key, const ByteField & iv)
    {
      if (key.size() != 32)
      {
        throw std::invalid_argument("Invalid key size");
      }
      if (iv.size() != IV_BYTE_SIZE)
      {
        throw std::invalid_argument("Invalid initial vector size");
      }

      /* Create and initialise the context */
      EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
      int len;
      size_t cipherDataLen;

      ByteField cipherData(data.size() + 64);
      /* Initialise the encryption operation. IMPORTANT - ensure you use a key
      * and IV size appropriate for your cipher
      * In this example we are using 256 bit AES (i.e. a 256 bit key). The
      * IV size for *most* modes is the same as the block size. For AES this
      * is 128 bits */
      EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, &key[0], &iv[0]);
      
      /* Provide the message to be encrypted, and obtain the encrypted output.
      * EVP_EncryptUpdate can be called multiple times if necessary
      */
      EVP_EncryptUpdate(ctx, &cipherData[0], &len, &data[0], data.size());
      cipherDataLen = len;
      
      /* Finalise the encryption. Further cipherData bytes may be written at
      * this stage.
      */
      EVP_EncryptFinal_ex(ctx, &cipherData[cipherDataLen], &len);
      cipherDataLen += len;

      /* Clean up */
      EVP_CIPHER_CTX_free(ctx);


      cipherData.resize(cipherDataLen);
      return cipherData;
    }

    ByteField Aes256cbcDecrypt(const ByteField & cipherData, const ByteField & key, const ByteField & iv)
    {
      if (key.size() != 32)
      {
        throw std::invalid_argument("Invalid key size");
      }
      if (iv.size() != IV_BYTE_SIZE)
      {
        throw std::invalid_argument("Invalid initial vector size");
      }
      if (cipherData.empty() || (key.size() != 32) )
      {
        throw std::invalid_argument("Empty cyphered binary");
      }

      ByteField plainData(cipherData.size() + 64);

     /* Create and initialise the context */
      EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

      int len;
      size_t plainDataLen;
      
      /* Initialise the decryption operation. IMPORTANT - ensure you use a key
      * and IV size appropriate for your cipher
      * In this example we are using 256 bit AES (i.e. a 256 bit key). The
      * IV size for *most* modes is the same as the block size. For AES this
      * is 128 bits */
      EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data());

      EVP_DecryptUpdate(ctx, plainData.data(), &len, cipherData.data(), cipherData.size());

      plainDataLen = len;

      /* Finalise the decryption. Further plaintext bytes may be written at
      * this stage.
      */
      EVP_DecryptFinal_ex(ctx, &plainData[plainDataLen], &len);
      plainDataLen += len;

      /* Clean up */
      EVP_CIPHER_CTX_free(ctx);

      plainData.resize(plainDataLen);

      return plainData;
    }
} // secw


