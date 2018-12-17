#!/bin/bash

SRC=`pwd`
DST=${SRC}/converted

mkdir -p ${DST}

for file in `ls _P*.ino`; do
  PREF=`echo $file|cut -d '_' -f-2`
  USES=`echo "USES${PREF}"`
  if ! grep -q $USES "$file"; then
    echo "#ifdef ${USES}" > ${DST}/${file}
    cat ${file} >> ${DST}/${file}
    echo "#endif // ${USES}" >> ${DST}/${file}
  fi
done
