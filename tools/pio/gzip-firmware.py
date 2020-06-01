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
    if ".bin" in bin_file:
        # create string with location and file names based on variant
        gzip_file = "{}.gz".format(bin_file)

        # check if new target files exist and remove if necessary
        if os.path.isfile(gzip_file): os.remove(gzip_file)

        # write gzip firmware file
        with open(bin_file,"rb") as fp:
            with gzip.open(gzip_file, "wb") as f:
                shutil.copyfileobj(fp, f)


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", [bin_gzip])