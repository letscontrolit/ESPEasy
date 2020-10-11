VENV=~/.venv/python3.8
SRC=~/GitHub/letscontrolit/ESPEasy
REPO=https://github.com/letscontrolit/ESPEasy.git
BRANCH=mega

PIO_BUILDENV=custom_ESP8266_4M1M

PULL_REQ=0
DESCRIPTION=""

VAGRANT_FOLDER="/vagrant"
BUILD_FOLDER=`echo "${VAGRANT_FOLDER}/build"`
CUSTOM_H_FILE="Custom.h"
ENV_LIST_FILE="pio_envlist.txt"

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
  virtualenv -p python3.8 ${VENV}
fi

# if repository directory does not exist, make it and clone repository
if [ ! -d ${SRC} ]; then
  mkdir -p ${SRC}
  git clone --depth=50 --branch=${BRANCH} ${REPO} ${SRC}
fi

# Activate Python virtual environment and install/upgrade packages
source ${VENV}/bin/activate
#pip install -r ${SRC}/docs/requirements.txt
pip install -r ${SRC}/requirements.txt
pip install -U platformio

# Update platformio
cd ${SRC}
platformio update

# patch platformio core libs for PUYA bug (https://github.com/letscontrolit/ESPEasy/issues/650)
cd ${SRC}/patches; ./check_puya_patch;
cd ${SRC}

VAGRANT_CUSTOM_H_FILE=`echo "${VAGRANT_FOLDER}/${CUSTOM_H_FILE}"`
VAGRANT_PIO_ENVLIST_FILE=`echo "${VAGRANT_FOLDER}/${ENV_LIST_FILE}"`
SRC_CUSTOM_H_FILE=`echo "${SRC}/src/${CUSTOM_H_FILE}"`
SRC_PIO_ENVLIST_FILE=`echo "${SRC}/src/${ENV_LIST_FILE}"`
if [ -f ${VAGRANT_CUSTOM_H_FILE} ]; then
  # replace Windows line endings with Unix line endings
  sed 's/\r$//g' ${VAGRANT_CUSTOM_H_FILE} > ${SRC_CUSTOM_H_FILE}
  echo "Copy ${VAGRANT_CUSTOM_H_FILE} > ${SRC_CUSTOM_H_FILE}"
fi
if [ -f ${VAGRANT_PIO_ENVLIST_FILE} ]; then
  # replace Windows line endings with Unix line endings
  sed 's/\r$//g' ${VAGRANT_PIO_ENVLIST_FILE} > ${SRC_PIO_ENVLIST_FILE}
  input=`echo "${SRC_PIO_ENVLIST_FILE}"`
  while IFS= read -r line
  do
    PLATFORMIO_BUILD_FLAGS="-D VAGRANT_BUILD" platformio run -e $line
  done < "$input"
else
  # Build custom_ESP8266_4M target in the platformio.ini file
  PLATFORMIO_BUILD_FLAGS="-D VAGRANT_BUILD" platformio run -e ${PIO_BUILDENV}
fi

# rename and check file
DATE=`date +%Y%m%d`
DESCRIPTION=`echo "${DATE}_vagrant"`
${SRC}/before_deploy -d ${DESCRIPTION}
mkdir -p ${BUILD_FOLDER}
mv *.zip ${BUILD_FOLDER}/
