/*  =========================================================================
    fty_security_wallet_classes - private header file

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

#ifndef FTY_SECURITY_WALLET_CLASSES_H_INCLUDED
#define FTY_SECURITY_WALLET_CLASSES_H_INCLUDED

//  Asserts check the invariants of methods. If they're not
//  fulfilled the program should fail fast. Therefore enforce them!
#ifdef NDEBUG
  #undef NDEBUG
  #include <assert.h>
  #define NDEBUG
#else
  #include <assert.h>
#endif

//  External dependencies
//#undef HAVE_LIBSYSTEMD
//#define HAVE_LIBSYSTEMD
#if defined (HAVE_LIBSYSTEMD)
#include <systemd/sd-daemon.h>
#endif
#include <openssl/evp.h>
#include <czmq.h>
#include <malamute.h>
#include <cxxtools/allocator.h>
#include <fty_log.h>
#include <fty_common_mlm.h>
#include <fty_common_socket.h>
#include <fty_common_messagebus.h>
#include <fty_common_dto.h>
#include <google/protobuf/stubs/common.h>
#include <fty-lib-certificate.h>

// Common defs

#define SECURITY_WALLET_AGENT "security-wallet"
#define DEFAULT_STORAGE_DATABASE_PATH  "/var/lib/fty/fty-security-wallet/database.json"
#define DEFAULT_STORAGE_CONFIGURATION_PATH  "/etc/fty/fty-security-wallet/configuration.json"
#define DEFAULT_ENDPOINT      "ipc://@/malamute"
#define DEFAULT_SOCKET      "/tmp/secw.socket"
#define SECW_NOTIFICATIONS "_SECW_NOTIFICATIONS"

#define MAPPING_AGENT "credential-asset-mapping"
#define DEFAULT_STORAGE_MAPPING_PATH  "/etc/fty/fty-security-wallet/mapping.json"
#define ACTIVE_VERSION                "1.0"

//  Internal API

#include "cam_credential_asset_mapping.h"
#include "cam_accessor.h"
#include "cam_exception.h"
#include "cam_helpers.h"
#include "cam_credential_asset_mapping_storage.h"
#include "cam_credential_asset_mapping_server.h"

#include "secw_portfolio.h"
#include "secw_document.h"
#include "secw_exception.h"
#include "secw_producer_accessor.h"
#include "secw_consumer_accessor.h"
#include "secw_external_certificate.h"
#include "secw_internal_certificate.h"
#include "secw_snmpv3.h"
#include "secw_snmpv1.h"
#include "secw_user_and_password.h"
#include "secw_helpers.h"
#include "secw_openssl_wrapper.h"
#include "secw_configuration.h"
#include "secw_client_accessor.h"
#include "secw_security_wallet.h"
#include "secw_security_wallet_server.h"

#include "fty_credential_asset_mapping_mlm_agent.h"
#include "fty_security_wallet_socket_agent.h"

#endif
