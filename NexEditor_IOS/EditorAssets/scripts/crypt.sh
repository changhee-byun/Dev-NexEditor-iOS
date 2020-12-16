#!/bin/sh
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT=${DIR}/../..
# NexEditor_IOS/submodules/NexEditor/DemoApp/nexassetpackagecryper/Tool
ENCRYPTOR_PATH=${PROJECT_ROOT}/submodules/NexEditor/DemoApp/nexassetpackagecryper/Tool
ENCRYPTOR_JAR=${ENCRYPTOR_PATH}/nexassetpackagecryper.jar

java -jar "${ENCRYPTOR_JAR}" $*
