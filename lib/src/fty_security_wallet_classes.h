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

//  Public classes, deps and defs
#include "../include/fty_security_wallet_library.h"

//  Private classes
#include "cam_helpers.h"
#include "cam_credential_asset_mapping_storage.h"
#include "cam_credential_asset_mapping_server.h"
#include "secw_helpers.h"
#include "secw_portfolio.h"
#include "secw_openssl_wrapper.h"
#include "secw_configuration.h"
#include "secw_client_accessor.h"
#include "secw_security_wallet.h"
#include "secw_security_wallet_server.h"

#endif
