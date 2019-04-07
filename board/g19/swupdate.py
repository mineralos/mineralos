# Copyright (C) 2015 Stefano Babic <sbabic@denx.de>
#
# Some parts from the patch class
#
# swupdate allows to generate a compound image for the
# in the "swupdate" format, used for updating the targets
# in field.
# See also http://sbabic.github.io/swupdate/
#
#
# To use, add swupdate to the inherit clause and set
# set the images (all of them must be found in deploy directory)
# that are part of the compound image.

import os
import argparse
import shutil

parser = argparse.ArgumentParser("Update bin generator")
parser.add_argument('--desc', dest='desc_file', required=True, type=str,
                    help='set the sw-description file path')
parser.add_argument('--boot', dest='boot_file', required=True, type=str,
                    help='set the BOOT.bin file path')
parser.add_argument('--devicetree', dest='devicetree_file', required=True, type=str,
                    help='set the devicetree.dtb file path')
parser.add_argument('--fpga', dest='fpga_file', required=True, type=str,
                    help='set the system.bit file path')
parser.add_argument('--kernel', dest='kernel_file', required=True, type=str,
                    help='set the uImage file path')
parser.add_argument('--rootfs', dest='rootfs_file', required=True, type=str,
                    help='set the rootfs.ubi file path')
parser.add_argument('--key', dest='key_file', required=True, type=str,
                    help='set the priv.pem signing key file path')
parser.add_argument('--workdir', dest='work_dir', required=True, type=str,
                    help='set the priv.pem signing key file path')
parser.add_argument('--out', dest='out_file', required=True, type=str,
                    help='set the update.swu output file path')
parser.add_argument('--version', dest='version', required=True, type=str,
                    help='set the update.swu version string')
parser.add_argument('--hwversion', dest='hwversion', required=True, type=str,
                    help='set the update.swu hardware-compatibility string')
parser.add_argument('--hwversionim', dest='hwversionim', required=True, type=str,
                    help='set the secondary update.swu hardware-compatibility string')
options = parser.parse_args()


def swupdate_is_hash_needed(s):
    with open(s, 'r') as f:
        for line in f:
            if line.find("@%s" % (filename)) != -1:
                return True
    return False

def swupdate_get_sha256(filepath):
    import hashlib

    m = hashlib.sha256()

    with open(filepath, 'rb') as f:
        while True:
            data = f.read(1024)
            if not data:
                break
            m.update(data)
    return m.hexdigest()

def swupdate_write_sha256(s, filename, hash):
    write_lines = []

    with open(s, 'r') as f:
        for line in f:
            write_lines.append(line.replace("@%s" % (filename), hash))

    with open(s, 'w+') as f:
        for line in write_lines:
            f.write(line)

def swupdate_write_version(s, filename, hash):
    write_lines = []

    with open(s, 'r') as f:
        for line in f:
            write_lines.append(line.replace("@%s" % (filename), hash))

    with open(s, 'w+') as f:
        for line in write_lines:
            f.write(line)

def swupdate_write_hwversion(s, filename, hwversion, hwversionim):
    write_lines = []

    if hwversionim == "":
        with open(s, 'r') as f:
            for line in f:
                write_lines.append(line.replace("@%s" % (filename), hwversion))
    else:
        with open(s, 'r') as f:
            for line in f:
                write_lines.append(line.replace("@%s" % (filename), hwversion + '", "' + hwversionim))

    with open(s, 'w+') as f:
        for line in write_lines:
            f.write(line)

def do_swuimage(sw, images, key, outfile, workdir, version, hwversion, hwversionim):

    list_for_cpio = ["sw-description"]
    list_for_cpio.append('sw-description.sig')
    shutil.copyfile(sw, os.path.join(workdir, "sw-description"))

    for image, filepath in images.items():
        if image != 'sw-description' and image != "sw-description.sig":
            hash = swupdate_get_sha256(filepath)
            swupdate_write_sha256(os.path.join(workdir, "sw-description"), image, hash)
            list_for_cpio.append(image)
    swupdate_write_version(os.path.join(workdir, "sw-description"), 'version', version)
    swupdate_write_hwversion(os.path.join(workdir, "sw-description"), 'hwversion', hwversion, hwversionim)

    privkey = key
    signcmd = "openssl dgst -sha256 -sign '%s' -out '%s' '%s'" % (
        privkey,
        os.path.join(workdir, "sw-description.sig"),
        os.path.join(workdir, "sw-description"))
    if os.system(signcmd) != 0:
        print("Failed to sign sw-description with %s" % (privkey))

    if os.path.exists(outfile):
        os.remove(outfile)
    line = 'for i in ' + ' '.join(list_for_cpio) + '; do echo $i;done | cpio -ov -H crc >' + outfile
    os.system("cd " + workdir + ";" + line)

if __name__ == '__main__':
    images = {
        "sw-description": options.desc_file,
        "sw-description.sig": os.path.join(options.work_dir, 'sw-description.sig'),
        "BOOT.bin": options.boot_file,
        "devicetree.dtb": options.devicetree_file,
        "system.bit": options.fpga_file,
        "uImage": options.kernel_file,
        "rootfs.ubi": options.rootfs_file
    }
    do_swuimage(options.desc_file, images, options.key_file, options.out_file, options.work_dir, options.version, options.hwversion, options.hwversionim)

    print('swupdate build done.')
