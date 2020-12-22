#!/bin/bash
# NOTE
#  Must be invoked from Xcode as 'Run Script' phase in 'Build Phase' section of project target settings
#  PROJECT_NAME, for example, won't be available otherwise

set -e
######################
# Options
######################

REVEAL_ARCHIVE_IN_FINDER=false
FRAMEWORK_NAME="${PROJECT_NAME}"
SIMULATOR_LIBRARY_PATH="${BUILD_DIR}/${CONFIGURATION}-iphonesimulator/${FRAMEWORK_NAME}.framework"
DEVICE_LIBRARY_PATH="${BUILD_DIR}/${CONFIGURATION}-iphoneos/${FRAMEWORK_NAME}.framework"
UNIVERSAL_LIBRARY_DIR="${BUILD_DIR}/${CONFIGURATION}-iphoneuniversal"
FRAMEWORK="${UNIVERSAL_LIBRARY_DIR}/${FRAMEWORK_NAME}.framework"

######################
# Build Frameworks
######################

echo "-- Building simulator target at ${PROJECT_NAME}.xcworkspace"
xcodebuild -workspace ${PROJECT_NAME}.xcworkspace -scheme ${PROJECT_NAME} -sdk iphonesimulator -configuration ${CONFIGURATION} build CONFIGURATION_BUILD_DIR=${BUILD_DIR}/${CONFIGURATION}-iphonesimulator 2>&1

echo "-- Building device target at ${PROJECT_NAME}.xcworkspace"
xcodebuild -workspace ${PROJECT_NAME}.xcworkspace -scheme ${PROJECT_NAME} -sdk iphoneos -configuration ${CONFIGURATION} build CONFIGURATION_BUILD_DIR=${BUILD_DIR}/${CONFIGURATION}-iphoneos 2>&1

######################
# Create directory for universal
######################
echo "-- Creating directory:${UNIVERSAL_LIBRARY_DIR}"
rm -rf "${UNIVERSAL_LIBRARY_DIR}"
mkdir "${UNIVERSAL_LIBRARY_DIR}"

echo "-- Creating directory:${FRAMEWORK}"
mkdir "${FRAMEWORK}"

######################
# Copy files Framework
######################

echo "-- Copying from:${DEVICE_LIBRARY_PATH}/."
echo "             to:${FRAMEWORK}"
cp -r "${DEVICE_LIBRARY_PATH}/." "${FRAMEWORK}"

######################
# Make an universal binary
######################

echo "-- Merging simulator:${SIMULATOR_LIBRARY_PATH}/${FRAMEWORK_NAME}"
echo "--               ios:${DEVICE_LIBRARY_PATH}/${FRAMEWORK_NAME}"
echo "--                to:${FRAMEWORK}/${FRAMEWORK_NAME}"
lipo "${SIMULATOR_LIBRARY_PATH}/${FRAMEWORK_NAME}" "${DEVICE_LIBRARY_PATH}/${FRAMEWORK_NAME}" -create -output "${FRAMEWORK}/${FRAMEWORK_NAME}" | echo

# For Swift framework, Swiftmodule needs to be copied in the universal framework
if [ -d "${SIMULATOR_LIBRARY_PATH}/Modules/${FRAMEWORK_NAME}.swiftmodule/" ];
then
    echo "-- Copying simulator swiftmodules"
    echo "-- from:${SIMULATOR_LIBRARY_PATH}/Modules/${FRAMEWORK_NAME}.swiftmodule"
    echo "--   to:${FRAMEWORK}/Modules/${FRAMEWORK_NAME}.swiftmodule/"
    cp -f ${SIMULATOR_LIBRARY_PATH}/Modules/${FRAMEWORK_NAME}.swiftmodule/* "${FRAMEWORK}/Modules/${FRAMEWORK_NAME}.swiftmodule/"
fi
                                                                      
if [ -d "${DEVICE_LIBRARY_PATH}/Modules/${FRAMEWORK_NAME}.swiftmodule/" ]; then
    echo "-- Copying device swiftmodules"
    echo "-- from:${DEVICE_LIBRARY_PATH}/Modules/${FRAMEWORK_NAME}.swiftmodule"
    echo "--   to:${FRAMEWORK}/Modules/${FRAMEWORK_NAME}.swiftmodule/"
    cp -f ${DEVICE_LIBRARY_PATH}/Modules/${FRAMEWORK_NAME}.swiftmodule/* "${FRAMEWORK}/Modules/${FRAMEWORK_NAME}.swiftmodule/"
fi
                                                                      
######################
# On Release, copy the result to release directory
######################
if [ "$OUTPUT_DIR" == "" ]; then
OUTPUT_DIR="${PROJECT_DIR}/Output/"
mkdir -p "$OUTPUT_DIR"
fi

echo "-- Copying framework to ${OUTPUT_DIR}"

cp -r "${FRAMEWORK}" "$OUTPUT_DIR"

if [ ${REVEAL_ARCHIVE_IN_FINDER} == true ]; then
    open "${OUTPUT_DIR}/"
fi

######################

DIST_OUTPUT_DIR="${OUTPUT_DIR}dist"
mkdir -p "$DIST_OUTPUT_DIR"

echo "-- Copying framework to ${DIST_OUTPUT_DIR}"

cp -r "${DEVICE_LIBRARY_PATH}/." "${DIST_OUTPUT_DIR}/${FRAMEWORK_NAME}.framework"
