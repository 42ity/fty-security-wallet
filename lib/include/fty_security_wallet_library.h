/*  =========================================================================
    fty-security-wallet - generated layer of public API

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

#ifndef FTY_SECURITY_WALLET_LIBRARY_H_INCLUDED
#define FTY_SECURITY_WALLET_LIBRARY_H_INCLUDED

//  Set up environment for the library

//  FTY_SECURITY_WALLET version macros for compile-time API detection
#define FTY_SECURITY_WALLET_VERSION_MAJOR 1
#define FTY_SECURITY_WALLET_VERSION_MINOR 0
#define FTY_SECURITY_WALLET_VERSION_PATCH 0

#define FTY_SECURITY_WALLET_MAKE_VERSION(major, minor, patch) \
    ((major) * 10000 + (minor) * 100 + (patch))
#define FTY_SECURITY_WALLET_VERSION \
    FTY_SECURITY_WALLET_MAKE_VERSION(FTY_SECURITY_WALLET_VERSION_MAJOR, FTY_SECURITY_WALLET_VERSION_MINOR, FTY_SECURITY_WALLET_VERSION_PATCH)

//  External dependencies
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
#define ACTIVE_VERSION "1.0"
#define SECURITY_WALLET_AGENT "security-wallet"
#define DEFAULT_STORAGE_DATABASE_PATH "/var/lib/fty/fty-security-wallet/database.json"
#define DEFAULT_STORAGE_CONFIGURATION_PATH "/etc/fty/fty-security-wallet/configuration.json"
#define DEFAULT_ENDPOINT "ipc://@/malamute"
#define DEFAULT_SOCKET "/tmp/secw.socket"
#define SECW_NOTIFICATIONS "_SECW_NOTIFICATIONS"
#define MAPPING_AGENT "credential-asset-mapping"
#define DEFAULT_STORAGE_MAPPING_PATH  "/etc/fty/fty-security-wallet/mapping.json"

//  Public classes
#include "cam_credential_asset_mapping.h"
#include "cam_accessor.h"
#include "cam_exception.h"
#include "secw_document.h"
#include "secw_exception.h"
#include "secw_producer_accessor.h"
#include "secw_consumer_accessor.h"
#include "secw_external_certificate.h"
#include "secw_internal_certificate.h"
#include "secw_snmpv3.h"
#include "secw_snmpv1.h"
#include "secw_user_and_password.h"
#include "fty_credential_asset_mapping_mlm_agent.h"
#include "fty_security_wallet_socket_agent.h"

#endif
