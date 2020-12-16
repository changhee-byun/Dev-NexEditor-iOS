/******************************************************************************
* File Name        : utils.cpp
* Description      : utilities for NexCRALBody_MC
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

#define LOCATION_TAG "Utils"

#define LOG_TAG "nexcral_mc"
#include "./utils.h"

#include <time.h>
#include <sys/time.h>

#include <pthread.h>

#include <sys/system_properties.h>

#include <vector>
#include <stdio.h>

// for differentiating between encoder and decoder in logs (U == undefined)
#define NEX_MC_TYPE ('U' - 'D')

// using android::OK;
#define OK (0);

#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
#include <android/log.h>
#include <dlfcn.h>

typedef int (*PFN_SYSTEM_PROP_GET)(const char *, char *);
static int __nex_system_property_get(const char* name, char* value)
{
    static PFN_SYSTEM_PROP_GET __real_system_property_get = NULL;
    if (!__real_system_property_get) {
        // libc.so should already be open, get a handle to it.
        void *handle = dlopen("libc.so", RTLD_NOLOAD);
        if (!handle) {
            __android_log_print(ANDROID_LOG_ERROR, "foobar", "Cannot dlopen libc.so: %s.\n", dlerror());
        } else {
            __real_system_property_get = (PFN_SYSTEM_PROP_GET)dlsym(handle, "__system_property_get");
        }
        if (!__real_system_property_get) {
            __android_log_print(ANDROID_LOG_ERROR, "foobar", "Cannot resolve __system_property_get(): %s.\n", dlerror());
        }
    }
    return (*__real_system_property_get)(name, value);
} 
#endif

namespace { // (anon0)
	class Initializers {
	public:
		Initializers()
		{
			pthread_mutex_init(&initializersLock, NULL);
		}
		~Initializers()
		{
			deinitializeAll(true);

			pthread_mutex_destroy(&initializersLock);
		}
		void registerInitializer(Nex_MC::Utils::Initializer *initializer)
		{
			pthread_mutex_lock(&initializersLock);
			initializers.push_back(initializer);
			pthread_mutex_unlock(&initializersLock);
		}
		void unregisterInitializer(Nex_MC::Utils::Initializer *initializer)
		{
			pthread_mutex_lock(&initializersLock);
			std::vector<Nex_MC::Utils::Initializer *>::iterator it;
			for (it = initializers.begin(); initializers.end() != it; ++it)
			{
				if ((*it) == initializer)
				{
					initializers.erase(it);
					break;
				}
			}
			pthread_mutex_unlock(&initializersLock);
		}
		void initializeAll()
		{
			pthread_mutex_lock(&initializersLock);
			std::vector<Nex_MC::Utils::Initializer *>::iterator it;

			for (it = initializers.begin(); initializers.end() != it; ++it)
			{
				(*it)->initialize();
			}
			pthread_mutex_unlock(&initializersLock);
		}
		void deinitializeAll(bool force)
		{
			pthread_mutex_lock(&initializersLock);
			std::vector<Nex_MC::Utils::Initializer *>::reverse_iterator rit;

			for (rit = initializers.rbegin(); initializers.rend() != rit; ++rit)
			{
				(*rit)->deinitialize(force);
			}

			pthread_mutex_unlock(&initializersLock);
		}
	private:
		pthread_mutex_t initializersLock;
		std::vector<Nex_MC::Utils::Initializer *> initializers;
	};
	Initializers initializersInstance;
} // namespace (anon0)

namespace Nex_MC {

namespace Utils {

	#ifndef OMX_TI_COLOR_FormatYUV420PackedSemiPlanar
	#define OMX_TI_COLOR_FormatYUV420PackedSemiPlanar 0x7F000100
	#endif
	#ifndef OMX_QCOM_COLOR_FormatYVU420SemiPlanar
	#define OMX_QCOM_COLOR_FormatYVU420SemiPlanar 0x7FA30C00
	#endif

	const char *ColorFormatString( size_t type )
	{
		static const char *kNames[] = {
			"OMX_COLOR_FormatUnused",
			"OMX_COLOR_FormatMonochrome",
			"OMX_COLOR_Format8bitRGB332",
			"OMX_COLOR_Format12bitRGB444",
			"OMX_COLOR_Format16bitARGB4444",
			"OMX_COLOR_Format16bitARGB1555",
			"OMX_COLOR_Format16bitRGB565",
			"OMX_COLOR_Format16bitBGR565",
			"OMX_COLOR_Format18bitRGB666",
			"OMX_COLOR_Format18bitARGB1665",
			"OMX_COLOR_Format19bitARGB1666",
			"OMX_COLOR_Format24bitRGB888",
			"OMX_COLOR_Format24bitBGR888",
			"OMX_COLOR_Format24bitARGB1887",
			"OMX_COLOR_Format25bitARGB1888",
			"OMX_COLOR_Format32bitBGRA8888",
			"OMX_COLOR_Format32bitARGB8888",
			"OMX_COLOR_FormatYUV411Planar",
			"OMX_COLOR_FormatYUV411PackedPlanar",
			"OMX_COLOR_FormatYUV420Planar",
			"OMX_COLOR_FormatYUV420PackedPlanar",
			"OMX_COLOR_FormatYUV420SemiPlanar",
			"OMX_COLOR_FormatYUV422Planar",
			"OMX_COLOR_FormatYUV422PackedPlanar",
			"OMX_COLOR_FormatYUV422SemiPlanar",
			"OMX_COLOR_FormatYCbYCr",
			"OMX_COLOR_FormatYCrYCb",
			"OMX_COLOR_FormatCbYCrY",
			"OMX_COLOR_FormatCrYCbY",
			"OMX_COLOR_FormatYUV444Interleaved",
			"OMX_COLOR_FormatRawBayer8bit",
			"OMX_COLOR_FormatRawBayer10bit",
			"OMX_COLOR_FormatRawBayer8bitcompressed",
			"OMX_COLOR_FormatL2",
			"OMX_COLOR_FormatL4",
			"OMX_COLOR_FormatL8",
			"OMX_COLOR_FormatL16",
			"OMX_COLOR_FormatL24",
			"OMX_COLOR_FormatL32",
			"OMX_COLOR_FormatYUV420PackedSemiPlanar",
			"OMX_COLOR_FormatYUV422PackedSemiPlanar",
			"OMX_COLOR_Format18BitBGR666",
			"OMX_COLOR_Format24BitARGB6666",
			"OMX_COLOR_Format24BitABGR6666",
		};

		const size_t numNames = sizeof(kNames) / sizeof(kNames[0]);

		if( OMX_TI_COLOR_FormatYUV420PackedSemiPlanar == type )
		{
			return "OMX_TI_COLOR_FormatYUV420PackedSemiPlanar";
		}
		else if ( OMX_QCOM_COLOR_FormatYVU420SemiPlanar == type )
		{
			return "OMX_QCOM_COLOR_FormatYVU420SemiPlanar";
		}
		else if ( numNames <= (size_t)type )
		{
			return "UNKNOWN";
		}
		else
		{
			return kNames[type];
		}
	}

	//==============================================================================
	BitStreamReader::BitStreamReader(const uint8_t *_pIn, const int _inLen)
	 : pIn(_pIn)
	 , inLen(_inLen)
	 , bitOffset(0)
	{
	}
	//==============================================================================
	BitStreamReader::~BitStreamReader()
	{
	}
	//==============================================================================
	bool BitStreamReader::ReadBit()
	{
		bool b = 0 != ((*pIn) & (1 << (7-bitOffset)));

		if (8 <= ++bitOffset)
		{
			++pIn;
			bitOffset = 0;
		}
		return b;
	}
	//==============================================================================
	unsigned long long BitStreamReader::ReadBits(int numBits)
	{
		unsigned long long temp = 0;

		for (int i = 0; i < numBits; ++i)
		{
			temp = (temp << 1) | ReadBit();
		}

		return temp;
	}
	//==============================================================================
	ue_t BitStreamReader::ue()
	{
		int leadingZeroBits = -1;
		for (bool b = false; !b; ++leadingZeroBits)
		{
			b = ReadBit();
		}
		ue_t ret = (1 << leadingZeroBits) - 1 + ReadBits(leadingZeroBits);

		return ret;
	}
	//==============================================================================
	se_t BitStreamReader::se()
	{
		ue_t ret = ue();
		return ((ret % 2) == 1 ? 1 : -1) * ((ret+1) / 2);
	}
	//==============================================================================
	BitStreamWriter::BitStreamWriter(uint8_t *_pOut)
	 : pOut(_pOut)
	 , bitOffset(0)
	{
	}
	//==============================================================================
	BitStreamWriter::~BitStreamWriter()
	{
	}
	//==============================================================================
	void BitStreamWriter::WriteBit(bool setBit)
	{
		if (setBit)
		{
			*pOut |= (1 << (7-bitOffset));
		}
		else
		{
			*pOut &= ~(1 << (7-bitOffset));
		}

		if (8 <= ++bitOffset)
		{
			++pOut;
			bitOffset = 0;
		}
	}
	//==============================================================================
	void BitStreamWriter::WriteBits(bool setBit, int numBits)
	{
		while (0 < numBits)
		{
			if (0 == bitOffset && 8 <= numBits)
			{
				*pOut = setBit ? 0xFF : 0x00;
				++pOut;
				numBits -= 8;
			}
			else
			{
				WriteBit(setBit);
				--numBits;
			}
		}
	}
	void BitStreamWriter::WriteValue(unsigned long long value, int numBits)
	{
		while (0 < numBits)
		{
			--numBits;
			WriteBit(0 != (value & (1 << numBits)));
		}
	}
	//==============================================================================
	void BitStreamWriter::Write_ue(ue_t ueToWrite)
	{
		int leadingZeroBits = 0;

		ue_t tmp = (ueToWrite + 1) >> 1;

		while (0 < tmp)
		{
			++leadingZeroBits;
			tmp >>= 1;
		}

		ue_t trailingBits = ueToWrite + 1 - (1 << leadingZeroBits);

		WriteBits(false, leadingZeroBits);
		WriteBit(true);

		WriteValue(trailingBits, leadingZeroBits);
	}
	//==============================================================================
	void BitStreamWriter::Write_se(se_t seToWrite)
	{
		ue_t ueToWrite = 0;

		if (0 < seToWrite)
		{
			ueToWrite = seToWrite * 2 - 1;
		}
		else if (0 > seToWrite)
		{
			ueToWrite = seToWrite * -2;
		}

		Write_ue(ueToWrite);
	}
	//==============================================================================
	status_t GetLineSize_planar(int width, int lineSizes[3])
	{
		//	maxstep = {1, 1, 1}; maxstepcomp = {0, 1, 2};
		lineSizes[0] = width;
		lineSizes[1] =
			lineSizes[2] = (width + 1) >> 1;
		return OK;
	}
	//==============================================================================
	status_t GetLineSize_semiPlanar(int width, int lineSizes[3])
	{
		//	maxstep = {1, 2}; maxstepcomp = {0, 1/2};
		lineSizes[0] = width;
		lineSizes[1] =
			lineSizes[2] = ((width + 1) >> 1) << 1;
		return OK;
	}
	//==============================================================================
	int SemaphoreTimeWait(sem_t* pSema, unsigned int intervalMiliSec /*= 1000*/)
	{
		if(NULL == pSema)
		{
			MC_ERR("pSema is NULL");
			return -10;
		}

		int retVal = 0;
		struct timespec ts;
		struct timeval now;

		gettimeofday(&now, NULL);
		ts.tv_sec = now.tv_sec + intervalMiliSec / 1000;
		ts.tv_nsec = now.tv_usec * 1000 + (intervalMiliSec % 1000) * 1000000;
		if(ts.tv_nsec > 1000000000)
		{
			ts.tv_sec += 1;
			ts.tv_nsec -= 1000000000;
		}

		retVal = sem_timedwait(pSema, &ts);

		return retVal;
	}

	bool isValid4CCChar( char c )
	{
//		return 0x20 <= c && 0x7E >= c;
		return 0x30 <= c && 0x7A >= c;
	}

	bool MakeFourCCString( unsigned int in, char out[5] )
	{
		out[0] = in >> 24;
		out[1] = (in >> 16) & 0xff;
		out[2] = (in >> 8) & 0xff;
		out[3] = in & 0xff;
		out[4] = '\0';
		if (!isValid4CCChar(out[0]) || !isValid4CCChar(out[1]) || !isValid4CCChar(out[2]) || !isValid4CCChar(out[3]))
			return false;
		return true;
	}

	unsigned int FourCCStringToU32( const char in[4] )
	{
		return ((unsigned int)in[0] << 24)
				| ((unsigned int)in[1] << 16)
				| ((unsigned int)in[2] << 8)
				| in[3];
	}

	int GetSystemProperty(const char* p,char* v)
	{
#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
		return __nex_system_property_get(p,v);
#else
		return __system_property_get(p,v);
#endif
	}

namespace { // anon123
	ChipType::ChipType g_chipType = ChipType::UNDEFINED;
	Manufacturer::Manufacturer g_manufacturer = Manufacturer::UNDEFINED;
	int g_buildVersionSDK = 0;

	bool IsQcom()
	{
		return -1 != access("/sys/devices/system/soc/soc0/build_id", F_OK) // msm
			|| -1 != access("/sys/devices/soc0/build_id", F_OK); // sdm
	}

	bool IsLSI_Exynos4()
	{
		return -1 != access("/sys/devices/system/exynos4-core", F_OK); // exynos4
	}

	bool IsLSI()
	{
		return -1 != access("/sys/devices/system/exynos4-core", F_OK) // exynos4
			|| -1 != access("/sys/devices/system/exynos5-core", F_OK) // exynos5
			|| -1 != access("/sys/devices/system/exynos-core", F_OK); // exynos? Note4, 5.0.1, SM-N910S // added by eric 2015-04-09
	//	return -1 != access("/sys/devices/system/s5pv310-core", F_OK); // GS2
	//	return -1 != access("/sys/devices/system/s5pv210-core", F_OK); // GS
	}

	class InitChipTypeAndManufacturer
	{
	public:
		InitChipTypeAndManufacturer()
		{
			char nexSysPropertys[PROP_VALUE_MAX];
			int nexPropertyLen = 0;
			memset( nexSysPropertys, 0x00, PROP_VALUE_MAX );

			g_chipType = ChipType::UNDEFINED;
			g_manufacturer = Manufacturer::UNDEFINED;

			nexPropertyLen = GetSystemProperty("ro.board.platform", nexSysPropertys);
			//nexPropertyLen = __system_property_get("ro.board.platform", nexSysPropertys);

			if (0 < nexPropertyLen)
			{
				if (!strncmp("eden", nexSysPropertys, sizeof("eden") - 1))
				{
					g_chipType = ChipType::EDEN;
				}
				else if (!strncmp("mrvl", nexSysPropertys, sizeof("mrvl") - 1))
				{
					g_chipType = ChipType::MARVELL;
				}
				else if (!strncmp("rk", nexSysPropertys, sizeof("rk") - 1))
				{
					g_chipType = ChipType::ROCK;
				}
				else if (!strncmp("msm8974", nexSysPropertys, sizeof("msm8974") - 1))
				{
					g_chipType = ChipType::QUALCOMM_8974;
				}
				else if (!strncmp("msm8", nexSysPropertys, sizeof("msm8") - 1))
				{
					g_chipType = ChipType::QUALCOMM_8XXX;
				}
				else if (!strncmp("msm7", nexSysPropertys, sizeof("msm7") - 1))
				{
					g_chipType = ChipType::QUALCOMM_7XXX;
				}
			}

			if (ChipType::UNDEFINED == g_chipType)
			{
				if (IsLSI_Exynos4())
				{
					g_chipType = ChipType::EXYNOS4;
				}
				else if (IsLSI())
				{
					g_chipType = ChipType::EXYNOS;
				}
			}

			if (ChipType::UNDEFINED == g_chipType)
			{
				nexPropertyLen = GetSystemProperty("ro.hardware", nexSysPropertys);
				if (0 < nexPropertyLen)
				{
					if (!strncmp("mt6", nexSysPropertys, sizeof("mt6") - 1))
					{
						g_chipType = ChipType::MTK;
					}
				}
			}

			if (ChipType::UNDEFINED == g_chipType)
			{
				nexPropertyLen = GetSystemProperty("ro.product.cpu.abi", nexSysPropertys);
				if (0 < nexPropertyLen)
				{
					if (!strncmp("x86", nexSysPropertys, sizeof("x86") - 1))
					{
						g_chipType = ChipType::X86_Device;
					}
				}
			}

			nexPropertyLen = GetSystemProperty("ro.product.manufacturer", nexSysPropertys);

			if (0 < nexPropertyLen)
			{
				if (!strncmp("samsung", nexSysPropertys, sizeof("samsung") - 1))
				{
					g_manufacturer = Manufacturer::SAMSUNG;
				}
				else if (!strncmp("LGE", nexSysPropertys, sizeof("LGE") - 1) || !strncmp("lge", nexSysPropertys, sizeof("lge") - 1))
				{
					g_manufacturer = Manufacturer::LGE;
				}
			}
		}
	};

	InitChipTypeAndManufacturer initChipTypeAndManufacturer;

	class InitPlatform
	{
	public:
		InitPlatform()
		{
			char nexSysPropertys[PROP_VALUE_MAX];
			int nexPropertyLen = 0;
			memset( nexSysPropertys, 0x00, PROP_VALUE_MAX );

			g_buildVersionSDK = 0;
			
			nexPropertyLen = GetSystemProperty("ro.build.version.sdk", nexSysPropertys);

			if (0 < nexPropertyLen)
			{
				g_buildVersionSDK = atoi(nexSysPropertys);
			}

			//[ro.product.model]: [Nexus 10]
		}
	};
	InitPlatform initPlatform;
}; // namespace (anon123)

	ChipType::ChipType GetChipType()
	{
		return g_chipType;
	}

	Manufacturer::Manufacturer GetManufacturer()
	{
		return g_manufacturer;
	}
	int GetBuildVersionSDK()
	{
		return g_buildVersionSDK;
	}

	unsigned int readBigEndianValue8( unsigned char* pData )
	{
		unsigned int usRet = 0;

		usRet |= ((unsigned int)pData[0]	 )&0x00ff;

		return usRet;
	}

	unsigned int readBigEndianValue16( unsigned char* pData )
	{
		unsigned int usRet = 0;

		usRet |= ((unsigned int)pData[0] << 8)&0xff00;
		usRet |= ((unsigned int)pData[1]     )&0x00ff;

		return usRet;
	}

	unsigned int readBigEndianValue24( unsigned char* pData )
	{
		unsigned int uRet = 0;

		uRet |= ((unsigned int)pData[0]<<16)&0x00ff0000;
		uRet |= ((unsigned int)pData[1]<<8 )&0x0000ff00;
		uRet |= ((unsigned int)pData[2]    )&0x000000ff;

		return uRet;
	}

	unsigned int readBigEndianValue32( unsigned char* pData )
	{
		unsigned int uRet = 0;

		uRet |= ((unsigned int)pData[0]<<24)&0xff000000;
		uRet |= ((unsigned int)pData[1]<<16)&0x00ff0000;
		uRet |= ((unsigned int)pData[2]<<8 )&0x0000ff00;
		uRet |= ((unsigned int)pData[3]    )&0x000000ff;

		return uRet;
	}

	void registerInitializer(Initializer *initializer)
	{
		initializersInstance.registerInitializer(initializer);
	}

	void unregisterInitializer(Initializer *initializer)
	{
		initializersInstance.unregisterInitializer(initializer);
	}

	void initializeAll()
	{
		initializersInstance.initializeAll();
	}

	void deinitializeAll()
	{
		initializersInstance.deinitializeAll(false);
	}


	//
	// Implementation part of WrapSetProperty class
	// 

	WrapSetProperty::WrapSetProperty()
		: pEncoderMCs(NULL)
		, nMaxEncoderMCs(0)
	{
		MC_ERR("WarpSetProperty constructor.");
	}

	WrapSetProperty::~WrapSetProperty()
	{
		if ( pEncoderMCs )
		{
			free(pEncoderMCs);
			pEncoderMCs = NULL;
		}
		MC_ERR("WarpSetProperty destructor.");
	}

	WrapSetProperty::CheckRet WrapSetProperty::CheckClientIdExist(void* ClientId)
	{
		int i;
		for ( i = 0 ; i < nMaxEncoderMCs ; i++ )
		{
			if ( ClientId == pEncoderMCs[i].pMC )
			{
				MC_ERR("WarpSetProperty CheckClientIdExist return UD_EXIST.");
				return USERDATA_EXIST;
			}
			else if ( ClientId == pEncoderMCs[i].id )
			{
				MC_ERR("WarpSetProperty CheckClientIdExist return ID_EXIST.");
				return ID_ONLY_EXIST;
			}
		}

		if ( i == nMaxEncoderMCs )
		{
			MC_ERR("WarpSetProperty CheckClientIdExist return NONE_EXIST.");
			return NONE_EXIST;
		}
		else
		{
			MC_ERR("WarpSetProperty CheckMCExist, why???");
			return NONE_EXIST;
		}
	}

	int WrapSetProperty::IncreaseEntry()
	{
		MC_ERR("WarpSetProperty IncreaseEntry. pre=%d",nMaxEncoderMCs);
		_PROPERTY_PER_CODEC* pTemp = pEncoderMCs;
		int nPreSize = nMaxEncoderMCs;
		nMaxEncoderMCs = MAX( 4, 2*nMaxEncoderMCs );
		pEncoderMCs = (_PROPERTY_PER_CODEC*)malloc(sizeof(_PROPERTY_PER_CODEC)*nMaxEncoderMCs);
		memset( pEncoderMCs, 0x00, sizeof(_PROPERTY_PER_CODEC)*nMaxEncoderMCs );

		if ( nPreSize )
		{
			memcpy( pEncoderMCs, pTemp, sizeof(_PROPERTY_PER_CODEC)*nPreSize );
			if ( pTemp ) free(pTemp);
		}
		return 0;
	}

	int WrapSetProperty::RegisterClientId(void* ClientId)
	{
		int i;
		for ( i = 0 ; i < nMaxEncoderMCs ; i++ )
		{
			if ( NULL == pEncoderMCs[i].id )
				break;
		}
		
		if ( i == nMaxEncoderMCs )
		{
			IncreaseEntry();
		}

		pEncoderMCs[i].id = ClientId;
		pEncoderMCs[i].pMC = NULL;
		pEncoderMCs[i].nCount = 0;
		pEncoderMCs[i].nPopIndex = 0;

		MC_ERR("WarpSetProperty RegisterClientId i=%d,%p",i,ClientId);
		return 0;
	}

	int WrapSetProperty::PushPropertyAndValuePair( void* ClientId, NXUINT32 uProperty, NXINT64 qValue )
	{
		int i;
		MC_ERR("WarpSetProperty PushPropertyAndValuePair c=%p,(u,v)=(%d,%lld)",ClientId,uProperty,qValue);
		for ( i = 0 ; i < nMaxEncoderMCs ; i++ )
		{
			if ( ClientId == pEncoderMCs[i].id )
				break;
		}
		if ( i == nMaxEncoderMCs )
		{
			MC_ERR("PushPropertyAndValuePair, there is no ClientId.");
			return -1;
		}
		if ( pEncoderMCs[i].pMC )
		{
			MC_ERR("PushPropertyAndValuePair, there is MC.");
			return -2;
		}
		if ( pEncoderMCs[i].nCount >= MAX_PROPERTY_VALUE_PAIR )
		{
			MC_ERR("PushPropertyAndValuePair, overflow pairs.");
			return -2;
		}
		
		MC_ERR("WarpSetProperty PushPropertyAndValuePair index=%d,count=%d",i,pEncoderMCs[i].nCount);
		pEncoderMCs[i].property_value[pEncoderMCs[i].nCount].uProperty = uProperty;
		pEncoderMCs[i].property_value[pEncoderMCs[i].nCount].qValue = qValue;
		pEncoderMCs[i].nCount++;

		return 0;		
	}

	
	WrapSetProperty::PopRet WrapSetProperty::PopPropertyAndValuePair( void* ClientId, NXUINT32 *puProperty, NXINT64 *pqValue )
	{
		int i;
		MC_ERR("WarpSetProperty PopPropertyAndValuePair c=%p",ClientId);
		if ( NULL == ClientId )
			return NONE;

		for ( i = 0 ; i < nMaxEncoderMCs ; i++ )
		{
			if ( ClientId == pEncoderMCs[i].id )
				break;
		}
		if ( i == nMaxEncoderMCs )
			return NONE;

		if ( pEncoderMCs[i].nCount == pEncoderMCs[i].nPopIndex )
			return NONE;

		*puProperty = pEncoderMCs[i].property_value[pEncoderMCs[i].nPopIndex].uProperty;
		*pqValue = pEncoderMCs[i].property_value[pEncoderMCs[i].nPopIndex].qValue;

		pEncoderMCs[i].nPopIndex++;

		MC_ERR("WarpSetProperty PopPropertyAndValuePair i=%d,(u,v)=(%d,%lld)",i,*puProperty,*pqValue);
		return VALID_PAIR;
	}


	int WrapSetProperty::RegisterUserData( void* ClientId, void* pUserData )
	{
		int i;
		for ( i = 0 ; i < nMaxEncoderMCs ; i++ )
		{
			if ( ClientId == pEncoderMCs[i].id )
				break;
		}

		if ( i < nMaxEncoderMCs )
		{
			pEncoderMCs[i].pMC = pUserData;
			MC_ERR("WarpSetProperty RegisterUserData i=%d, set userdata to list registered",i);
			return 0;
		}

		for ( i = 0 ; i < nMaxEncoderMCs ; i++ )
		{
			if ( NULL == pEncoderMCs[i].id )
				break;
		}
		
		if ( i == nMaxEncoderMCs )
		{
			IncreaseEntry();
		}

		pEncoderMCs[i].id = ClientId;
		pEncoderMCs[i].pMC = pUserData;
		pEncoderMCs[i].nCount = 0;
		pEncoderMCs[i].nPopIndex = 0;

		MC_ERR("WarpSetProperty RegisterUserData, i=%d, MaxEncoderC=%d.", i, nMaxEncoderMCs);

		return 0;
	}

	int WrapSetProperty::UnregisterUserData(void* pUserData)
	{
		int i;
		for ( i = 0 ; i < nMaxEncoderMCs ; i++ )
		{
			if ( pUserData == pEncoderMCs[i].pMC )
				break;
		}
		if ( i == 4 )
		{
			MC_ERR("WrapSetProperty::UnregisterUserData Couldn't find MC pointer in list.");
			return -1;
		}

		pEncoderMCs[i].pMC = NULL;
		pEncoderMCs[i].id = NULL;
		pEncoderMCs[i].nCount = 0;
		pEncoderMCs[i].nPopIndex = 0;

		MC_ERR("WarpSetProperty UnregisterUserData, i=%d", i);
		return 0;
	}
	
	
}; // namespace Utils
}; // namespace Nex_MC

/*-----------------------------------------------------------------------------
Revision History 
Author		Date			Version		Description of Changes
-------------------------------------------------------------------------------
...
-----------------------------------------------------------------------------*/
