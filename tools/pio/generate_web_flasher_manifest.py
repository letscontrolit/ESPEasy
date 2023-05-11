import argparse
import json
import sys
import os

manifest_binfiles = {}


def create_arg_parser():
    # Creates and returns the ArgumentParser object

    parser = argparse.ArgumentParser(description='Description of your app.')
    parser.add_argument('inputDirectory',
                    help='Path to the input directory.')
    parser.add_argument('--outputDirectory',
                    help='Path to the output that contains the resumes.')
    return parser


def create_display_text(description, families):
    fam_split = []
    esp32_split = []
    if 'ESP8266' in families:
        fam_split.append('ESP8266')
    if 'ESP32' in families:
        fam_split.append('ESP32')
    if 'ESP32-S2' in families:
        esp32_split.append('S2')
    if 'ESP32-S3' in families:
        esp32_split.append('S3')
    if 'ESP32-C2' in families:
        esp32_split.append('C2')
    if 'ESP32-C3' in families:
        esp32_split.append('C3')
    if 'ESP32-C6' in families:
        esp32_split.append('C6')
    if 'ESP32-H2' in families:
        esp32_split.append('H2')

    if len(esp32_split) > 0:
        fam_split.append('ESP32-' + '/'.join(esp32_split))

    fam_str = ','.join(fam_split)

    return '{} [{}]'.format(description, fam_str)


def parse_filename(file, version, variant, file_suffix):
    #print("{} : {} {} {}".format(file, version, variant, file_suffix))

    chipFamily = 'NotSet'
    manifest_suff = ''
    add_improv = True

    main_group = 'Misc'    # e.g. "4M Flash" or "Device Specific"
    group = 'NotSet'       # e.g. "Energy" or "CollectionA"
    description = 'NotSet'
    sub_group = 'NotSet'
    flash_size = 'NotSet'

    state = 'Missed'

    if ".factory.bin" in file_suffix:
        if 'ESP32' in variant:
            manifest_suff = '.manifest.json'
            if 'ESP32s2' in variant:
                chipFamily = 'ESP32-S2'
            elif 'ESP32s3' in variant:
                chipFamily = 'ESP32-S3'
            elif 'ESP32c2' in variant:
                chipFamily = 'ESP32-C2'
            elif 'ESP32c3' in variant:
                chipFamily = 'ESP32-C3'
            elif 'ESP32c6' in variant:
                chipFamily = 'ESP32-C6'
            elif 'ESP32h2' in variant:
                chipFamily = 'ESP32-H2'
            else:
                chipFamily = 'ESP32'

            if '_4M316k' in variant:
                flash_size = '4M'
                main_group = '4M Flash'
            elif '_16M1M' in variant:
                flash_size = '16M1M'
                main_group = '16M Flash'
            elif '_16M8M' in variant:
                flash_size = '16M8M'
                main_group = '16M Flash'
    else:
        if ".bin" in file_suffix and ".gz" not in file_suffix and 'ESP32' not in variant:
            chipFamily = 'ESP8266'
            manifest_suff = '.manifest.json'
            add_improv = False

            if '_4M1M' in variant:
                flash_size = '4M'
                main_group = '4M Flash'
            elif '_4M2M' in variant:
                flash_size = '4M2M'
                main_group = '4M Flash'
            elif '_2M256' in variant:
                flash_size = '2M256'
                main_group = '2M Flash'
            elif '_1M' in variant:
                flash_size = '1M'
                main_group = '1M Flash'


    if 'NotSet' not in chipFamily:
        for special in ['minimal_', 'core', '_302_', 'Domoticz', 'FHEM', 'hard_', 'beta', 'alt_wifi', 'OTA']:
            if special in variant:
                sub_group = variant
                description = variant

        if 'NotSet' in sub_group:
            if 'climate_' in variant:
                group = 'Climate'
            elif 'energy_' in variant:
                group = 'Energy'
            elif 'display_' in variant:
                group = 'Display'
            elif 'neopixel_' in variant:
                group = 'NeoPixel'
            elif 'normal_' in variant:
                group = 'Normal'
            elif 'max_' in variant:
                group = 'MAX'
            elif 'custom_' in variant:
                group = 'Custom'
            elif 'collection' in variant:
                variant_split = variant.split('_')
                group = 'Collection{}'.format(variant_split[1])

            if 'NotSet' not in group:
                sub_group_spit = group.split('_')
                sub_group_spit.append(flash_size)
                if '_PSRAM' in variant:
                    sub_group_spit.append('PSRAM')
                if 'LittleFS' in variant:
                    sub_group_spit.append('LittleFS')
                if '_IRExt' in variant:
                    sub_group_spit.append('IRExt')
                elif '_IR' in variant:
                    sub_group_spit.append('IR')
                if '_VCC' in variant:
                    sub_group_spit.append('VCC')
                if '_ETH' in variant:
                    sub_group_spit.append('ETH')

                sub_group = '_'.join(sub_group_spit)
                description = ' '.join(sub_group_spit)

            state = "Group"
        else:
            state = "No Group"
            main_group = 'Misc'

        if 'custom_' in variant:
            if 'Misc' in main_group:
                main_group = 'Custom Misc'
            else:
                main_group = 'Custom'
        if 'hard_' in variant:
            main_group = 'Device Specific'

    if ".factory.bin" in file_suffix or 'ESP32' not in file:
        #print('{:10s}: {:34s}\t{:10s} {} / {}'.format(state, sub_group, chipFamily, version, file))

        print('{:10s}: {:34s}\t{:10s} {} / {}'.format(state, description, chipFamily, version, file))

        if main_group not in manifest_binfiles:
            manifest_binfiles[main_group] = {}

        if sub_group not in manifest_binfiles[main_group]:
            manifest = {}
            manifest['name'] = description
            families = []
            families.append(chipFamily)
            manifest['displaytext'] = description
            manifest['families'] = families
            manifest['version'] = version
            manifest['new_install_prompt_erase'] = True
            parts = dict([('path', file), ('offset', 0)])
            if add_improv:
                builds = dict([('chipFamily', chipFamily), ('improv', False), ('parts', [parts])])
            else:
                builds = dict([('chipFamily', chipFamily), ('parts', [parts])])
            manifest['builds'] = [builds]
            manifest_binfiles[main_group][sub_group] = manifest
        else:
            parts = dict([('path', file), ('offset', 0)])
            if add_improv:
                builds = dict([('chipFamily', chipFamily), ('improv', False), ('parts', [parts])])
            else:
                builds = dict([('chipFamily', chipFamily), ('parts', [parts])])
            manifest_binfiles[main_group][sub_group]['builds'].append(builds)
            manifest_binfiles[main_group][sub_group]['families'].append(chipFamily)

        display_string = create_display_text(description, manifest_binfiles[main_group][sub_group]['families'])
        manifest_binfiles[main_group][sub_group]['displaytext'] = display_string



def list_folder(bin_folder):
    print(bin_folder)
    for root, dirs, files in os.walk(bin_folder):
        for file in files:
            if file.startswith('ESP_Easy') and file.endswith('.bin'):
                fullname = os.path.join(root, file)
                fname_split = file.split('.')
                variantfull = fname_split.pop(0)
                file_suffix = '.' + '.'.join(fname_split)

                vfull_split = variantfull.split('_')
                vfull_split.pop(0) # ESP
                vfull_split.pop(0) # Easy
                vfull_split.pop(0) # mega
                version = vfull_split.pop(0)
                variant = '_'.join(vfull_split)
                parse_filename(file, version, variant, file_suffix)


if __name__ == "__main__":
    arg_parser = create_arg_parser()
    parsed_args = arg_parser.parse_args(sys.argv[1:])
    if os.path.exists(parsed_args.inputDirectory):
        list_folder(parsed_args.inputDirectory)
        print(json.dumps(manifest_binfiles))

