VENV=~/.venv/python2.7
SRC=~/GitHub/letscontrolit/ESPEasy
REPO=https://github.com/letscontrolit/ESPEasy.git
BRANCH=mega

PIO_BUILDENV=custom_ESP8266_4M1M

PULL_REQ=0
DESCRIPTION=""

while getopts p:d: option
do
case "${option}"
in
p)
  # get a specific pull request
  PULL_REQ=${OPTARG}
  ;;
d) DESCRIPTION=${OPTARG};;
esac
done



# If virtualenv does not exist, make it.
if [ ! -d ${VENV} ]; then
  mkdir -p ${VENV}
  virtualenv -p python2.7 ${VENV}
fi

# if repository directory does not exist, make it and clone repository
if [ ! -d ${SRC} ]; then
  mkdir -p ${SRC}
  git clone --depth=50 --branch=${BRANCH} ${REPO} ${SRC}
fi

# Activate Python virtual environment and install/upgrade packages
source ${VENV}/bin/activate
pip install -U platformio
#pip install -r ${SRC}/docs/requirements.txt


# Update platformio
cd ${SRC}
platformio update

# patch platformio core libs for PUYA bug (https://github.com/letscontrolit/ESPEasy/issues/650)
cd ${SRC}/patches; ./check_puya_patch;
cd ${SRC}

# Build custom_ESP8266_4M target in the platformio.ini file
PLATFORMIO_BUILD_FLAGS="-D CONTINUOUS_INTEGRATION" platformio run -e ${PIO_BUILDENV}

# rename and check file
DATE=`date +%Y%m%d`
DESCRIPTION=`echo "${DATE}_vagrant"`
${SRC}/before_deploy -d ${DESCRIPTION}
mkdir -p /vagrant/build
mv *.zip /vagrant/build/
