#!/bin/sh
set -e
TARGET_PACKAGE_DIR=output/package
SAMPLE_ROOT_DIR=${TARGET_PACKAGE_DIR}/sample
APIDEMOS_DIR=${SAMPLE_ROOT_DIR}/APIDemos
BASIC_DIR=${SAMPLE_ROOT_DIR}/Basic
CWD=$PWD

rm -rf ${TARGET_PACKAGE_DIR}
mkdir -p ${SAMPLE_ROOT_DIR}
cp APICHANGES.md ${TARGET_PACKAGE_DIR}
cp APIMIGRATION*.md ${TARGET_PACKAGE_DIR}
cp CHANGES.md ${TARGET_PACKAGE_DIR}
cp -a Frameworks ${TARGET_PACKAGE_DIR}/framework
cp -a Samples/APIDemos ${SAMPLE_ROOT_DIR}
cp -a Samples/Basic ${BASIC_DIR}
cp -a Samples/shared ${SAMPLE_ROOT_DIR}/shared

# sample/APIDemos
rm -rf ${APIDEMOS_DIR}/Makefile \
    ${APIDEMOS_DIR}/Podfile \
    ${APIDEMOS_DIR}/Podfile.lock \
    ${APIDEMOS_DIR}/Pods

mv ${APIDEMOS_DIR}/Podfile.dist \
    ${APIDEMOS_DIR}/Podfile
sed -e "s/\.\.\/\.\.\/Frameworks/\.\.\/\.\.\/framework/g" -i '' ${APIDEMOS_DIR}/Podfile

# sample/Basic
rm -rf ${BASIC_DIR}/Makefile \
    ${BASIC_DIR}/Podfile \
    ${BASIC_DIR}/Podfile.dev \
    ${BASIC_DIR}/Podfile.lock \
    ${BASIC_DIR}/Pods

mv ${BASIC_DIR}/Podfile.dist \
    ${BASIC_DIR}/Podfile

sed -e "s/\.\.\/\.\.\/Frameworks/\.\.\/\.\.\/framework/g" -i '' ${BASIC_DIR}/Podfile

cd ${CWD}/${APIDEMOS_DIR}
pod install
xcodebuild -workspace APIDemos.xcworkspace -scheme APIDemos

cd ${CWD}/${BASIC_DIR}
pod install
xcodebuild -workspace Basic.xcworkspace -scheme Basic
