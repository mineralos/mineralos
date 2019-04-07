#!/bin/sh -e

path=$(pwd)

if [ ! -d /mnt/upgrade ];
then
	mkdir /mnt/upgrade
fi

if [ -e /usr/bin/ctrl_bd ]; then
ret=`cat /usr/bin/ctrl_bd | grep "XILINX" | wc -l`
else
ret=0
fi

if [ ! -f /config/mac ];then
    id_h=`devmem 0x43c000f0`
    id_l=`devmem 0x43c000f4`
    echo $id_h$id_l > /tmp/chip_id
    mac_string=`openssl rand -rand /tmp/chip_id -hex 6`
    mac0=`echo ${mac_string:0:2} | awk 'NR==1 {s=and($NF,0xfe); printf("%02x",s)}'`
    mac1=${mac_string:2:2}
    mac2=${mac_string:4:2}
    mac3=${mac_string:6:2}
    mac4=${mac_string:8:2}
    mac5=${mac_string:10:2}
    mac="$mac0:$mac1:$mac2:$mac3:$mac4:$mac5"
else
    mac=`cat /config/mac`
fi

envbootcmd='if env exists image_flag; then tempmac=${ethaddr} && tempimageflag=${image_flag} && temphwrevision=${hwrevision} && env default -a && setenv image_flag $tempimageflag && setenv ethaddr $tempmac && setenv hwrevision $temphwrevision && saveenv && saveenv && reset; fi; nand read 0x400000 0x580000 0x400000 && fpga loadb 0 0x400000 0x400000 && nand read 0x2000000 0x480000 0x80000 && nand read 0x2080000 0xd80000 0x500000 && nand read 0x4000000 0xb000000 0x2200000 && mw f8005000 ABC000 && bootm 0x2080000 0x4000000 0x2000000'
envbootargs='console=ttyPS0,115200 root=/dev/ram0 rw'
allowed_fw='Fri Nov 17 17:57:49 CST 2017'
ant_system_filesystem_version=`sed -n 1p /usr/bin/compile_time`
memory_size=`awk '/MemTotal/{total=$2}END{print total}' /proc/meminfo`

if [ $ret -eq 1 ];then

    if [ ! -e /dev/ubi_ctrl ];then
            echo "File system isn't UBI, Please update ubi package first!"
            cd $path
            rm -rf *.tar.gz
            exit 1
    fi

    if [ "$ant_system_filesystem_version" != "$allowed_fw" ]; then
        echo "Unknown firmware version $ant_system_filesystem_version detected, flash Antminer-S9-all-201711171757-autofreq-user-Update2UBI-NF.tar.gz before migrating."
        cd $path
        rm -rf *.tar.gz
        exit 1
    fi

    if [ "$memory_size" != "1015424" ] && [ "$memory_size" != "495632" ]; then
        echo "Unknown ram size $memory_size detected, aborting."
        cd $path
        rm -rf *.tar.gz
        exit 1
    fi

    if [ -e BOOT.bin ]; then
            if [ -e fw_setenv ]; then
                flash_erase /dev/mtd0 0x0 0 >/dev/null 2>&1
                chmod +x fw_setenv
                ./fw_setenv -c ./fw_env.config kernel_addr_r 0x2000000 2>&1
                ./fw_setenv -c ./fw_env.config kernel_addr_r 0x2000000 2>&1
                ./fw_setenv -c ./fw_env.config fdt_high 0xffffffff 2>&1
                ./fw_setenv -c ./fw_env.config fdt_high 0xffffffff 2>&1
                ./fw_setenv -c ./fw_env.config initrd_high 0xffffffff 2>&1
                ./fw_setenv -c ./fw_env.config initrd_high 0xffffffff 2>&1
                ./fw_setenv -c ./fw_env.config ethaddr $mac 2>&1
                ./fw_setenv -c ./fw_env.config ethaddr $mac 2>&1
                ./fw_setenv -c ./fw_env.config bootargs $envbootargs 2>&1
                ./fw_setenv -c ./fw_env.config bootargs $envbootargs 2>&1
                ./fw_setenv -c ./fw_env.config bootcmd "$envbootcmd" 2>&1
                ./fw_setenv -c ./fw_env.config bootcmd "$envbootcmd" 2>&1
            fi
            nandwrite -p -s 0x0 /dev/mtd0 ./BOOT.bin >/dev/null 2>&1
            rm -rf BOOT.bin
    fi

    if [ -e devicetree1g.dtb ] && [ "$memory_size" == "1015424" ]; then
            nandwrite -p -s 0x480000 /dev/mtd0 ./devicetree1g.dtb >/dev/null 2>&1
            nandwrite -p -s 0x500000 /dev/mtd0 ./devicetree1g.dtb >/dev/null 2>&1
            ./fw_setenv -c ./fw_env.config hwrevision "bm_xil_1g:s9.bm_xil_1g" 2>&1
            ./fw_setenv -c ./fw_env.config hwrevision "bm_xil_1g:s9.bm_xil_1g" 2>&1
            rm devicetree1g.dtb
    elif [ -e devicetree512m.dtb ] && [ "$memory_size" == "495632" ]; then
            nandwrite -p -s 0x480000 /dev/mtd0 ./devicetree512m.dtb >/dev/null 2>&1
            nandwrite -p -s 0x500000 /dev/mtd0 ./devicetree512m.dtb >/dev/null 2>&1
            ./fw_setenv -c ./fw_env.config hwrevision "bm_xil_512m:s9.bm_xil_512m" 2>&1
            ./fw_setenv -c ./fw_env.config hwrevision "bm_xil_512m:s9.bm_xil_512m" 2>&1
            rm devicetree512m.dtb
    else
        echo "Error writing device tree, aborting."
        exit 1
    fi

    if [ -e system.bit ]; then
            nandwrite -p -s 0x580000 /dev/mtd0 ./system.bit >/dev/null 2>&1
            nandwrite -p -s 0x980000 /dev/mtd0 ./system.bit >/dev/null 2>&1
            rm system.bit
    fi

    if [ -e uImage ]; then
            nandwrite -p -s 0xD80000 /dev/mtd0 ./uImage >/dev/null 2>&1
            nandwrite -p -s 0x1280000 /dev/mtd0 ./uImage >/dev/null 2>&1
            rm uImage
    fi

    if [ -e rootfs.cpio.uboot ]; then
            flash_erase /dev/mtd2 0x0 0 >/dev/null 2>&1
            nandwrite -p -s 0x0 /dev/mtd2 ./rootfs.cpio.uboot >/dev/null 2>&1
            rm rootfs.cpio.uboot
    fi

    sync

else
    echo "this is not compatible with the c5 controller"
fi

rm -rf *.tar.gz


#/sbin/reboot -f &
