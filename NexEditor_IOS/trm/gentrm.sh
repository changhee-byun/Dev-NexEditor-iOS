#!/bin/bash

perl -pi -e "s/\r//g" *

E_BADARGS=100

CONFIG="doxy_config_for_nexeditor_ios"
PDFNAME="NexEditorSDK_for_iOS_Technical_Reference_Manual.pdf"

EDITION="ios"
VERSION="1.5.6"
if [ $# -gt 0 ]; then
  VERSION="$1"
fi

PROJECTROOT=$(pwd)

OUTPUTDIR=${PROJECTROOT}/output
LOGOPATH=${PROJECTROOT}/image/nexlogo.jpg
LATEXDIR=${OUTPUTDIR}/latex
HTMLDIR="${OUTPUTDIR}/html"
PDFPATH="${OUTPUTDIR}/pdf/$PDFNAME"

mkdir -p ${OUTPUTDIR}/pdf
mkdir -p ${OUTPUTDIR}/latex
mkdir -p ${OUTPUTDIR}/html

pushd ${PROJECTROOT}/config
  
( cat "$CONFIG" ; echo "PROJECT_NUMBER=$VERSION" ) | doxygen -
RETVAL=$?
[ $RETVAL -ne 0 ] && exit $RETVAL

echo
echo "DOXYGEN finished successfully"

echo "DIR is `pwd`"

echo "== Running pdflatex ..."
pushd $LATEXDIR
pdflatex refman.tex
RETVAL=$?
popd
[ $RETVAL -ne 0 ] && exit $RETVAL

echo "== Running makeindex ..."
pushd $LATEXDIR
makeindex refman
RETVAL=$?
popd
[ $RETVAL -ne 0 ] && exit $RETVAL

echo "== Running pdflatex ..."
pushd $LATEXDIR
pdflatex refman.tex
RETVAL=$?
popd
[ $RETVAL -ne 0 ] && exit $RETVAL

echo "== Copying refman.pdf to ${PDFPATH} ..."
pushd $LATEXDIR
cp refman.pdf "$PDFPATH"
popd


popd

pushd $LATEXDIR
cp "$LOGOPATH" "$HTMLDIR"
popd

echo
echo "SUCCESS!  Created PDF for version=$VERSION, edition=$EDITION"
echo "     PDF: $PDFPATH"
