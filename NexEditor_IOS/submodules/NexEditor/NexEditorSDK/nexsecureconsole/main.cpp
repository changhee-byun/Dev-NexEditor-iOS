#include <iostream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include "NexSecureWare.h"
#include "NexBase64.h"

// #define DEBUG_MODE

using namespace std;

typedef unsigned char BYTE;

static BYTE g_InfoFlag[] = {0x54, 0x6b,     0x56,   0x59,   0x56,   0x46,   0x4a,   0x46,   0x51,
	0x55,   0x31,   0x4a,   0x54,   0x6b,   0x63,   0x75,   0x51,   0x30,   0x39,   0x4e, 0x41,
	0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41};

// Global Variables
FILE            *pDllFile;
NEXSECUREINFO   secureInfo;
BYTE*           pDLLData = NULL;
long            dllDataSize = 0;
long            dllDataInfoPos = 0;
long            dllDataInfoLen = 0;

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


char* trim(char *s);
char* rtrim(char* s);
char* ltrim(char *s);

int main(int argc, char *argv[])
{
//QCoreApplication a(argc, argv);

	NexSecure_Create(NEXSECURE_MAJOR_VERSION, NEXSECURE_MINOR_VERSION);

// checks out whether arguments are valid
	if (argc == 1 || argc == 2)
	{
		printUsage();
		return -1;
	}

// parses options
	if (strcmp("-i", argv[2]) == 0 || strcmp("--print-info", argv[2]) == 0)
	{
		readDynamicLibrary(argv);
		printProperties();
	}
	else if (strcmp("-u", argv[2]) == 0 || strcmp("--update", argv[2]) == 0)
	{
		readDynamicLibrary(argv);
		updateProperties(argc, argv);
	}
	else
	{
		fprintf(stderr, "Usage is wrong!\n\n");
		printUsage();
		return -1;
	}

	return 0;
// return a.exec();
}


void printUsage()
{
	cout << "Usage : modifydllinfo FILE [options] [property=value]" << endl;
	cout << "Options:" << endl;
	cout << "  " << "-u, --update			" << "Update properties of a dynamic library" << endl;
	cout << "  " << "-i, --print-info		" << "Print properties of a dynamic library" << endl;
}

void readDynamicLibrary(char *argv[])
{
	int nRead;

	pDllFile = fopen(argv[1], "rb");
	if (pDllFile == NULL)
	{
		fprintf(stderr, "Failed opening the DLL library\n");
		handleError(&secureInfo, pDLLData, pDllFile);
	}

	memset(&secureInfo, 0x00, sizeof(NEXSECUREINFO));
	strcpy(secureInfo.m_strKey, "NEXTREAMING.COM");

	fseek(pDllFile, 0, SEEK_END);
	dllDataSize = ftell(pDllFile);
	if (dllDataSize <= 0)
	{
		fprintf(stderr, "The DLL library is empty\n");
		handleError(&secureInfo, pDLLData, pDllFile);
	}

	pDLLData = new BYTE[dllDataSize];
	fseek(pDllFile, 0, SEEK_SET);
	nRead = fread(pDLLData, sizeof(BYTE), dllDataSize, pDllFile);
	if (nRead != dllDataSize)
	{
		fprintf(stderr, "Failed reading the DLL library\n");
		handleError(&secureInfo, pDLLData, pDllFile);
	}

	fclose(pDllFile);
}

unsigned int writeDataToFile(char *pPath, BYTE* pData, unsigned int uLen ) {
	unsigned int	uRet=0;
	
	FILE *fp = fopen(pPath, "wb");
	if (fp == NULL)
	{
		fprintf(stderr, "Error occurs when opening a file to write. [%s]", pPath);
		exit(0);
	}

	uRet = fwrite(pData, sizeof(BYTE), uLen, fp);
	fclose(fp);

	return uRet;
}

void printProperties()
{
	for (dllDataInfoPos = 0; dllDataInfoPos < dllDataSize - sizeof(g_InfoFlag); dllDataInfoPos++)
	{
		if (memcmp(pDLLData + dllDataInfoPos, g_InfoFlag, sizeof(BYTE)*20) == 0)
		{
			for (dllDataInfoLen = 0; dllDataInfoLen < dllDataSize - dllDataInfoPos - 1; dllDataInfoLen++)
			{
				if (*(pDLLData + dllDataInfoPos + dllDataInfoLen) == 0x3d &&
					*(pDLLData + dllDataInfoPos + dllDataInfoLen + 1) == 0)
				{
					dllDataInfoLen += 2;
					break;
				}
			}
			if (NexSecure_SetBitsData(pDLLData + dllDataInfoPos, dllDataInfoLen) != NEXSECURE_ERROR_NONE)
				handleError(&secureInfo, pDLLData, pDllFile);
			if (NexSecure_GetInfo(&secureInfo) != NEXSECURE_ERROR_NONE)
				handleError(&secureInfo, pDLLData, pDllFile);
			int maxCount = getMaxCount();
			if (secureInfo.m_uiSecureItemCount > maxCount)
				handleError(&secureInfo, pDLLData, pDllFile);

			for (int i = 0; i < secureInfo.m_uiSecureItemCount; i++)
			{
				cout << secureInfo.m_SecureItem[i].m_strItemName;
				cout << "=" << secureInfo.m_SecureItem[i].m_strItemValue << endl;
			}


			//print hexa encoded secure data to copy source file.
			FILE *fpcpp=fopen("g_PlatformInfo_NexSDKInfo.txt", "wb");
			if ( fpcpp != NULL ) {
				char sBuf[32];
				for (int i=0 ; i<dllDataInfoLen ; i++)
				{
					//memset(sBuf, 0x00, sizeof(sBuf));
					if (i%20 == 0) {
						sprintf(sBuf, "\n");
						fwrite(sBuf, sizeof(BYTE), strlen(sBuf), fpcpp);
						//memset(sBuf, 0x00, sizeof(sBuf));
					}
					sprintf(sBuf, "0x%0x, ", *(pDLLData + dllDataInfoPos+i));
					fwrite(sBuf, sizeof(BYTE), strlen(sBuf), fpcpp);
				}
				fclose(fpcpp);

				fprintf(stderr, "Please replace g_PlatformInfo_NexSDKInfo.txt to g_PlatformInfo of NexSDKInfo.txt file.\n");
			}

			// Make clean text from base64 encoded data to review.
			{
				int writeLen=0;
				BYTE *pBuf = new BYTE[dllDataInfoLen];
			
				memset(pBuf, 0x00, dllDataInfoLen);
				writeLen=Base64ToBits(pBuf, dllDataInfoLen, (unsigned char*)pDLLData + dllDataInfoPos, (int)dllDataInfoLen);
				writeDataToFile((char *)"new_secure_decrypt.txt", pBuf, (unsigned int)writeLen);
				fprintf(stderr, "Please review new_secure_decrypt.txt file. writeLen=%d\n", writeLen);
			}
			
			return;
		}
	}
}

void updateProperties(int argc, char *argv[])
{
	int nOutLen;
	int size;
	NEXSECUREITEM *properties = NULL;

	for (dllDataInfoPos = 0; dllDataInfoPos < dllDataSize - sizeof(g_InfoFlag); dllDataInfoPos++)
	{
		if (memcmp(pDLLData + dllDataInfoPos, g_InfoFlag, sizeof(BYTE)*20) == 0)
		{
			for (dllDataInfoLen = 0; dllDataInfoLen < dllDataSize - dllDataInfoPos - 1; dllDataInfoLen++)
			{
				if (*(pDLLData + dllDataInfoPos + dllDataInfoLen) == 0x3d &&
					*(pDLLData + dllDataInfoPos + dllDataInfoLen + 1) == 0)
				{
					dllDataInfoLen += 2;
					break;
				}
			}
			if (NexSecure_SetBitsData(pDLLData + dllDataInfoPos, dllDataInfoLen) != NEXSECURE_ERROR_NONE)
				handleError(&secureInfo, pDLLData, pDllFile);
			if (NexSecure_GetInfo(&secureInfo) != NEXSECURE_ERROR_NONE)
				handleError(&secureInfo, pDLLData, pDllFile);
			int maxCount = getMaxCount();
			if (secureInfo.m_uiSecureItemCount > maxCount)
				handleError(&secureInfo, pDLLData, pDllFile);

			break;
		}
	}

#if defined(DEBUG_MODE)
	for (int i = 0; i <  secureInfo.m_uiSecureItemCount; i++)
	{
		fprintf(stderr, "key : %s, value : %s\n", secureInfo.m_SecureItem[i].m_strItemName,
				secureInfo.m_SecureItem[i].m_strItemValue);
	}
#endif

	if (pDLLData == NULL)
	{
		fprintf(stderr, "Something is wrong with the DLL library\n");
		return;
	}

	if (secureInfo.m_uiSecureItemCount == 0)
	{
		fprintf(stderr, "Something is wrong as to the number of items\n");
		memset(&secureInfo, 0x00, sizeof(NEXSECUREINFO));
		strcpy(secureInfo.m_strKey, "NEXTREAMING.COM");
	}
	else
	{
		bool bNewProperty=true;
		int nexSecureItemSize = getItemSize();
		//properties = (NEXSECUREITEM*)malloc((argc - 3) * nexSecureItemSize);		
		properties = (NEXSECUREITEM*)malloc((argc - 3) * (nexSecureItemSize+ITEM_NAME_MAX_LEN));
		getPropertyUpdated(argc, argv, properties, &size);
		for (int i = 0; i < size; i++)
		{
			bNewProperty = true;
			for (int j = 0; j < secureInfo.m_uiSecureItemCount; j++)
			{
				if (strcmp((properties + i)->m_strItemName, secureInfo.m_SecureItem[j].m_strItemName) == 0)
				{
					bNewProperty = false;
					//fprintf(stderr, "Update property! [%s/%s]. m_uiSecureItemCount=%d\n", (properties + i)->m_strItemName, (properties + i)->m_strItemValue, secureInfo.m_uiSecureItemCount);
					secureInfo.m_SecureItem[j].m_strItemValue = new char[nexSecureItemSize];
					strcpy(secureInfo.m_SecureItem[j].m_strItemValue, (properties + i)->m_strItemValue);
					break;
				}
			}

			if ( bNewProperty ) {
				strcpy(secureInfo.m_SecureItem[secureInfo.m_uiSecureItemCount].m_strItemName, (properties + i)->m_strItemName);
				secureInfo.m_SecureItem[secureInfo.m_uiSecureItemCount].m_strItemValue = new char[nexSecureItemSize];
				strcpy(secureInfo.m_SecureItem[secureInfo.m_uiSecureItemCount].m_strItemValue, (properties + i)->m_strItemValue);
				secureInfo.m_uiSecureItemCount++;
				//fprintf(stderr, "It is new property! [%s/%s]. m_uiSecureItemCount=%d\n", (properties + i)->m_strItemName, (properties + i)->m_strItemValue, secureInfo.m_uiSecureItemCount);
			}
		}

		//free is not mandatory...program is closing just after update finished
		//free(properties);
	}

#if defined(DEBUG_MODE)
	for (int i = 0; i <  secureInfo.m_uiSecureItemCount; i++)
	{
		fprintf(stderr, "key : %s, value : %s\n", secureInfo.m_SecureItem[i].m_strItemName,
				secureInfo.m_SecureItem[i].m_strItemValue);
	}
#endif

	if (NexSecure_SetInfo(&secureInfo) != NEXSECURE_ERROR_NONE)
		handleError(&secureInfo, pDLLData, pDllFile);
	if (NexSecure_GetBitsData(pDLLData + dllDataInfoPos, dllDataInfoLen, &nOutLen) != NEXSECURE_ERROR_NONE)
		handleError(&secureInfo, pDLLData, pDllFile);
	if (dllDataInfoLen != nOutLen)
		handleError(&secureInfo, pDLLData, pDllFile);

	pDllFile = fopen(argv[1], "wb");
	if (pDllFile == NULL)
	{
		fprintf(stderr, "Error occurs when opening a file to write");
		handleError(&secureInfo, pDLLData, pDllFile);
	}

	fwrite(pDLLData, sizeof(BYTE), dllDataSize, pDllFile);
#if defined(DEBUG_MODE)
	fprintf(stderr, "dllDataSize : %ld\n", dllDataSize);
#endif
	fclose(pDllFile);

	return;
}

void getPropertyUpdated(int argc, char *argv[], NEXSECUREITEM *property, int *size)
{
	int index = 0;
	const char *delimiter = "=";
	char *str;
	int itemSize = getItemSize();

	for (int i = 3; i < argc; i++)
	{
		str = strtok(argv[i], delimiter);       
		strcpy((property + index)->m_strItemName, trim(str));
		str = strtok(NULL, delimiter);
		(property + index)->m_strItemValue = new char[itemSize];
		strcpy((property + index)->m_strItemValue, trim(str));
		index++;
	}

	*size = index;
}

void handleError(NEXSECUREINFO *secureInfo, BYTE *pDLLData, FILE *pDllFile)
{
	memset(secureInfo, 0x00, sizeof(NEXSECUREINFO));
	strcpy(secureInfo->m_strKey, "NEXTREAMING.COM");
	if (pDLLData)
	{
		delete [] pDLLData;
		pDLLData = NULL;
	}

	if (pDllFile)
	{
		fclose(pDllFile);
	}

	exit(EXIT_FAILURE);
}

int getMaxCount()
{
	cout << "Version: " << secureInfo.m_strKey << endl;
	if (strcmp(secureInfo.m_strKey, SECURE_KEY_V2) == 0)
	{
		return ITEM_MAX_COUNT_V2;
	}
	if (strcmp(secureInfo.m_strKey, SECURE_KEY_V3) == 0)
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
	if (strcmp(secureInfo.m_strKey, SECURE_KEY_V2) == 0)
	{
		return ITEM_VALUE_MAX_LEN_V2;
	}
	if (strcmp(secureInfo.m_strKey, SECURE_KEY_V3) == 0)
	{
		return ITEM_VALUE_MAX_LEN_V3;
	}
	else
	{
		return ITEM_VALUE_MAX_LEN;
	}
}

char* rtrim(char* s)
{
	char t[4000];
	char *end;

	strcpy(t, s); 
	end = t + strlen(t) - 1;
	while (end != t && isspace(*end))
		end--;
	*(end + 1) = '\0';
	s = t;

	return s;
}

char* ltrim(char *s)
{
	char* begin;
	begin = s;

	while (*begin != '\0')
	{
		if (isspace(*begin))
			begin++;
		else
		{
			s = begin;
			break;
		}
	}

	return s;
}

char* trim(char *s)
{
	return rtrim(ltrim(s));
}
