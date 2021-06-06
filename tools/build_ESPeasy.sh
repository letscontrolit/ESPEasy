#!/bin/bash

# Build script to make a build in Linux.
# Tested on Ubuntu 18.04 from within Windows 10

# Needed packages:
# sudo apt-get update
# sudo apt-get upgrade
# sudo apt install python3-minimal virtualenv build-essential zip binutils
# For Python 3.8:
# sudo apt install -y software-properties-common
# sudo add-apt-repository ppa:deadsnakes/ppa
# sudo apt install -y python3.8



# Full directory name of the script no matter where it is being called from.
# It will work as long as the last component of the path used to find the script is not a symlink
# See: https://stackoverflow.com/a/246128
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"


REPO=https://github.com/letscontrolit/ESPEasy.git
BRANCH=mega


if [ ! -f "${SCRIPT_DIR}/../platformio.ini" ]; then
  echo "File is not present in known folder."
  echo "Please checkout full ESPEasy repository using:"
  echo "  git clone ${REPO}"
  exit 1
fi

cd ${SCRIPT_DIR}
cd ..
SRC=`pwd`

PULL_REQ=0
DESCRIPTION=""
GITTAG=""
HELP=0
BUILD_ALL=1
BUILD_DOCS=0
BUILD_ESP32=0
BUILD_ESP82XX=0

# $@ is all command line parameters passed to the script.
# -o is for short options like -v
# -l is for long options with double dash like --version
# the comma separates different long options
# -a is for long options with single dash like -version
options=$(getopt -l "help,pull-request:,description:,tag:,src-dir:,docs,esp32,esp82xx" -o ":hp:d:t:s:" -a -- "$@")

# set --:
# If no arguments follow this option, then the positional parameters are unset. Otherwise, the positional parameters 
# are set to the arguments, even if some of them begin with a ‘-’.
eval set -- "$options"

while true
do
case $1 in
-h|--help) HELP=1;;
-p|--pull-request)
  # get a specific pull request
  shift
  PULL_REQ=$1
  ;;
-d|--description) 
  shift
  DESCRIPTION=$1
  ;;
-t|--tag) 
  shift
  GITTAG=$1
  ;;
-s|--src-dir) 
  shift
  SRC=$1
  ;;
--docs)
  BUILD_ALL=0
  BUILD_DOCS=1
  ;;
--esp32)
  BUILD_ALL=0
  BUILD_ESP32=1
  ;;
--esp82xx)
  BUILD_ALL=0
  BUILD_ESP82XX=1
  ;;
\?) echo "Invalid option: $2" 1>&2
   HELP=1;;
--)
    shift
    break;;
esac
shift
done

VENV=`echo "${SRC}/venv/python3.8"`
TMP_DIST=`echo "${SRC}/ESPEasy_collect_dist"`


if (( $BUILD_ALL != 0 )); then
  BUILD_DOCS=1
  BUILD_ESP32=1
  BUILD_ESP82XX=1
fi

if (( $HELP != 0 )); then
   echo "ESPEasy Build Script"
   echo ""
   echo "Usage:"
   echo " build_ESPeasy.sh [-p <pull req. nr>] [-s <src dir>] [-d <description>] [--docs] [--esp32] [--esp82xx] [-h]"
   echo " build_ESPeasy.sh [-t <Git Tag>] [-s <src dir>] [-d <description>] [--docs] [--esp32] [--esp82xx] [-h]"
   echo ""
   echo " -h, -help,         --help          Display this help message"
   echo " -p, -pull-request, --pull-request  Build specific pull request NNNN (cannot be used with -t)"
   echo " -t, -tag,          --tag           Build based on Git tag (cannot be used with -p)"
   echo " -d, -description,  --description   Set description"
   echo " -s, -src-dir,      --src-dir       Display this help message"
   echo " --docs                             Build documentation"
   echo " --esp32                            Build all ESP32 PlatformIO project tasks"
   echo " --esp82xx                          Build all ESP8266/ESP8285 PlatformIO project tasks"
   echo "   If no build target (docs/esp32/esp82xx) is given, all will be built"
   echo ""
   echo "Examples:"
   echo "  build_ESPeasy.sh -p NNNN                            Build specific pull request NNNN (cannot be used with -t)"
   echo "  build_ESPeasy.sh -t mega-20210223                   Build based on Git tag (cannot be used with -p)"
   echo "  build_ESPeasy.sh -d MyGreatNewFeature               Set description"
   echo "  build_ESPeasy.sh -s ~/GitHub/letscontrolit/ESPEasy  Fetch source & build in set src dir"
   echo ""
   echo ""
   echo "Required (Ubuntu) packages:"
   echo "  sudo apt-get update"
   echo "  sudo apt-get upgrade"
   echo "  sudo apt install python3-minimal virtualenv build-essential zip binutils coreutils"
   echo "For Python 3.8:"
   echo "  sudo apt install -y software-properties-common"
   echo "  sudo add-apt-repository ppa:deadsnakes/ppa"
   echo "  sudo apt install -y python3.8"
   echo "For Sphinx (convert):"
   echo "  sudo apt install imagemagick"
   echo ""
   echo ""
   echo "Summary:"
   echo "  Pull Req     : ${PULL_REQ}"
   echo "  Git Tag      : ${GITTAG}"
   echo "  Description  : ${DESCRIPTION}"
   echo "  SRC dir      : ${SRC}"
   echo "  Python venv  : ${VENV}"
   echo "  TmpDir ZIP   : ${TMP_DIST}"
   echo "  Build Docs   : ${BUILD_DOCS}"
   echo "  Build ESP32  : ${BUILD_ESP32}"
   echo "  Build ESP82xx: ${BUILD_ESP82XX}"
   echo ""
   echo "      Use at your own risk!!"
   exit 0
else 
   echo "Summary:"
   echo "  Pull Req     : ${PULL_REQ}"
   echo "  Git Tag      : ${GITTAG}"
   echo "  Description  : ${DESCRIPTION}"
   echo "  SRC dir      : ${SRC}"
   echo "  Python venv  : ${VENV}"
   echo "  TmpDir ZIP   : ${TMP_DIST}"
   echo "  Build Docs   : ${BUILD_DOCS}"
   echo "  Build ESP32  : ${BUILD_ESP32}"
   echo "  Build ESP82xx: ${BUILD_ESP82XX}"
   echo ""
   echo "      Use at your own risk!!"
   echo ""
   echo "Continue in 5 seconds..."
   echo "Press CTRL-C to abort"
   sleep 5
fi


# if repository directory does not exist, make it and clone repository
if [ ! -d ${SRC} ]; then
  mkdir -p ${SRC}
  if [ $? -eq 0 ]; then
    echo "Created ${SRC}"
  else
    echo "Could not create source dir: ${SRC}"
    exit 1
  fi
  git clone --depth=50 --branch=${BRANCH} ${REPO} ${SRC}
  if [ $? -ne 0 ]; then
    echo "Could not fetch sources:"
    echo "  git clone --depth=50 --branch=${BRANCH} ${REPO} ${SRC}"
    exit 1
  fi
fi

# If virtualenv does not exist, make it.
if [ ! -d ${VENV} ]; then
  echo "Creating Python virtual env in: ${VENV}"
  mkdir -p ${VENV}
  virtualenv -p python3.8 ${VENV}
  if [ $? -ne 0 ]; then
    echo "Could not create Python virtual env:"
    echo "  virtualenv -p python3.8 ${VENV}"
    exit 1
  fi
fi

# Activate Python virtual environment and install/upgrade packages
source ${VENV}/bin/activate
if [ $? -ne 0 ]; then
  echo "Could not activate Python virtual env:"
  echo "  source ${VENV}/bin/activate"
  exit 1
fi

pip install -U platformio
# suppress lots of messages when packages are already installed with the correct version.
set -o pipefail; pip install -r ${SRC}/docs/requirements.txt | { grep -v "already satisfied" || :; }

# fetch latest version from active branch
cd ${SRC}
git fetch --tags
git submodule update --init --recursive
if [ -z $GITTAG ]; then
  if (( $PULL_REQ != 0 )); then
    git fetch origin +refs/pull/${PULL_REQ}/merge:
    if [ $? -eq 0 ]; then
      echo "Success!  git fetch origin +refs/pull/${PULL_REQ}/merge:"
    else
      echo "Could not fetch pull request:"
      echo "  git fetch origin +refs/pull/${PULL_REQ}/merge:"
      exit 1
    fi
    git checkout -qf FETCH_HEAD
    if [ -z "$DESCRIPTION" ]
    then
      GIT_DESCRIBE=`git describe|cut -d'-' -f-3`
      DESCRIPTION=`echo "${GIT_DESCRIBE}-PR_${PULL_REQ}"`

      echo "DESCRIPTION: ${DESCRIPTION}"
    fi
  else 
    # Just checkout the main branch and pull latest commits
    git checkout ${BRANCH}
    if [ $? -eq 0 ]; then
      echo "Success!  git checkout ${BRANCH}"
    else
      echo "Could not checkout branch ${BRANCH}"
      exit 1
    fi
  fi
else
  git checkout tags/$GITTAG -b $GITTAG
  if [ $? -eq 0 ]; then
    echo "Success!  git checkout tags/$GITTAG -b $GITTAG"
  else
    git checkout tags/$GITTAG
    if [ $? -eq 0 ]; then
      echo "Success!  git checkout tags/$GITTAG"
    else
      echo "Could not checkout Git tag:"
      echo "  git checkout tags/$GITTAG"
      exit 1
    fi
  fi
fi

if [ -z "$DESCRIPTION" ]; then
  DESCRIPTION=`git describe|cut -d'-' -f-3`

  echo "DESCRIPTION: ${DESCRIPTION}"
fi


if (( $BUILD_DOCS != 0 )); then
  # Build documentation
  cd ${SRC}/docs
  make html
fi

# Update (and clean) all targets
# N.B. clean does also install missing packages which must be installed before applying patches.
cd ${SRC}
platformio update
platformio run --target clean
# patch platformio core libs for PUYA bug (https://github.com/letscontrolit/ESPEasy/issues/650)
cd ${SRC}/patches; ./check_puya_patch;
cd ${SRC}

if [ -d "build_output/" ]; then
  rm -Rf build_output/*
fi


if (( $BUILD_ESP32 != 0 )); then
  # Must look into all possible env definitions.
  # Exclude so called "spec_" (special) builds
  # Include ESP32
  for ENV in `grep "^\[env:" platformio*.ini |cut -d'[' -f2|cut -d']' -f1|cut -d':' -f2|sort -n|grep -v spec_|grep -i esp32`;
  do 
    PLATFORMIO_BUILD_FLAGS="-D CONTINUOUS_INTEGRATION" platformio run -e ${ENV}
  done
fi

if (( $BUILD_ESP82XX != 0 )); then
  # Must look into all possible env definitions.
  # Exclude so called "spec_" (special) builds
  # Exclude ESP32
  for ENV in `grep "^\[env:" platformio*.ini |cut -d'[' -f2|cut -d']' -f1|cut -d':' -f2|sort -n|grep -v spec_|grep -iv esp32`;
  do 
    PLATFORMIO_BUILD_FLAGS="-D CONTINUOUS_INTEGRATION" platformio run -e ${ENV}
  done
fi

# Rename all built files, compute CRC and insert binaryFilename
# Collect all in a zip file.

if [ -d ${TMP_DIST} ]; then
  rm -Rf ${TMP_DIST}/*
fi

if [ ! -d ${TMP_DIST} ]; then
  mkdir -p ${TMP_DIST}
  if [ $? -eq 0 ]; then
      echo "Created temp dir ${TMP_DIST} to collect files for zip"
  else
      echo "Could not create tmp dir: ${TMP_DIST}"
      exit 1
  fi
fi

BUILD_LOG=`echo "${TMP_DIST}/buildlog.txt"`

# PIO 3.x :
# BINARY_PATH=".pioenvs"

# PIO 4.0 and newer:
BINARY_PATH=`echo "${SRC}/.pio/build"`



echo "### Creating zip archives"

#Naming convention:
# ESP_Easy_[github version]_[plugin set]_[chip type]_[flash memory].bin

if [ -d "build_output/reject" ]; then
  zip -qq ${SRC}/ESPEasy_ELF_files_$DESCRIPTION.zip build_output/reject/*
fi

zip -qq ${SRC}/ESPEasy_ELF_files_$DESCRIPTION.zip build_output/debug/*
echo "### Created ${SRC}/ESPEasy_ELF_files_$DESCRIPTION.zip"


mkdir -p ${TMP_DIST}
cp -r dist/* ${TMP_DIST}/

if (( $BUILD_DOCS != 0 )); then
  if [ -d "docs/build" ]; then
    # Docs have been created
    zip -r -qq ${SRC}/ESPEasy_docs_$DESCRIPTION.zip docs/build/*
    echo "### Created ${SRC}/ESPEasy_docs_$DESCRIPTION.zip"
  fi
fi

#create a source structure that is the same as the original ESPEasy project (and works with the howto on the wiki)
#rm -rf dist/Source 2>/dev/null

mkdir -p ${TMP_DIST}/source
cp -r lib ${TMP_DIST}/source/
cp -r src ${TMP_DIST}/source/
cp -r misc ${TMP_DIST}/source/
cp -r static ${TMP_DIST}/source/
cp -r tools ${TMP_DIST}/source/
cp platformio*.ini ${TMP_DIST}/source/
cp *.txt ${TMP_DIST}/source/
cp *.csv ${TMP_DIST}/source/
cp README* ${TMP_DIST}/source/

cp -r build_output/bin/* ${TMP_DIST}/bin
rm -f ${TMP_DIST}/bin/*ESP32*

cd ${TMP_DIST}

if (( $BUILD_ESP82XX != 0 )); then
  if [ "$(ls -A ${TMP_DIST}/bin/)"  ]; then
    echo
    zip -qq ${SRC}/ESPEasy_ESP82xx_$DESCRIPTION.zip -r .
    echo "### Created ${SRC}/ESPEasy_ESP82xx_$DESCRIPTION.zip"
  fi
fi

cd ${SRC}
# Remove non-ESP32 builds
rm -f ${TMP_DIST}/bin/*

# Remove flash tools not compatible with ESP32
rm -f ${TMP_DIST}/dist/ESP.Easy.Flasher.exe
rm -f ${TMP_DIST}/dist/esptool.exe
rm -f ${TMP_DIST}/dist/FlashESP8266.exe


if (( $BUILD_ESP32 != 0 )); then
  # Copy ESP32 builds
  cp -r build_output/bin/*ESP32* ${TMP_DIST}/bin

  cd ${TMP_DIST}

  if [ "$(ls -A ${TMP_DIST}/bin/)"  ]; then
    echo
    zip -qq ${SRC}/ESPEasy_ESP32_$DESCRIPTION.zip -r .
    echo "### Created ${SRC}/ESPEasy_ESP32_$DESCRIPTION.zip"
  fi
fi

rm -Rf ${TMP_DIST}/* 2>/dev/null
rmdir ${TMP_DIST}

echo "Done!"
