#!/bin/bash
	
## declare variable

BASEDIR=$(dirname $0)
TARGET_PROJECT=""
TARGET_PROJECT_PATH="../DemoApp/ApiDemos"
APK_OUTPUT_PATH=$TARGET_PROJECT_PATH/app/build/outputs/apk/release/app-release

# In order to speed up the ndk-build, set "-jN" to BUILD_OPTION_FOR_NDK_MULTITHREAD
BUILD_OPTION_FOR_NDK_MULTITHREAD=""
#BUILD_OPTION_FOR_NDK_MULTITHREAD="-j8"

## declare function
function clean_demo_app() {
    cd $TARGET_PROJECT_PATH

    ./gradlew :app:clean
    ERR=$?
    cd $SAVED_PWD
    echo "Clean result: $ERR"
    if [ $ERR -ne 0 ]; then
        echo "== ERROR: clean app"
        exit $ERR;
    fi
}
function build_demo_app() {
    cd $TARGET_PROJECT_PATH

    ./gradlew :app:assembleRelease
    ERR=$?
    cd $SAVED_PWD
    echo "Build result: $ERR"
    if [ $ERR -ne 0 ]; then
        echo "== ERROR: assemble app"
        exit $ERR;
    fi

    cp $APK_OUTPUT_PATH.apk $1
}

SAVED_PWD=`pwd`
clean_demo_app
build_demo_app $1
