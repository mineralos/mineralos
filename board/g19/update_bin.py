#!/bin/python3
# -*- coding: utf-8 -*-
import sys
import struct
import binascii
import argparse

parser = argparse.ArgumentParser("Update bin generator")
parser.add_argument('--cmd', dest='cmd_file', required=True, type=str,
                    help='set the upgrade.cmd file path')
parser.add_argument('--upgrade', dest='upgrade_file', required=True, type=str,
                    help='set the upgrade.py file path')
parser.add_argument('--boot', dest='boot_file', required=True, type=str,
                    help='set the BOOT.bin file path')
parser.add_argument('--devicetree', dest='devicetree_file', required=True, type=str,
                    help='set the devicetree.dtb file path')
parser.add_argument('--kernel', dest='kernel_file', required=True, type=str,
                    help='set the uImage file path')
parser.add_argument('--rootfs', dest='rootfs_file', required=True, type=str,
                    help='set the rootfs.jffs2 file path')
parser.add_argument('--out', dest='out_file', required=True, type=str,
                    help='set the update.bin output file path')
options = parser.parse_args()

gInnoUpgrScriptFile = 'upgrade.py'
gInnoUpgrFile       = 'update.bin'
gInnoUpgrEncryptKey = 99
gInnoUpgrPackFmt    = '<I'
gInnoUpgrDataFile   = 'upgrade.data'

# 文本加密
# text: 待加密的文本 (bytes/string)
def TextEncrypt(text):
    encryptText = bytearray(text)
    for i in range(0, len(encryptText)):
        encryptText[i] = (encryptText[i] + gInnoUpgrEncryptKey) & 0xff
    return encryptText

# 打包update.bin：MakeScriptSegment/MakeDataSegment/MakeUpgrFile
def MakeScriptSegment(scriptPath):
    # 生成script段，由以下部分组成
    # 1.segHeadLen: 4 Bytes (为1~5项长度之和)
    # 2.segNameLen: 4 Bytes
    # 3.segName:   segNameLen Bytes
    # 4.reserved:   16 Bytes
    # 5.segDataLen: 4 Bytes
    # 6.segData:    segDataLen Bytes
    # 读取script文件
    fd = open(scriptPath, 'rb')
    filedata = fd.read()
    fd.close()
    # 文本加密
    encryptData = TextEncrypt(filedata)
    # 初始化头部信息
    segName = gInnoUpgrScriptFile.encode(encoding='UTF-8', errors='strict') # utf-8编码
    segName = TextEncrypt(segName)                                          # 文本加密
    segNameLen = len(segName)
    segHeadLen = segNameLen + 28
    segResv0 = 0
    segResv1 = 0
    segResv2 = 0
    segResv3 = 0
    #encryptData = encryptData.encode(encoding='UTF-8', errors='strict')
    segDataLen = len(encryptData)
    # 打包
    segment = b''
    segment += struct.pack(gInnoUpgrPackFmt, segHeadLen)
    segment += struct.pack(gInnoUpgrPackFmt, segNameLen)
    segment += segName
    segment += struct.pack(gInnoUpgrPackFmt, segResv0)
    segment += struct.pack(gInnoUpgrPackFmt, segResv1)
    segment += struct.pack(gInnoUpgrPackFmt, segResv2)
    segment += struct.pack(gInnoUpgrPackFmt, segResv3)
    segment += struct.pack(gInnoUpgrPackFmt, segDataLen)
    segment += encryptData

    return segment

def MakeDataSegment(pkgTab):
    '''
    data段由segHead、pkgTab、segData三部分组成
    segHead由以下字段构成:
    1.segHeadLen: 4 Bytes (为1~5项长度之和)
    2.segNameLen: 4 Bytes
    3.segName：   segNameLen Bytes
    4.segPkgNum:  4 Bytes
    5.reserved:   12 Bytes
    6.segDataLen: 4 Bytes
    pkgTab包含若干个条目，每个条目对应一个package，定义如下：
    1.pkgOffset: 4 Bytes
    2.pkgLen:    4 Bytes
    3.pkgName：  24 Bytes
    紧随其后的segData中按照pkgTab中定义的偏移量和长度存放各package的数据
    '''

    # 生成data段package table和数据部分
    segPkgTab = b''
    segData = b''
    segPkgTabLen = 32 * len(pkgTab)
    offset = segPkgTabLen       # offset为pkgData相对于pkgTab起始位置的偏移量
    for pkg in pkgTab:
        # 读取文件内容
        fd = open(pkg[1], 'rb')
        filedata = fd.read()
        fd.close()
        # 文本加密
        pkgName = pkg[0].encode(encoding='UTF-8', errors='strict')
        isEncrypt = pkg[2]
        if isEncrypt:
            pkgName = TextEncrypt(pkgName)
            filedata = TextEncrypt(filedata)
        # 生成package table
        filelen = len(filedata)
        segPkgTab += struct.pack(gInnoUpgrPackFmt, offset)      # offset
        segPkgTab += struct.pack(gInnoUpgrPackFmt, filelen)     # length
        segPkgTab += struct.pack('24s', pkgName)                # name
        # 打包
        segData += filedata
        offset += filelen

    # 初始化头部信息
    segName = gInnoUpgrDataFile.encode(encoding='UTF-8', errors='strict')
    segNameLen = len(segName)
    segHeadLen = segNameLen + 28
    segPkgNum = len(pkgTab)
    segResv0 = 0
    segResv1 = 0
    segResv2 = 0
    segDataLen = segPkgTabLen + len(segData)
    # 打包
    segHead = b''
    segHead += struct.pack(gInnoUpgrPackFmt, segHeadLen)
    segHead += struct.pack(gInnoUpgrPackFmt, segNameLen)
    segHead += segName
    segHead += struct.pack(gInnoUpgrPackFmt, segPkgNum)
    segHead += struct.pack(gInnoUpgrPackFmt, segResv0)
    segHead += struct.pack(gInnoUpgrPackFmt, segResv1)
    segHead += struct.pack(gInnoUpgrPackFmt, segResv2)
    segHead += struct.pack(gInnoUpgrPackFmt, segDataLen)

    return segHead + segPkgTab + segData


def MakeUpgrFile(scriptPath, pkgTab, lock):
    # 设置矿场锁，防止升级public release
    filepath = None
    isLock = 0
    isLock = 0
    filepath = options.out_file

    # step1: 构造升级包数据部分
    upgrData = MakeScriptSegment(scriptPath) + MakeDataSegment(pkgTab)

    # step2: 构造包头，由以下部分组成
    # 1.headLen:  4 Bytes (为1~6项长度之和)
    # 2.crc32:    4 Bytes (数据部分crc校验值)
    # 3.nameLen:  4 Bytes
    # 4.name：    segNameLen Bytes
    # 5.isLock:   4 Bytes
    # 6.reserved: 12 Bytes
    # 7.dataLen:  4 Bytes
    crc32 = binascii.crc32(upgrData)
    name = gInnoUpgrFile.encode(encoding='UTF-8', errors='strict')
    nameLen = len(name)
    headLen = nameLen + 32
    dataLen = len(upgrData)
    resv0 = 0
    resv1 = 0
    resv2 = 0
    upgrHead = b''
    upgrHead += struct.pack(gInnoUpgrPackFmt, headLen)
    upgrHead += struct.pack(gInnoUpgrPackFmt, crc32)
    upgrHead += struct.pack(gInnoUpgrPackFmt, nameLen)
    upgrHead += name
    upgrHead += struct.pack(gInnoUpgrPackFmt, isLock)
    upgrHead += struct.pack(gInnoUpgrPackFmt, resv0)
    upgrHead += struct.pack(gInnoUpgrPackFmt, resv1)
    upgrHead += struct.pack(gInnoUpgrPackFmt, resv2)
    upgrHead += struct.pack(gInnoUpgrPackFmt, dataLen)

    # step3: 写入升级文件
    fd = open(filepath, 'wb')
    fd.write(upgrHead + upgrData)
    fd.close()


# 升级数据包组成: 
#         名称                  输入路径             是否加密
gInnoUpgrPkgTable = (
        ['upgrade.cmd',        options.cmd_file,        True],
        ['rootfs.jffs2',       options.rootfs_file,     True],
        ['BOOT.bin.G19',       options.boot_file,       True],
        ['devicetree.dtb.G19', options.devicetree_file, True],
        ['uImage.G19',         options.kernel_file,     True])

if __name__ == '__main__':
    # 根据lock参数决定是否生成升级锁定版本
    isLock = 0
    #print(gInnoUpgrPkgTable)

    MakeUpgrFile(options.upgrade_file, gInnoUpgrPkgTable, isLock)

    print('build done.')

