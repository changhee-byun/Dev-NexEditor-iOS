@echo off

setlocal
set /p str=Please input customer name : 

mkdir %str%
if not "%ERRORLEVEL%" == "0" goto ERROR_FOLDER
echo create key folder

java -jar KeyczarTool.jar create --location=./%str% --purpose=crypt --name=assetcrypt
if not "%ERRORLEVEL%" == "0" goto ERROR_KEY_META
echo Generate key metadata file

java -jar KeyczarTool.jar addkey --location=./%str% --size=128 --status=primary
if not "%ERRORLEVEL%" == "0" goto ERROR_KEY_GEN
echo Generate primary key file

java -jar KeyczarTool.jar addkey --location=./%str% --size=128
if not "%ERRORLEVEL%" == "0" goto ERROR_KEY_GEN
echo Generate key file

java -jar KeyczarTool.jar addkey --location=./%str% --size=128
if not "%ERRORLEVEL%" == "0" goto ERROR_KEY_GEN
echo Generate key file

java -jar KeyczarTool.jar addkey --location=./%str% --size=128
if not "%ERRORLEVEL%" == "0" goto ERROR_KEY_GEN
echo Generate key file

java -jar KeyczarTool.jar addkey --location=./%str% --size=128
if not "%ERRORLEVEL%" == "0" goto ERROR_KEY_GEN
echo Generate key file

java -jar KeyczarTool.jar addkey --location=./%str% --size=128
if not "%ERRORLEVEL%" == "0" goto ERROR_KEY_GEN
echo Generate key file

java -jar nexassetpackagecryper.jar key %str%
if not "%ERRORLEVEL%" == "0" goto ERROR_KEY

echo Apply ./%str%/keys.txt to the source code

pause

goto QUIT

:ERROR_FOLDER
echo Can't create folder.
goto QUIT

:ERROR_KEY_META
echo Can't generate key metadata
goto QUIT

:ERROR_KEY_GEN
echo Can't generate keys
goto QUIT

:ERROR_KEY
echo Can't merge keys.txt for source code
goto QUIT

:QUIT