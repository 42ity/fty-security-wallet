/*  =========================================================================
    fty_security_wallet_server - secw broker agent

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

#ifndef FTY_SECURITY_WALLET_SERVER_H_INCLUDED
#define FTY_SECURITY_WALLET_SERVER_H_INCLUDED

#include "fty_actor.h"
#include "secw_portfolio.h"
/**
 * \brief Agent SecurityWalletServer main server actor
 */
class SecurityWalletServer final : public FtyActor
{
public:
    SecurityWalletServer(zsock_t *pipe);

private:
    bool handleMailbox(zmsg_t *message) override;
    bool handlePipe(zmsg_t *message) override;
    zmsg_t *generateErrorMsg(ZstrGuard &correlation_id, const char* err_msg);
    zmsg_t *generateErrorMsg(const char* err_msg);
    zmsg_t *handleGetDummyRequest(ZstrGuard &correlation_id, ZstrGuard &portfolio_name, bool isSecret);
    DummyStorage m_storage;
    
};

//  @interface
//  Create an fty_security_wallet_server actor
void fty_security_wallet_server(zsock_t *pipe, void *endpoint);

void fty_security_wallet_server_test (bool verbose);

#endif
