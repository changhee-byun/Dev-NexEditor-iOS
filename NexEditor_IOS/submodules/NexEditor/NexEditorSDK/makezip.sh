#!/bin/bash
	
## declare variable

BASEDIR=$(dirname $0)
SAVED_PWD=`pwd`

TARGET_FILES_PATH=$1 #"../packages/tmp"
ZIP_NAME=$2 #"editorsdk"
TARGET_PROJECT_PATH=$3 #"../packages"

# check SDK
echo "== Checking Directory"
if ! [ -d "${TARGET_FILES_PATH}" ]; then
    echo
    echo "== ERROR: Package was not built. ${TARGET_FILES_PATH}"
    exit 1
fi

# Make zip file
echo "== Making zip file($ZIP_NAME)"
cd $TARGET_FILES_PATH

#zip -r $ZIP_NAME.zip ./ 2>&1 > $SAVED_PWD/make-zip-$ZIP_NAME.log
zip -r $ZIP_NAME.zip ./ 

cd $SAVED_PWD