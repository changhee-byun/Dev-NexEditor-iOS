#!/bin/bash

set -e
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CWD=`pwd`
KEYNAME=NEX

function encrypt_packages {
# input: inroot, outzip_dir, clenzip_dir, keyname
keyname=$1
inroot=$2
cleanzips_dir=$3
outzips_dir=$4
packages=`ls ${inroot}`

CRYPT=${DIR}/crypt.sh

mkdir -p ${cleanzips_dir}
mkdir -p ${outzips_dir}

for i in ${packages}; 
do
    echo $i;
    cd ${inroot}/$i;
    zip -r ${cleanzips_dir}/$i.zip ./*;
    ${CRYPT} enc ${keyname} ${cleanzips_dir}/$i.zip ${outzips_dir}/$i.zip;
    # unzip encrypted packages
    rm -rf ${outzips_dir}/$i
    mkdir -p ${outzips_dir}/$i;
    cd ${outzips_dir}/$i;
    unzip ../$i.zip;
    rm ../$i.zip;
    # unzip clean packages
    rm -rf ${cleanzips_dir}/$i
    mkdir -p ${cleanzips_dir}/$i
    touch ${cleanzips_dir}/$i/e.json
    cd ${cleanzips_dir}/$i
    unzip ${cleanzips_dir}/$i.zip
    rm ${cleanzips_dir}/$i.zip
done

cd ${CWD}
}

INROOT=${DIR}/../../submodules/PackagedAssets/INTERNAL/Template/
CLEANZIPS_DIR=${DIR}/../clear/TemplateAssets.bundle/packages
OUTZIPS_DIR=${DIR}/../encrypted/TemplateAssets.bundle/packages

encrypt_packages ${KEYNAME} ${INROOT} ${CLEANZIPS_DIR} ${OUTZIPS_DIR}

INROOT=${DIR}/../../submodules/PackagedAssets/INTERNAL/TextEffects/
CLEANZIPS_DIR=${DIR}/../clear/OverlayTitleAssets.bundle/packages
OUTZIPS_DIR=${DIR}/../encrypted/OverlayTitleAssets.bundle/packages

encrypt_packages ${KEYNAME} ${INROOT} ${CLEANZIPS_DIR} ${OUTZIPS_DIR}
