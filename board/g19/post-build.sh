#!/bin/sh

BOARD_DIR="$(dirname $0)"
FIRMWARE_VERSION=$(cat ${BR2_EXTERNAL}/.git/refs/heads/master)
BACKEND_VERSION=$(cat ${BR2_EXTERNAL}/.git/modules/overlay/var/www/webif/api/refs/heads/master)
CGMINER_VERSION=$(cat ${TARGET_DIR}/etc/cgminer_git.hash)

echo "FIRMWARE=${FIRMWARE_VERSION}" > ${TARGET_DIR}/etc/git_hashes
echo "BACKEND=${BACKEND_VERSION}" >> ${TARGET_DIR}/etc/git_hashes
echo "CGMINER=${CGMINER_VERSION}" >> ${TARGET_DIR}/etc/git_hashes

openssl rsa -in ${BOARD_DIR}/priv.pem -pubout -out ${TARGET_DIR}/etc/swupdate.pem
