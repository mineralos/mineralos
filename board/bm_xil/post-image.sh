#!/bin/sh

BOARD_DIR="$(dirname $0)"
GENIMAGE_CFG="${BOARD_DIR}/genimage.cfg"
GENIMAGE_TMP="${BUILD_DIR}/genimage.tmp"
OUTPUT_DIR="${BR2_EXTERNAL}/board/g19/images"
MKIMAGE="${HOST_DIR}/bin/mkimage"
TARGET_HWREVISION="${TARGET_DIR}/etc/hwrevision"
S9_HWREVISION="${BR2_EXTERNAL}/package/bmminer-mix/hwrevision_s9"
HARDWARE_COMPAT_IM=""
if cmp --silent "${TARGET_HWREVISION}" "${S9_HWREVISION}" ; then
	MODEL_PREFIX="s9"
	HARDWARE_COMPAT="s9.bm_xil_1g"
	HARDWARE_COMPAT_IM="s9.bm_xil_512m"
	cp ${BOARD_DIR}/bm_btccv_fpga.bit ${BINARIES_DIR}/system.bit
else
	echo "MODEL DETECTION ERROR"
	exit 1
fi
echo "Building images for ${MODEL_PREFIX}"

SWU_VERSION="${MODEL_PREFIX}_$(date -u +%Y%m%d_%H%M%S)"
SWU_FILENAME="${SWU_VERSION}.swu"

rm -rf "${GENIMAGE_TMP}"

cp ${BOARD_DIR}/BOOT.bif ${BINARIES_DIR}
cp ${BINARIES_DIR}/zynq-zc702.dtb ${BINARIES_DIR}/devicetree1g.dtb
cp ${BINARIES_DIR}/zynq-zc702.dtb ${BINARIES_DIR}/devicetree512m.dtb

$HOST_DIR/bin/fdtput -t x ${BINARIES_DIR}/devicetree512m.dtb /memory reg 0 20000000
$HOST_DIR/bin/fdtput -t x ${BINARIES_DIR}/devicetree512m.dtb /memory linux,usable-memory 0 100000 0 1f000000

genimage                               \
	--rootpath "${TARGET_DIR}"     \
	--tmppath "${GENIMAGE_TMP}"    \
	--inputpath "${BINARIES_DIR}"  \
	--outputpath "${BINARIES_DIR}" \
	--config "${GENIMAGE_CFG}"

(cd ${BINARIES_DIR} && $HOST_DIR/bin/mkbootimage ${BINARIES_DIR}/BOOT.bif ${BINARIES_DIR}/BOOT.bin)

MIGRATE_FILENAME="${SWU_VERSION}.tar.gz"

tar -zcvf "${BINARIES_DIR}/${MIGRATE_FILENAME}" \
	-C "${BOARD_DIR}" \
	"runme.sh" \
	"fw_setenv" \
	"fw_env.config" \
	"ubi_info" \
	-C "${BINARIES_DIR}" \
	"BOOT.bin" \
	"devicetree1g.dtb" \
	"devicetree512m.dtb" \
	"system.bit" \
	"uImage" \
	"rootfs.cpio.uboot" \

$HOST_DIR/bin/python3 ${BOARD_DIR}/swupdate.py \
	--desc "${BOARD_DIR}/sw-description"         \
	--boot "${BINARIES_DIR}/BOOT.bin"        \
	--devicetree1g "${BINARIES_DIR}/devicetree1g.dtb"   \
	--devicetree512m "${BINARIES_DIR}/devicetree512m.dtb"   \
	--fpga "${BINARIES_DIR}/system.bit"        \
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
