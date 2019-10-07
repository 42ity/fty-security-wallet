/*  =========================================================================
    secw_internal_certificate - Document parsers for internal certificate document

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

#ifndef SECW_INTERNAL_CERTIFICATE_H_INCLUDED
#define SECW_INTERNAL_CERTIFICATE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  @interface
//  Create a new secw_internal_certificate
FTY_SECURITY_WALLET_EXPORT secw_internal_certificate_t *
    secw_internal_certificate_new (void);

//  Destroy the secw_internal_certificate
FTY_SECURITY_WALLET_EXPORT void
    secw_internal_certificate_destroy (secw_internal_certificate_t **self_p);


//  @end

#ifdef __cplusplus
}
#endif

#endif
