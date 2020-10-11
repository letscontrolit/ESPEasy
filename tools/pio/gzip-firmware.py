# Inspired by: https://github.com/arendst/Tasmota/blob/development/pio/gzip-firmware.py
# Thanks Theo & Jason2866 :)

Import('env')
import os
import shutil
import gzip

OUTPUT_DIR = "build_output{}".format(os.path.sep)

def bin_gzip(source, target, env):
    bin_file = str(target[0])
    #print("\u001b[33m bin_file: \u001b[0m  {}".format(bin_file))
    # for now do not make .gz files for 1M builds as we don't have a proper 2-step updater supporting .bin.gz
    if "_1M" in bin_file:
        print("\u001b[33m skip gzip for: \u001b[0m  {}".format(bin_file))

    if "_1M" not in bin_file and ".bin" in bin_file:
        # create string with location and file names based on variant
        gzip_file = "{}.gz".format(bin_file)

        # check if new target files exist and remove if necessary
        if os.path.isfile(gzip_file): os.remove(gzip_file)

        # write gzip firmware file
        with open(bin_file,"rb") as fp:
            with gzip.open(gzip_file, "wb") as f:
                shutil.copyfileobj(fp, f)
                print("\u001b[33m gzip: \u001b[0m  {}".format(bin_file))


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", [bin_gzip])