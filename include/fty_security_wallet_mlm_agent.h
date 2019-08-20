/*  =========================================================================
    fty_security_wallet_mlm_agent - Security Wallet malamute agent

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

#ifndef FTY_SECURITY_WALLET_MLM_AGENT_H_INCLUDED
#define FTY_SECURITY_WALLET_MLM_AGENT_H_INCLUDED

#include "fty_common_mlm_agent.h"

//  @interface
//  Create an security wallet actor
void 
    fty_security_wallet_mlm_agent(zsock_t *pipe, void *endpoint);

//  Self test
void
    fty_security_wallet_mlm_agent_test (bool verbose);

#endif



