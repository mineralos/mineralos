#!/bin/sh

BOARD_DIR="$(dirname $0)"
GENIMAGE_CFG="${BOARD_DIR}/genimage.cfg"
GENIMAGE_TMP="${BUILD_DIR}/genimage.tmp"
OUTPUT_DIR="${BR2_EXTERNAL}/board/g19/images"
MKIMAGE="${HOST_DIR}/bin/mkimage"
TARGET_HWREVISION="${TARGET_DIR}/etc/hwrevision"
T1_HWREVISION="${BR2_EXTERNAL}/package/cgminer/hwrevision_t1"
T2_HWREVISION="${BR2_EXTERNAL}/package/cgminer/hwrevision_t2"
A8PLUS_HWREVISION="${BR2_EXTERNAL}/package/cgminer4t4/hwrevision_a8plus"
B29_HWREVISION="${BR2_EXTERNAL}/package/sgminer4a11/hwrevision_b29"
B52_HWREVISION="${BR2_EXTERNAL}/package/sgminer4a12/hwrevision_b52"
D9_HWREVISION="${BR2_EXTERNAL}/package/sgminer4a11/hwrevision_d9"
S11_HWREVISION="${BR2_EXTERNAL}/package/sgminer4a12/hwrevision_s11"
A5_HWREVISION="${BR2_EXTERNAL}/package/sgminer4a7/hwrevision_a5"
A6_HWREVISION="${BR2_EXTERNAL}/package/cgminer4a6/hwrevision_a6"
HARDWARE_COMPAT_IM=""
if cmp --silent "${TARGET_HWREVISION}" "${T1_HWREVISION}" ; then
	MODEL_PREFIX="t1"
	HARDWARE_COMPAT="t1.g19"
elif cmp --silent "${TARGET_HWREVISION}" "${T2_HWREVISION}" ; then
	MODEL_PREFIX="t2"
	HARDWARE_COMPAT="t2.g19"
elif cmp --silent "${TARGET_HWREVISION}" "${A8PLUS_HWREVISION}" ; then
	MODEL_PREFIX="a8+"
	HARDWARE_COMPAT="a8+.g19"
elif cmp --silent "${TARGET_HWREVISION}" "${B29_HWREVISION}" ; then
	MODEL_PREFIX="b29"
	HARDWARE_COMPAT="b29.g19"
	HARDWARE_COMPAT_IM="d9.g19"
elif cmp --silent "${TARGET_HWREVISION}" "${D9_HWREVISION}" ; then
	MODEL_PREFIX="d9"
	HARDWARE_COMPAT="b29.g19"
	HARDWARE_COMPAT_IM="d9.g19"
elif cmp --silent "${TARGET_HWREVISION}" "${B52_HWREVISION}" ; then
	MODEL_PREFIX="b52"
	HARDWARE_COMPAT="b52.g19"
	HARDWARE_COMPAT_IM="s11.g19"
elif cmp --silent "${TARGET_HWREVISION}" "${S11_HWREVISION}" ; then
	MODEL_PREFIX="s11"
	HARDWARE_COMPAT="b52.g19"
	HARDWARE_COMPAT_IM="s11.g19"
elif cmp --silent "${TARGET_HWREVISION}" "${A5_HWREVISION}" ; then
	MODEL_PREFIX="a5"
	HARDWARE_COMPAT="a5.g19"
elif cmp --silent "${TARGET_HWREVISION}" "${A6_HWREVISION}" ; then
	MODEL_PREFIX="a6"
	HARDWARE_COMPAT="a6.g19"
else
	echo "MODEL DETECTION ERROR"
	exit 1
fi
echo "Building images for ${MODEL_PREFIX}"

SWU_VERSION="${MODEL_PREFIX}_$(date -u +%Y%m%d_%H%M%S)"
SWU_FILENAME="${SWU_VERSION}.swu"

rm -rf "${GENIMAGE_TMP}"

cp ${BOARD_DIR}/uEnv.txt ${BINARIES_DIR}
cp ${BOARD_DIR}/BOOT.bif ${BINARIES_DIR}
cp ${BOARD_DIR}/fsbl.elf ${BINARIES_DIR}
cp ${BOARD_DIR}/system.bit ${BINARIES_DIR}
cp ${BINARIES_DIR}/zynq-zc702.dtb ${BINARIES_DIR}/devicetree.dtb

genimage                               \
	--rootpath "${TARGET_DIR}"     \
	--tmppath "${GENIMAGE_TMP}"    \
	--inputpath "${BINARIES_DIR}"  \
	--outputpath "${BINARIES_DIR}" \
	--config "${GENIMAGE_CFG}"

(cd ${BINARIES_DIR} && $HOST_DIR/bin/mkbootimage ${BINARIES_DIR}/BOOT.bif ${BINARIES_DIR}/BOOT.bin)

$HOST_DIR/bin/python3 ${BOARD_DIR}/swupdate.py \
	--desc "${BOARD_DIR}/sw-description"         \
	--boot "${BINARIES_DIR}/BOOT.bin"        \
	--devicetree "${BINARIES_DIR}/devicetree.dtb"   \
	--fpga "${BOARD_DIR}/system.bit"        \
	--kernel "${BINARIES_DIR}/uImage"        \
	--rootfs "${BINARIES_DIR}/rootfs.ubi"  \
	--key "${BOARD_DIR}/priv.pem"  \
	--workdir "${BINARIES_DIR}"  \
	--out "${BINARIES_DIR}/${SWU_FILENAME}" \
	--version "${SWU_VERSION}" \
	--hwversion "${HARDWARE_COMPAT}" \
	--hwversionim "${HARDWARE_COMPAT_IM}"

# $HOST_DIR/bin/python3 ${BOARD_DIR}/update_bin.py \
# 	--cmd "${BOARD_DIR}/upgrade.cmd"         \
# 	--upgrade "${BOARD_DIR}/upgrade.py"      \
# 	--boot "${BINARIES_DIR}/BOOT.bin"        \
# 	--devicetree "${BINARIES_DIR}/zynq-zc702.dtb"   \
# 	--kernel "${BINARIES_DIR}/uImage"        \
# 	--rootfs "${BINARIES_DIR}/rootfs.jffs2"  \
# 	--out "${BINARIES_DIR}/update.bin"
