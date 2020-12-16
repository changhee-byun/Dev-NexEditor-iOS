#!/bin/bash
	
## declare variable
DOWNLOADSERVER="http://download.nexstreaming.com/aprs/"
#DOWNLOADSERVER="https://httpbin.org/post"

##
CUSTOMER="editor_ci"
USERNAME="editorci"
USEREMAIL="martin.lee@kinemaster.com"

SUBJECT=$1
DESC=$2
FILEPATH1=$3
FILEPATH1=$4

SUBJECT="test"
DESC="uploadtest"
FILEPATH1="./nexeditor_release_note.md"

HEADERS="Content-Type: application/x-www-form-urlencoded"
PARAMS="-F \"mode=upload\" -F customer=$CUSTOMER -F agree=Y -F email=$USEREMAIL -F \"name=$USERNAME\" -F subject=$SUBJECT -F desc=$DESC "


echo "curl --trace-ascii /dev/stdout -w "%{http_code}" $PARAMS -F \"file[]=@$FILEPATH1; filename=note.md\"  -X POST $DOWNLOADSERVER"
response=$(curl --trace-ascii /dev/stdout -w "%{http_code}" $PARAMS -F "file[]=@$FILEPATH1; filename=note.md"  -X POST $DOWNLOADSERVER)

echo $response

