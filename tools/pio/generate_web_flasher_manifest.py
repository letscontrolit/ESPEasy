import argparse
import json
import sys
import os
import  collections

manifest_binfiles = {}


def create_arg_parser():
    # Creates and returns the ArgumentParser object

    parser = argparse.ArgumentParser(description='Generate index.html and manifest JSON files for web flasher.')
    parser.add_argument('inputDirectory',
                        help='Path to the input directory containing ESPEasy .bin files.')
    parser.add_argument('--outputDirectory',
                        help='Path to the output that contains the resumes. e.g. --outputDirectory=bin/')
    return parser


def create_display_text(description, version, families):
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
    if 'ESP32-C5' in families:
        esp32_split.append('C5')
    if 'ESP32-C61' in families:
        esp32_split.append('C61')
    elif 'ESP32-C6' in families:
        esp32_split.append('C6')
    if 'ESP32-H21' in families:
        esp32_split.append('H21')
    elif 'ESP32-H2' in families:
        esp32_split.append('H2')
    if 'ESP32-P4' in families:
        esp32_split.append('P4')

    if len(esp32_split) > 0:
        fam_split.append('ESP32-' + '/'.join(esp32_split))

    fam_str = ','.join(fam_split)

    return '{} {} [{}]'.format(version, description, fam_str)


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

    build_flags = ''

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
            elif 'ESP32c5' in variant:
                chipFamily = 'ESP32-C5'
            elif 'ESP32c61' in variant:
                chipFamily = 'ESP32-C61'
            elif 'ESP32c6' in variant:
                chipFamily = 'ESP32-C6'
            elif 'ESP32h21' in variant:
                chipFamily = 'ESP32-H21'
            elif 'ESP32h2' in variant:
                chipFamily = 'ESP32-H2'
            elif 'ESP32p4' in variant:
                chipFamily = 'ESP32-P4'
            else:
                chipFamily = 'ESP32'

            if '_4M316k' in variant:
                flash_size = '4M'
                main_group = '4M Flash'
            elif '_8M1M' in variant:
                flash_size = '8M1M'
                main_group = '8M Flash'
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
                # Select based on "4M1M" here to keep any occasional "4M2M" build
                # separated in another group
                if '_4M1M' in variant:
                    main_group = '4M Flash Collection Builds'


            if 'NotSet' not in group:
                sub_group_spit = group.split('_')

                # Add some flags to differentiate the groups
                # but also use this to sort the items per main group.
                specials = []
                specials.append(flash_size)
                if '_PSRAM' in variant:
                    specials.append('PSRAM')
                if 'LittleFS' in variant:
                    specials.append('LittleFS')
                if '_IRExt' in variant:
                    specials.append('IRExt')
                elif '_IR' in variant:
                    specials.append('IR')
                if '_VCC' in variant:
                    specials.append('VCC')
                if '_ETH' in variant:
                    specials.append('ETH')
                if 'solo1' in variant:
                    specials.append('Solo1')


                for sp in specials:
                    sub_group_spit.append(sp)

                sub_group = '_'.join(sub_group_spit)
                description = ' '.join(sub_group_spit)
                build_flags = '_'.join(specials)

            state = "Group"
        else:
            state = "No Group"
            main_group = 'Misc'

        if 'collection' in variant:
            if '4M Flash' in main_group:
                main_group = '4M Flash Collection Builds'

        if 'custom_' in variant:
            if 'Misc' in main_group:
                main_group = 'Custom Misc'
            else:
                main_group = 'Custom'
        if 'hard_' in variant:
            main_group = 'Device Specific'

        if 'solo1' in variant:
            # Web flasher cannot detect whether it is an ESP32-classic or ESP32-solo1
            # Thus make a separate group for the solo1
            main_group = '4M Flash ESP32-solo1'

        if 'LittleFS' in variant:
            main_group += ' LittleFS'
        else:
            main_group += ' SPIFFS'

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
            manifest['build_flags'] = build_flags
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

        display_string = create_display_text(description, version, manifest_binfiles[main_group][sub_group]['families'])
        manifest_binfiles[main_group][sub_group]['displaytext'] = display_string
        manifest_binfiles[main_group][sub_group]['manifestfilename'] = '{}{}'.format(sub_group, manifest_suff)



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


def generate_manifest_files(bin_folder, output_prefix):
    # options for HTML. Will be sorted based on file order and flags
    html_options = {}

    # HTML option lines, generated from sorted html_options
    group_lines = []

    # the main grouping in the combo box on the web flasher page
    main_group_list = [
        '4M Flash',
        '4M Flash ESP32-solo1',
        '4M Flash Collection Builds',
        '8M Flash',
        '16M Flash',
        '2M Flash',
        '1M Flash',
        'Device Specific',
        'Custom',
        'Custom Misc',
        'Misc']

    main_group_list_littlefs = []
    main_group_list_spiffs = []
    for main_group in main_group_list:
        main_group_list_littlefs.append("{} {}".format(main_group, 'LittleFS'))
        main_group_list_spiffs.append("{} {}".format(main_group, 'SPIFFS'))

    main_group_list_littlefs.extend(main_group_list_spiffs)
    main_group_list = main_group_list_littlefs


    for main_group in main_group_list:
        if main_group in manifest_binfiles:
            html_options_group = []
            group_lines.append('        <optgroup label="{}">\n'.format(main_group))
            for sub_group in manifest_binfiles[main_group]:
                value = '{}{}'.format(
                    output_prefix,
                    manifest_binfiles[main_group][sub_group]['manifestfilename'])
                label = manifest_binfiles[main_group][sub_group]['displaytext']
                build_flags = manifest_binfiles[main_group][sub_group]['build_flags']
                manifest_file = os.path.join(
                    bin_folder,
                    manifest_binfiles[main_group][sub_group]['manifestfilename']
                )

                manifest = manifest_binfiles[main_group][sub_group]

                manifest.pop('build_flags')
                manifest.pop('families')
                manifest.pop('manifestfilename')

                html_sub_group = dict(
                    [
                        ('value', value),
                        ('label', label),
                        ('flags', build_flags),
                        ('manifest_file', manifest_file),
                        ('manifest', manifest)
                    ]
                )
                html_options_group.append(html_sub_group)

            html_options_group_sorted = sorted(html_options_group, key=lambda x: x['flags'])
            html_options[main_group] = html_options_group_sorted

            for option in html_options_group_sorted:
                group_lines.append('          <option value="{}" >{}</option>\n'.format(
                    option['value'],
                    option['label']))

                with open(option['manifest_file'], "w") as file:
                    json.dump(option['manifest'], file, indent=4)

            group_lines.append('        </optgroup>\n')

    #print(json.dumps(html_options, indent=2))

    html_out_file = os.path.join(bin_folder, 'index.html')

    with open(html_out_file, "w") as html_file:
        lines = [
            '<!DOCTYPE html>\n',
            '<html>\n',
            '  <head>\n',
            '    <style>\n',
            '      body {\n',
            '        font-family: sans-serif;\n',
            '      }\n',
            '      .pick-variant {\n',
            '        margin-bottom: 16px;\n',
            '      }\n',
            '    </style>\n',
            '    <script\n',
            '      type="module"\n',
            '      src="https://unpkg.com/tasmota-esp-web-tools@8.1.5/dist/web/install-button.js?module"\n',
            '    ></script>\n',
            '  </head>\n',
            '  <body>\n',
            '    <h1>Install ESPEasy </h1>\n',
            '\n',
            '    <div class="pick-variant">\n',
            '      <p>\n',
            '        To install ESPEasy, connect your ESP device to your computer, pick your\n',
            '        selected variant and click the install button.\n',
            '        <br>\n',
            '        <br>\n',
            '        See <a href="https://espeasy.readthedocs.io/en/latest/Plugin/_Plugin.html#list-of-official-plugins" >Documentation</a> for a list of which plugin is included in what build variant.\n',
            '      </p>\n',
            '      <select>\n'
        ]

        lines_tail = [
            '      </select>\n',
            '    </div>\n',
            '    <esp-web-install-button></esp-web-install-button>\n',

            '    <br>\n',
            '    <br>\n',
            '    See <a href="latest/" >latest/</a> for a pre-release test build.\n',
            '    <br>\n',
            '    See <a href="../" >../</a> for last official build.\n',
            '    <br>\n',
            '    <a href="all.zip" >all.zip</a> containing all bin files in a single zip file.\n',

            '    <br>\n',
            '    <br>\n',
            '    <h2>Migrate ESP32 installs from SPIFFS to LittleFS</h2>\n',
            '    From 2025/06/26 onward, there will be no longer SPIFFS builds for ESP32-xx.\n',
            '    <br>\n',
            '    See <a href="https://espeasy.readthedocs.io/en/latest/Reference/Migrate_SPIFFS_to_LittleFS.html" >Migrate from SPIFFS to LittleFS (ESP32)</a> in the documentation on how to migrate older (ESP32) SPIFFS installs to LittleFS\n',

            '    <script>\n',
            '      const selectEl = document.querySelector(".pick-variant select");\n',
            '      const installEl = document.querySelector("esp-web-install-button");\n',
            '      installEl.manifest = selectEl.value;\n',
            '      selectEl.addEventListener("change", () => {\n',
            '        installEl.manifest = selectEl.value;\n',
            '      });\n',
            '    </script>\n',
            '  </body>\n',
            '</html>'
        ]

        html_file.writelines(lines)
        html_file.writelines(group_lines)
        html_file.writelines(lines_tail)


if __name__ == "__main__":
    arg_parser = create_arg_parser()
    parsed_args = arg_parser.parse_args(sys.argv[1:])
    if os.path.exists(parsed_args.inputDirectory):
        list_folder(parsed_args.inputDirectory)
        generate_manifest_files(parsed_args.inputDirectory, parsed_args.outputDirectory)
        #print(json.dumps(manifest_binfiles))

