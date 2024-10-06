
# ESPEasy tools

This is a set of simple but powerful command line tools for scripting updates, and for backup and restore of configuration settings and ESPeasy rule files

## Security

Some warnings apply:

- Be careful not to use these tools in a multiuser environment. The ESPEasy device password may display with something like `ps f -ef` while commands are in progress.
- Protect your `$HOME/.espeasy` credential files as well as your home directory from access for other users. 
- Make sure you have physical access / can powercycle your ESPEasy devices. In rare cases a power cycle is needed for updates to work, or if a formware upload does not complete

## Installation and setup

### Prereqs

You will need:

- A `bash` shell environment (Linux, MacOS, WSL does not really matter)
- `curl`, `jq` and `gzip` must be available
- A working `mail` command is needed if you want backups sent somewhere else by email

### Copy scripts

The `espeasy*` scripts will normally be copied somewhere in your `$PATH`, for example:

```bash
cd tools
chmod a+rx espeasy*
cp -v espeasy* /usr/local/bin/ # you may need sudo command in front
```

### Credentials

You will normally set up a `$HOME/.espeasy` file as follows for the ESPEasy password. This is not needed if the `CURLARGS` variable is set up in your shell environment from somewhere else. In that case, create an empty file.

```bash
#
# espeasy logins
#
CURLFLAGS=""
CURLARGS=${CURLARGS:-"$CURLFLAGS -u admin:password"} # change password to yours
```

Then make sure this file is readable by your user only (`chmod 600 $HOME/.espeasy`)

This assumes that the same ESPEasy passwords apply on all devices, but you can also do something like `env CURLARGS="-u admin:otherpasswd" espeasy....` if some devices need another password.

## Backup / restore

### Single device

ESP device backup for a single device can be done to your current directory as follows:

```bash
$ espeasybackup 192.168.202.62
PROG: Backup list has 1 nodes: 192.168.202.62
espeasybackup: Backup 192.168.202.62 config.dat --> 20240827-205701-MitsubishiIR-12-config.dat ... OK
espeasybackup: Backup 192.168.202.62 security.dat --> 20240827-205701-MitsubishiIR-12-security.dat ... OK
espeasybackup: Backup 192.168.202.62 notification.dat --> 20240827-205701-MitsubishiIR-12-notification.dat ... OK
espeasybackup: Backup 192.168.202.62 rules1.txt --> 20240827-205701-MitsubishiIR-12-rules1.txt ... OK
espeasybackup: Backup 192.168.202.62 rules2.txt --> 20240827-205701-MitsubishiIR-12-rules2.txt ... OK
espeasybackup: Backup 192.168.202.62 rules3.txt --> 20240827-205701-MitsubishiIR-12-rules3.txt ... OK
espeasybackup: Backup 192.168.202.62 rules4.txt --> 20240827-205701-MitsubishiIR-12-rules4.txt ... OK
espeasybackup: Backup 192.168.202.62 esp.css --> 20240827-205701-MitsubishiIR-12-esp.css ... OK
```

... and restored as follows:

```bash
$ rm *.css # sometimes causes problems
$ espeasyconfig 192.168.202.62 20240827-205701-MitsubishiIR-12*
espeasyconfig: File name config.dat: '20240827-205701-MitsubishiIR-12-config.dat' -> '/tmp/espeasyconfig-1185//config.dat'
espeasyconfig: File name notification.dat: '20240827-205701-MitsubishiIR-12-notification.dat' -> '/tmp/espeasyconfig-1185//notification.dat'
espeasyconfig: File name rules1.txt: '20240827-205701-MitsubishiIR-12-rules1.txt' -> '/tmp/espeasyconfig-1185//rules1.txt'
espeasyconfig: File name rules2.txt: '20240827-205701-MitsubishiIR-12-rules2.txt' -> '/tmp/espeasyconfig-1185//rules2.txt'
espeasyconfig: File name rules3.txt: '20240827-205701-MitsubishiIR-12-rules3.txt' -> '/tmp/espeasyconfig-1185//rules3.txt'
espeasyconfig: File name rules4.txt: '20240827-205701-MitsubishiIR-12-rules4.txt' -> '/tmp/espeasyconfig-1185//rules4.txt'
espeasyconfig: File name security.dat: '20240827-205701-MitsubishiIR-12-security.dat' -> '/tmp/espeasyconfig-1185//security.dat'
Upload config.dat: ########### 100.0%
espeasyconfig: config.dat upload OK
########### 100.0%
espeasyconfig: notification.dat upload OK
########### 100.0%
espeasyconfig: rules1.txt upload OK
########### 100.0%
espeasyconfig: rules2.txt upload OK
########### 100.0%
espeasyconfig: rules3.txt upload OK
########### 100.0%
espeasyconfig: rules4.txt upload OK
########### 100.0%
espeasyconfig: security.dat upload OK
```

The restore destination must be same ESP device type

### Multiple devices

If you add the `-a` option, the peer device list will be used to find your ESP devices, and also add any well known ESP MAC addresses found in the local arp table (`arp -a`)

```bash
$ espeasybackup -a  192.168.202.62
espeasybackup: Initial list 192.168.202.62 192.168.202.63  ...
espeasybackup: Secondary list: 192.168.202.242 192.168.202.52 192.168.202.61 192.168.202.62 192.168.202.63 192.168.202.65 192.168.202.66 192.168.202.67 ...
espeasybackup: Backup list has 8 nodes: 192.168.202.242 192.168.202.52 192.168.202.61 192.168.202.62 192.168.202.63 192.168.202.65 192.168.202.66 192.168.202.67
espeasybackup: Backup 192.168.202.242 config.dat --> 20240827-210214-TestBallValve-2-config.dat ... OK
espeasybackup: Backup 192.168.202.242 security.dat --> 20240827-210214-TestBallValve-2-security.dat ... OK
espeasybackup: Backup 192.168.202.242 notification.dat --> 20240827-210214-TestBallValve-2-notification.dat ... OK
espeasybackup: Backup 192.168.202.242 rules1.txt --> 20240827-210214-TestBallValve-2-rules1.txt ... OK
espeasybackup: Backup 192.168.202.242 rules2.txt --> 20240827-210214-TestBallValve-2-rules2.txt ... OK
espeasybackup: Backup 192.168.202.242 rules3.txt --> 20240827-210214-TestBallValve-2-rules3.txt ... OK
espeasybackup: Backup 192.168.202.242 rules4.txt --> 20240827-210214-TestBallValve-2-rules4.txt ... OK
espeasybackup: Backup 192.168.202.62 config.dat --> 20240827-210214-MitsubishiIR-12-config.dat ... OK
espeasybackup: Backup 192.168.202.62 security.dat --> 20240827-210214-MitsubishiIR-12-security.dat ... OK
espeasybackup: Backup 192.168.202.62 notification.dat --> 20240827-210214-MitsubishiIR-12-notification.dat ... OK
espeasybackup: Backup 192.168.202.62 rules1.txt --> 20240827-210214-MitsubishiIR-12-rules1.txt ... OK
espeasybackup: Backup 192.168.202.62 rules2.txt --> 20240827-210214-MitsubishiIR-12-rules2.txt ... OK
espeasybackup: Backup 192.168.202.62 rules3.txt --> 20240827-210214-MitsubishiIR-12-rules3.txt ... OK
espeasybackup: Backup 192.168.202.62 rules4.txt --> 20240827-210214-MitsubishiIR-12-rules4.txt ... OK
espeasybackup: Backup 192.168.202.62 esp.css --> 20240827-210214-MitsubishiIR-12-esp.css ... OK
.
.
.
```

### cron jobs for backup

You may want to back up your ESP devices regularly to a remote place. This is a way to find all ESP devices from the peer list of some og them (`-a` option), compress into a `.tar.gz` file (`-Z` option), and email that to a mail account (when `MAILTO` defined in the environment):

```bash
20 06 * * 6  env MAILTO=admin@mydomain.net /usr/local/bin/espeasybackup -a -Z 192.168.202.52 192.168.202.62 192.168.202.64 > /tmp/espeasybackup.log 2>&1
```

## New firmware deployment

To upload new firmware, you specify a firmware file and as many IP addresses / hostnames that you want to update with that same hardware

```bash
$ espeasyupdate  -f ESP_Easy_mega_20240826_IR_ESP32c3_4M316k_LittleFS_CDC.bin 192.168.202.242
espeasyupdate: Updating 192.168.202.242 ...
espeasyupdate: 192.168.202.242 already has 'Custom IR for AC - 20240822-1'
espeasyupdate: Firmware on file (ESP_Easy_mega_IR_ESP32c3_4M316k_LittleFS_CDC) different from device (ESP_Easy_mega_public_IR_ESP32c3_4M316k_LittleFS_CDC). Use -f to force update.
<META http-equiv="refresh" content="15;URL=/">Update Success! Rebooting...
```

The `-f` option is needed if you want to switch from one firmware file to another, or repeat installing a git build already deployed.

These facts are tested from the ESPEasy device as follows:

```bash
# IP is hostname or IP address
curl -s "http://${IP}/json"| jq -r '.System."Git Build"'        # normally skipped if already deployed
curl -s "http://${IP}/json"| jq -r '.System."Binary Filename"'  # normally skipped if binary file is different
```
