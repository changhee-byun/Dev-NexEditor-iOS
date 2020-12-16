#!/bin/sh

function getNexEditorVersion {
    VERSION_PATH=$1

    #Init variable
    VERSION=""
    MAJOR=""
    MINOR=""
    PATCH=""
    BUILD=""

    #Parsing 1 : Find the line including version number.
    MAJOR=`grep "^#[A-Za-z 	]*NEXEDITOR_MAJOR_VERSION[^*]" ${VERSION_PATH}`
    MINOR=`grep "^#[A-Za-z 	]*NEXEDITOR_MINOR_VERSION[^*]" ${VERSION_PATH}`
    PATCH=`grep "^#[A-Za-z 	]*NEXEDITOR_PATCH_VERSION[^*]" ${VERSION_PATH}`


    #Parsing 2 : Filtering for leaving version number only.
    MAJOR=${MAJOR//[^0-9]/}
    MINOR=${MINOR//[^0-9]/}
    PATCH=${PATCH//[^0-9]/}
    #BUILD=${BUILD//[^0-9]/}

    #Combination all of versions.
    VERSION=${MAJOR}.${MINOR}.${PATCH}
    echo ${VERSION}
}

function getReleaseNoteVerion {
    RELEASE_NOTE=$1

    #Init variable
    VERSION=""

    #Parsing 1 : Find the line including latest version number.
    # "## v.2.12.31 based on v2.10.8"
    VERSION=`grep -o -E "## v.[0-9]+.[0-9]+.[0-9]+" $RELEASE_NOTE | head -1`

    #Parsing 2 : Filtering for leaving version number only.
    VERSION=${VERSION#*.} # remove "## v."
    VERSION=${VERSION//[^0-9.0-9.0-9]/}

    #Print sample app version.
    echo ${VERSION}
}

function getReleaseNote() {
    RELEASE_NOTE_FILE=$1
    #Init variable
    VERSION=""

    #Parsing 1 : Find the line including latest version number.
    # "## v.2.12.31 based on v2.10.8"
    local findVer=0
    NOTE=""
    while read line; do
        VERSION=`echo $line | grep -o -E "## v.[0-9]+.[0-9]+.[0-9]+"`
        if [ "$VERSION" != "" ]; then
            if [ $findVer -eq 0 ]; then
                findVer=1
            elif [ $findVer -eq 1 ]; then
                findVer=2
                break;
            fi
        fi

        if [ $findVer -eq 1 ]; then
            NOTE="$NOTE<br>$line"
        fi

    done < $RELEASE_NOTE_FILE

    echo "$NOTE"
}

function checkSDKVersionAndReleseNote() {
    #VERSION_PATH=$1
    #RELEASE_NOTE=$2

    local EDITORSDK_VERSION=$(getNexEditorVersion $1)
    local LAST_RELEASENOTE_VERSION=$(getReleaseNoteVerion $2)
    if [ $EDITORSDK_VERSION != $LAST_RELEASENOTE_VERSION ]; then
	    echo "Version not matched! - SDK version : $EDITORSDK_VERSION / Latest release note version : $LAST_RELEASENOTE_VERSION"
		exit 1
	fi
}

export -f getNexEditorVersion
export -f getReleaseNoteVerion
export -f checkSDKVersionAndReleseNote
