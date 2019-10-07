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

/*
@header
    secw_internal_certificate - Document parsers for internal certificate document
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

//  Structure of our class

struct _secw_internal_certificate_t {
    int filler;     //  Declare class properties here
};


//  --------------------------------------------------------------------------
//  Create a new secw_internal_certificate

secw_internal_certificate_t *
secw_internal_certificate_new (void)
{
    secw_internal_certificate_t *self = (secw_internal_certificate_t *) zmalloc (sizeof (secw_internal_certificate_t));
    assert (self);
    //  Initialize class properties here
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the secw_internal_certificate

void
secw_internal_certificate_destroy (secw_internal_certificate_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        secw_internal_certificate_t *self = *self_p;
        //  Free class properties here
        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}

