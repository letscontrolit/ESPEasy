#!/bin/bash

# Build script to make a build in Linux.
# Tested on Ubuntu 18.04 from within Windows 10

# Needed packages:
# sudo apt-get update
# sudo apt-get upgrade
# sudo apt install python-minimal virtualenv build-essential zip

VENV=~/.venv/python2.7
SRC=~/GitHub/letscontrolit/ESPEasy
REPO=https://github.com/letscontrolit/ESPEasy.git
BRANCH=mega

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
pip install -r ${SRC}/docs/requirements.txt

# fetch latest version from active branch
cd ${SRC}
git fetch --tags
git rebase
git submodule update --init --recursive
if (( $PULL_REQ != 0 )); then
  git fetch origin +refs/pull/${PULL_REQ}/merge:
  git checkout -qf FETCH_HEAD
  if [ -z "$DESCRIPTION" ]
  then
    GIT_DESCRIBE=`git describe|cut -d'-' -f-3`
    DESCRIPTION=`echo "${GIT_DESCRIBE}-PR_${PULL_REQ}"`
  fi
fi

# Build documentation
cd ${SRC}/docs
make html

# Update (and clean) all targets
# N.B. clean does also install missing packages which must be installed before applying patches.
cd ${SRC}
platformio update
platformio run --target clean
# patch platformio core libs for PUYA bug (https://github.com/letscontrolit/ESPEasy/issues/650)
cd ${SRC}/patches; ./check_puya_patch;
cd ${SRC}

# Build all targets in the platformio.ini file.
PLATFORMIO_BUILD_FLAGS="-D CONTINUOUS_INTEGRATION" platformio run

#for ENV in `grep "^\[env:" platformio.ini |cut -d':' -f2|cut -d']' -f1`;
#do
#  platformio run -e ${ENV}
#done

# Rename all built files, compute CRC and insert binaryFilename
# Collect all in a zip file.
if [ -z "$DESCRIPTION" ]
then
  ${SRC}/before_deploy
else
  ${SRC}/before_deploy -d ${DESCRIPTION}
fi
