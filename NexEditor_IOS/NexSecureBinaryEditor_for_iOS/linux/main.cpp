#include <iostream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include "NexSecureWare.h"
#include "NexSecureUpdater.h"

//#define DEBUG_MODE

using namespace std;

typedef unsigned char BYTE;

static BYTE g_InfoFlag[] = {0x54, 0x6b, 	0x56, 	0x59, 	0x56, 	0x46, 	0x4a, 	0x46, 	0x51,
        0x55, 	0x31, 	0x4a, 	0x54, 	0x6b, 	0x63, 	0x75, 	0x51, 	0x30, 	0x39, 	0x4e, 0x41,
		0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41};

#define SECURE_KEY_V2 "NEXTREAMING.COM_V2"
#define SECURE_KEY_V3 "NEXTREAMING.COM_V3"

// Global Variables
FILE 			*pDllFile;
NEXSECUREINFO	secureInfo;
BYTE*           pDLLData = NULL;
long            dllDataSize = 0;
long            dllDataInfoPos = 0;
long            dllDataInfoLen = 0;
long			chunkCount = 0;

// Function Protytypes
void handleError(NEXSECUREINFO *secureInfo, BYTE *pDLLData, FILE *pDllFile);
void readDynamicLibrary(char *argv[]);
void printProperties();
void updateProperties(int argc, char *argv[]);
unsigned int getTotalNumProperty();
void getPropertyUpdated(int argc, char *argv[], NEXSECUREITEM *property, int *size);
void printUsage();
int getMaxCount();
int getItemSize();

int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);

    NexSecure_Create(NEXSECURE_MAJOR_VERSION, NEXSECURE_MINOR_VERSION);

    // checks out whether arguments are valid
    if(argc == 1 || argc == 2) {
        printUsage();
        return -1;
    }

    // parses options
    if(strcmp("-i", argv[2]) == 0 || strcmp("--print-info", argv[2]) == 0) {
        readDynamicLibrary(argv);
        printProperties();
    } else if(strcmp("-u", argv[2]) == 0 || strcmp("--update", argv[2]) == 0) {
        readDynamicLibrary(argv);
        updateProperties(argc, argv);
    } else {
        fprintf(stderr, "Usage is wrong!\n\n");
        printUsage();
		return -1;
    }
    
    return 0;
    // return a.exec();
}


void printUsage() {
    cout << "Usage : modifydllinfo FILE [options] [property=value]" << endl;
    cout << "Options:" << endl;
    cout << "  " << "-u, --update			" << "Update properties of a dynamic library" << endl;
    cout << "  " << "-i, --print-info		" << "Print properties of a dynamic library" << endl;
}

void readDynamicLibrary(char *argv[]) {
	
    int nRead;

	char fileName[1024] = {0,};
	strcpy(fileName, argv[1]);
	strcat(fileName, "/NexPlayerSDK");
	fprintf(stderr, "will read %s\n", fileName);
    pDllFile = fopen(fileName, "rb");
    if(pDllFile == NULL) {
        fprintf(stderr, "Failed opening the DLL library\n");
        handleError(&secureInfo, pDLLData, pDllFile);
    }
    memset(&secureInfo, 0x00, sizeof(NEXSECUREINFO));
    strcpy(secureInfo.m_strKey, "NEXTREAMING.COM");
	
    fseek(pDllFile, 0, SEEK_END);
    dllDataSize = ftell(pDllFile);
    if(dllDataSize <= 0) {
        fprintf(stderr, "The DLL library is empty\n");
        handleError(&secureInfo, pDLLData, pDllFile);
    }
	
#if defined(DEBUG_MODE)
	fprintf(stderr, "Size is %ld\n", dllDataSize);
#endif
	
    pDLLData = new BYTE[dllDataSize];
    fseek(pDllFile, 0, SEEK_SET);
    nRead = fread(pDLLData, sizeof(BYTE), dllDataSize, pDllFile);
    if(nRead != dllDataSize) {
        fprintf(stderr, "Failed reading the DLL library\n");
        handleError(&secureInfo, pDLLData, pDllFile);
    }
	fclose(pDllFile);
}

// get secure information only. Create and destroy should be called from caller.
int parseProperties(NEXSECUREINFO* info)
{
	//TODO:Modify this parts.
	char *firstChunk = NexSecureUpdater_getFirstChunk(pDLLData, dllDataSize);
	if( !firstChunk ) {
		fprintf(stderr, "Could not find the first chunk!\n");
		return 1;
	}
	
	chunkCount = NexSecureUpdater_countAllChunks(pDLLData, dllDataSize);
	fprintf(stderr, "Found %ld chunks!\n", chunkCount);
	
	
	NexSecure_SetBitsData((unsigned char*)firstChunk, strlen(firstChunk)+1);
	NexSecure_GetInfo(info);
	return 0;
}

void printProperties() {
	NEXSECUREINFO info;
	memset(&info, 0, sizeof(NEXSECUREINFO));

	if( NexSecure_Create(NEXSECURE_MAJOR_VERSION, NEXSECURE_MINOR_VERSION) != NEXSECURE_ERROR_NONE ) {
		fprintf(stderr, "Could not create NexSecureWare!\n");
		return;
	}

	int ret = parseProperties(&info);
	if(ret != 0)
	{
		fprintf(stderr, "Parse Failed! %d\n", ret);
	}
	else
	{
		for( int i=0; i<(int)info.m_uiSecureItemCount; i++ ) {
			cout << info.m_SecureItem[i].m_strItemName;
			cout << "=" << info.m_SecureItem[i].m_strItemValue << endl;
		}
	}
	
	NexSecure_Destroy();
	return;
}

void updateProperties(int argc, char *argv[]) {
    int nOutLen;
    int size;
    NEXSECUREITEM *properties = NULL;
	
	if( NexSecure_Create(NEXSECURE_MAJOR_VERSION, NEXSECURE_MINOR_VERSION) != NEXSECURE_ERROR_NONE ) {
		fprintf(stderr, "Could not create NexSecureWare!\n");
		return;
	}
	
	int ret = parseProperties(&secureInfo);
	if(ret != 0)
	{
		fprintf(stderr, "Parse Failed! %d\n", ret);
	}
	

#if defined(DEBUG_MODE)
    for(int i = 0; i <  secureInfo.m_uiSecureItemCount; i++) {
        fprintf(stderr, "key : %s, value : %s\n", secureInfo.m_SecureItem[i].m_strItemName,
                secureInfo.m_SecureItem[i].m_strItemValue);
    }
#endif
	
    if(pDLLData == NULL) {
        fprintf(stderr, "Something is wrong with the DLL library\n");
        return;
    }

    if(secureInfo.m_uiSecureItemCount == 0)	{
        fprintf(stderr, "Something is wrong as to the number of items\n");
        memset(&secureInfo, 0x00, sizeof(NEXSECUREINFO));
        strcpy(secureInfo.m_strKey, "NEXTREAMING.COM");
    } else {
		int nexSecureItemSize = getItemSize();
        //properties = (NEXSECUREITEM*)malloc((argc - 3) * nexSecureItemSize);		
		properties = (NEXSECUREITEM*)malloc((argc - 3) * (nexSecureItemSize+12));
        getPropertyUpdated(argc, argv, properties, &size);
        for(int i = 0; i < size; i++) {
            for(int j = 0; j < secureInfo.m_uiSecureItemCount; j++) {
                if(strcmp((properties + i)->m_strItemName, secureInfo.m_SecureItem[j].m_strItemName) == 0) {
					secureInfo.m_SecureItem[j].m_strItemValue = new char[nexSecureItemSize];
                    strcpy(secureInfo.m_SecureItem[j].m_strItemValue, (properties + i)->m_strItemValue);
                    break;
                }
            }
        }
		
		//free is not mandatory...program is closing just after update finished
		//free(properties);
    }

#if defined(DEBUG_MODE)
    for(int i = 0; i <  secureInfo.m_uiSecureItemCount; i++) {
        fprintf(stderr, "key : %s, value : %s\n", secureInfo.m_SecureItem[i].m_strItemName,
                secureInfo.m_SecureItem[i].m_strItemValue);
    }
#endif

	NexSecure_SetInfo(&secureInfo);
	
	unsigned char resultBuf[25000] = {0};
	int resultLen = 0;
	
	NexSecure_GetBitsData(resultBuf, sizeof(resultBuf), &resultLen);
	
	
	int updatedChunks = NexSecureUpdater_updateAllChunks(pDLLData, dllDataSize, (char*)resultBuf);
	if( updatedChunks )
	{
		fprintf(stderr, "Updated %i chunks.\n", updatedChunks);

		char fileName[1024] = {0,};
		strcpy(fileName, argv[1]);
		strcat(fileName, "/NexPlayerSDK");
		fprintf(stderr, "will write %s\n", fileName);
		
		pDllFile = fopen(fileName, "wb");
		if(pDllFile == NULL ) {
			fprintf(stderr, "Error occurs when opening a file to write\n");
			handleError(&secureInfo, pDLLData, pDllFile);
		}
		
		fwrite(pDLLData, sizeof(BYTE), dllDataSize, pDllFile);

	} else {
		fprintf(stderr, "Unable to update any chunks.\n");
	}
	
#if defined(DEBUG_MODE)
    fprintf(stderr, "dllDataSize : %ld\n", dllDataSize);
#endif
	
	NexSecure_Destroy();
    fclose(pDllFile);
    return;
}

void getPropertyUpdated(int argc, char *argv[], NEXSECUREITEM *property, int *size) {
    int index = 0;
    const char *delimiter = "=";
    char *str;
	int itemSize = getItemSize();

    for(int i = 3; i < argc; i++) {
        str = strtok(argv[i], delimiter);		
        strcpy((property + index)->m_strItemName, str);
        str = strtok(NULL, delimiter);
		(property + index)->m_strItemValue = new char[itemSize];
        strcpy((property + index)->m_strItemValue, str);
        index++;
    }

    *size = index;
}

void handleError(NEXSECUREINFO *secureInfo, BYTE *pDLLData, FILE *pDllFile) {
    memset(secureInfo, 0x00, sizeof(NEXSECUREINFO));
    strcpy(secureInfo->m_strKey, "NEXTREAMING.COM");
    if(pDLLData) {
        delete [] pDLLData;
        pDLLData = NULL;
    }

    if(pDllFile) {
        fclose(pDllFile);
    }

    exit(EXIT_FAILURE);
}

int getMaxCount()
{
	cout << "Version: " << secureInfo.m_strKey << endl;
	if(strcmp(secureInfo.m_strKey, SECURE_KEY_V2) == 0)
	{
		return ITEM_MAX_COUNT_V2;
	}
	if(strcmp(secureInfo.m_strKey, SECURE_KEY_V3) == 0)
	{		
		return ITEM_MAX_COUNT_V3;
	}
	else
	{
		return ITEM_MAX_COUNT;
	}
}

int getItemSize()
{
	if(strcmp(secureInfo.m_strKey, SECURE_KEY_V2) == 0)
	{
		return ITEM_VALUE_MAX_LEN_V2;
	}
	if(strcmp(secureInfo.m_strKey, SECURE_KEY_V3) == 0)
	{		
		return ITEM_VALUE_MAX_LEN_V3;
	}
	else
	{
		return ITEM_VALUE_MAX_LEN;
	}
}



