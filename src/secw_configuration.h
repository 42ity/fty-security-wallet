/*  =========================================================================
    secw_configuration - Handle the configuration of a security wallet

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

#ifndef SECW_CONFIGURATION_H_INCLUDED
#define SECW_CONFIGURATION_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  @interface
//  Create a new secw_configuration
FTY_SECURITY_WALLET_PRIVATE secw_configuration_t *
    secw_configuration_new (void);

//  Destroy the secw_configuration
FTY_SECURITY_WALLET_PRIVATE void
    secw_configuration_destroy (secw_configuration_t **self_p);


//  @end

#ifdef __cplusplus
}
#endif

#endif
