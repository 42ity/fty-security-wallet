#!/bin/bash
#
# Copyright (C) 2019 - 2020 Eaton
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#  \brief   Make sure the security wallet db is created with security wallet user
#  \author  Clement Perrette
#

SECW_USER="secw-daemon"
SECW_GROUP="secw-daemon"

SECW_DIR="/var/lib/fty/fty-security-wallet"
SECW_DB="database.json"

# Credential asset mapping is included into the security wallet daemon
CAM_DB="mapping.json"

die() {
    echo "FATAL: $*" >&2
    exit 1
}

getent passwd "${SECW_USER}" >/dev/null \
|| die "The '${SECW_USER}' user account is not defined in this system"


getent group "${SECW_GROUP}" >/dev/null \
|| die "The '${SECW_GROUP}' group account is not defined in this system"

chown ${SECW_USER}:${SECW_GROUP} "${SECW_DIR}"
chmod 0700 "${SECW_DIR}"

if [ -f "${SECW_DIR}/${SECW_DB}" ]; then
    chown ${SECW_USER}:${SECW_GROUP} "${SECW_DIR}/${SECW_DB}"
    chmod 0600 "${SECW_DIR}/${SECW_DB}"
fi

if [ -f "${SECW_DIR}/${CAM_DB}" ]; then
    chown ${SECW_USER}:${SECW_GROUP} "${SECW_DIR}/${CAM_DB}"
    chmod 0600 "${SECW_DIR}/${CAM_DB}"
fi
