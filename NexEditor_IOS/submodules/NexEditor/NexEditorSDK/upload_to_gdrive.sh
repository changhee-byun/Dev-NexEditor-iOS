#!/bin/bash
echo "#################################"
echo "you need to install gdrive"
echo "https://github.com/gdrive-org/gdrive"
echo "#################################"

function findFileID() {
    #"Uploaded 1v_y0_yr5PWsFcLWfQUoBR1uDt8c2OAto at 9.5 KB/s, total 14.0 KB"
    #RETSTRING=$1
    #RETSTRING=${RETSTRING#*Uploaded[[:space:]]} # remove "Uploaded "
    #FILEID=${RETSTRING:0:33} # 33 characters

    stringarray=( $1 )
    FILEID=""
    if [[ "${stringarray[0]}" == "Uploaded" ]] ; then
        if [ ${#stringarray[1]} -eq 33 ] ; then
            FILEID=${stringarray[1]}
        else 
            return 1
        fi
    else 
        return 1
    fi
    echo $FILEID
    return 0
}

function uploadToApiDemos() {
    PARENTID="1fQSmySPS08ZOgSSWgzMV8S50iSxXjc1x" #apidemo_release folder
    FILEPATH=$1

    #echo "apidemos upload start"
    # upload to google drive
    RESULT=`gdrive upload -p $PARENTID $FILEPATH | grep "Uploaded"`
    if [[ "$RESULT" == "" || $? -ne 0 ]]  
    then
        echo "== ERROR: upload  to gdrive"
        exit $?;
    fi

    # find file id
    FILEID=$(findFileID "$RESULT")
    if [ $? -ne 0 ]; then
        echo "== ERROR: can't find file id"
        exit $?;
    fi

    # get a download link
    RESULT=( `gdrive info $FILEID | grep "DownloadUrl:"` )
    #echo "upload end"
    echo "${RESULT[1]}"
}

function uploadToEditorSDK() {
    PARENTID="1QetMbZzLHKrfUCrwAnBOAUS1GuSuaVcf" # editor_release folder
    FILEPATH=$1

    #echo "editor sdk upload start"
    # upload to google drive
    RESULT=`gdrive upload -p $PARENTID $FILEPATH | grep "Uploaded"`
    if [[ "$RESULT" == "" || $? -ne 0 ]]  
    then
        echo "== ERROR: upload  to gdrive"
        exit $?;
    fi

    # find file id
    FILEID=$(findFileID "$RESULT")
    if [ $? -ne 0 ]; then
        echo "== ERROR: can't find file id"
        exit $?;
    fi

    # get a download link
    RESULT=( `gdrive info $FILEID | grep "DownloadUrl:"` )
    echo "${RESULT[1]}"
}

function uploadToEditorObjDump() {
    PARENTID="1NRfeMmD7CjGQ_zOUl--OxO6OmxKdWgkJ" # obj_dump folder
    FILEPATH=$1
    SDK_VER=$2

    #echo "editor sdk upload start"
    # upload to google drive
    RESULT=`gdrive upload -p $PARENTID $FILEPATH | grep "Uploaded"`
    if [[ "$RESULT" == "" || $? -ne 0 ]]  
    then
        echo "== ERROR: upload  to gdrive"
        exit $?;
    fi

    # find file id
    FILEID=$(findFileID "$RESULT")
    if [ $? -ne 0 ]; then
        echo "== ERROR: can't find file id"
        exit $?;
    fi

    # get a download link
    RESULT=( `gdrive info $FILEID | grep "DownloadUrl:"` )
    echo "${RESULT[1]}"
}

export -f uploadToApiDemos
export -f uploadToEditorSDK
export -f uploadToEditorObjDump

#uploadToEditorSDK "../Release/package/NexEditor_v2.12.31_for_kinemaster.zip"
