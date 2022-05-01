# Inspired by: https://github.com/arendst/Tasmota/blob/development/pio/name-firmware.py
# Thanks Theo & Jason2866 :)

Import('env')
import os
import shutil
import json

OUTPUT_DIR = "build_output{}".format(os.path.sep)

def get_max_bin_size(env_name, file_suffix):
    # FIXME TD-er: Must determine the max size for .bin.gz files

    max_bin_size = 1044464
    if "_1M" in env_name:
        # max 872 kiB - 16 bytes
        max_bin_size = 892912
    if "_1M_OTA" in env_name:
        # max 600 kiB - 16 bytes
        max_bin_size = 614384
    if "4M316k" in env_name or "_ESP32_4M2M" in env_name:
        # ESP32 with 1800k of sketch space.
        max_bin_size = 1900544
        if "factory" in file_suffix:
            # Factory bin files include a part which is not overwritten via OTA
            max_bin_size = max_bin_size + 65536
    if "_ESP32_" in env_name or "_ESP32s2_" in env_name:
        if "_16M8M" in env_name or "_16M2M" in env_name or "_16M1M" in env_name:
            # ESP32 with 4096k of sketch space.
            max_bin_size = 4194304
    if "debug_" in env_name:
        # Debug env, used for analysis, not to be run on a node.
        max_bin_size = 0

    return max_bin_size


    


def copy_to_build_output(sourcedir, variant, file_suffix):
    in_file = "{}{}".format(variant, file_suffix)
    full_in_file = os.path.join(sourcedir, in_file)

    if os.path.isfile(full_in_file):
        if ".bin" in file_suffix:
            file_size = os.path.getsize(full_in_file)
            if file_size > get_max_bin_size(variant, file_suffix):
                print("\u001b[31m file size:\u001b[0m {} > {}".format(file_size, get_max_bin_size(variant, file_suffix)))
                out_file = "{}reject{}{}".format(OUTPUT_DIR, os.path.sep, in_file)
                if not os.path.isdir("{}{}".format(OUTPUT_DIR, "reject")):
                    os.mkdir("{}{}".format(OUTPUT_DIR, "reject"))
            else:
                out_file = "{}bin{}{}".format(OUTPUT_DIR, os.path.sep, in_file)
        else:
            out_file = "{}debug{}{}".format(OUTPUT_DIR, os.path.sep, in_file)

        if os.path.isfile(out_file):
            os.remove(out_file)

        #print("\u001b[33m in file : \u001b[0m  {}".format(full_in_file))
        
        print("\u001b[33m copy to: \u001b[0m  {}".format(out_file))
        shutil.copy(full_in_file, out_file)

        generate_webflash_json_manifest(variant, file_suffix)


def generate_webflash_json_manifest(variant, file_suffix):
    chipFamily = 'NotSet'
    manifest_suff = ''
    add_improve = True

    if ".factory.bin" in file_suffix:
        if 'ESP32s2' in variant:
            chipFamily = 'ESP32-S2'
            manifest_suff = '.factory.manifest.json'
        else:
            if 'ESP32' in variant:
                chipFamily = 'ESP32'
                manifest_suff = '.factory.manifest.json'
    else:
        if ".bin" in file_suffix and ".gz" not in file_suffix and 'ESP32' not in variant:
            chipFamily = 'ESP8266'
            manifest_suff = '.manifest.json'
            add_improve = False
    if 'NotSet' not in chipFamily:
        json_path = "{}json{}".format(OUTPUT_DIR, os.path.sep) 

        bin_file = "{}{}".format(variant, file_suffix)
        manifest_file = "{}{}".format(variant, manifest_suff)
        out_file = "{}{}".format(json_path, manifest_file)

        manifest = {}
        manifest['name'] = bin_file
        manifest['new_install_prompt_erase'] = True
        parts = dict([('path', bin_file), ('offset', 0)])
        if add_improve:
            builds = dict([('chipFamily', chipFamily), ('improv', False), ('parts', [parts])])
        else:
            builds = dict([('chipFamily', chipFamily), ('parts', [parts])])
        manifest['builds'] = [builds]

        with open(out_file, "w") as file:
            print("\u001b[33m JSON to: \u001b[0m  {}".format(out_file))
            json.dump(manifest, file)


def bin_elf_copy(source, target, env):
    variant = env['PROGNAME']
    split_path = str(source[0]).rsplit(os.path.sep, 1)

    # Create a dump of the used build environment
    with open('{}{}{}.env.txt'.format(split_path[0], os.path.sep, variant), 'w') as outfile:
        outfile.write(env.Dump())
        outfile.close()
    
    # check if output directories exist and create if necessary
    if not os.path.isdir(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)

    for d in ['bin', 'debug', 'json']:
        if not os.path.isdir("{}{}".format(OUTPUT_DIR, d)):
            os.mkdir("{}{}".format(OUTPUT_DIR, d))

    for suff in [".elf", ".bin", ".bin.gz", ".factory.bin", ".env.txt"]:
        copy_to_build_output(split_path[0], variant, suff)

    import datetime
    print("\u001b[33m Timestamp:\u001b[0m", datetime.datetime.now())

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", [bin_elf_copy])
