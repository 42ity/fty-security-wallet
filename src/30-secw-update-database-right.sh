#!/bin/bash

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
chmod 0771 "${SECW_DIR}"

if [ -f "${SECW_DIR}/${SECW_DB}" ]; then
    chown ${SECW_USER}:${SECW_GROUP} "${SECW_DIR}/${SECW_DB}"
    chmod 0600 "${SECW_DIR}/${SECW_DB}"
fi

if [ -f "${SECW_DIR}/${CAM_DB}" ]; then
    chown ${SECW_USER}:${SECW_GROUP} "${SECW_DIR}/${CAM_DB}"
    chmod 0600 "${SECW_DIR}/${CAM_DB}"
fi
