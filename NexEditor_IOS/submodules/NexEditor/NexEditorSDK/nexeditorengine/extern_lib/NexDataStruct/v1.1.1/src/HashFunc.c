#include "HashFunc.h"


NXINT32 HashFunc(NXVOID *a_pKey, NXINT32 a_nTableSize) 
{
	NXCHAR *pStr = a_pKey;
	NXINT32 nVal = 0;

	while (*pStr != '\0')
	{
		NXINT32 nTemp = 0;

		nVal = (nVal << 4) + (*pStr);

		if ((nTemp = (nVal & 0xf0000000)))
		{
			nVal = nVal ^ (nTemp >> 24);
			nVal = nVal ^ nTemp;
		}

		pStr++;
	}

	return nVal % a_nTableSize;
}
