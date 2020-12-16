/******************************************************************************
* File Name        : utils.h
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

#ifndef _NEXCRAL_MC_UTILS_H_
#define _NEXCRAL_MC_UTILS_H_

#ifndef LOG_TAG
#define LOG_TAG "nexcral_mc"
#endif

//#include <utils/Errors.h>

#include "utils/debug.h"
#include "NexSAL_Internal.h"
#include <semaphore.h>

#define EXPORTED __attribute__((__visibility__("default")))

#ifndef NULL
#define NULL (0)
#endif

#define xstr(s) str(s)
#define str(s) #s

#define IS_BINARY_FLAG_SET(a, b) (((a) & (b)) == (b))

#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

#define ROUNDUP(x, y) (((x)+((y)-1)) & (-1 * (y)))

#ifndef MIN
#define MIN(x, y) (((x)>(y)) ? (y):(x))
#endif

#ifndef MAX
#define MAX(x, y) (((x)>(y)) ? (x):(y))
#endif

#define EXTERNAL_CALL_WITH_BOTH_ACTIONS(name, loglevel, actual_call, exc_act, ok_act)              \
do                                                                                                 \
{                                                                                                  \
    STATUS(Nex_MC::Log::EXTERNAL_CALL, loglevel, "->" #name);                                      \
    int extcallret = actual_call;                                                                  \
    STATUS(Nex_MC::Log::EXTERNAL_CALL, loglevel, #name "->");                                      \
    if (0 != extcallret)                                                                           \
    {                                                                                              \
        MC_ERR("Exception occurred while calling " #name " (%d)", extcallret);                     \
        mc->exceptionState = true;                                                                 \
        exc_act;                                                                                   \
    }                                                                                              \
    else                                                                                           \
    {                                                                                              \
        ok_act;                                                                                    \
    }                                                                                              \
} while (0)

#define EXTERNAL_CALL(name, loglevel, actual_call) EXTERNAL_CALL_WITH_BOTH_ACTIONS(name, loglevel, actual_call, , )
#define EXTERNAL_CALL_WITH_ACTION(name, loglevel, actual_call, exc_act) EXTERNAL_CALL_WITH_BOTH_ACTIONS(name, loglevel, actual_call, exc_act, )

namespace Nex_MC {
namespace Utils {
	typedef int32_t status_t;

	typedef unsigned int ue_t;
	typedef signed int se_t;

	class BitStreamReader {
	public:
		BitStreamReader(const uint8_t *_pIn, const int _inLen);
		virtual ~BitStreamReader();

		virtual bool ReadBit();
		unsigned long long ReadBits(int numBits /*1-64*/);
		ue_t ue();
		se_t se();

		const uint8_t *pIn;
		int inLen;
		uint8_t bitOffset;
	};

	class BitStreamWriter {
	public:
		BitStreamWriter(uint8_t *_pOut);
		virtual ~BitStreamWriter();

		virtual void WriteBit(bool setBit);
		virtual void WriteBits(bool setBit, int numBits);
		void WriteValue(unsigned long long value, int numBits);
		void Write_ue(ue_t ueToWrite);
		void Write_se(se_t seToWrite);

		uint8_t *pOut;
		uint8_t bitOffset;
	};

	const char *ColorFormatString(size_t type);

	status_t GetLineSize_planar(int width, int lineSizes[3]);
	status_t GetLineSize_semiPlanar(int width, int lineSizes[3]);

	int SemaphoreTimeWait(sem_t* pSema, unsigned int intervalMiliSec = 1000);

	bool MakeFourCCString( unsigned int in, char out[5] );
	unsigned int FourCCStringToU32( const char in[4] );
	int GetSystemProperty(const char*,char*);

	namespace ChipType {
		typedef enum ChipType {
			  UNDEFINED = 0
			, EDEN
			, EXYNOS4
			, EXYNOS
			, MARVELL
			, MTK
			, ROCK
			, QUALCOMM_7XXX
			, QUALCOMM_8974
			, QUALCOMM_8XXX
			, X86_Device
			, ENUM_PADDING = 0x7FFFFFFF
		} ChipType;
	}

	ChipType::ChipType GetChipType();

	namespace Manufacturer {
		typedef enum Manufacturer {
			  UNDEFINED = 0
			, LGE
			, SAMSUNG
			, ENUM_PADDING = 0x7FFFFFFF
		} Manufacturer;
	}

	Manufacturer::Manufacturer GetManufacturer();
	int GetBuildVersionSDK();

	unsigned int readBigEndianValue8( unsigned char* pData );
	unsigned int readBigEndianValue16( unsigned char* pData );
	unsigned int readBigEndianValue24( unsigned char* pData );
	unsigned int readBigEndianValue32( unsigned char* pData );


	class Initializer {
	protected:
		virtual ~Initializer() {}
	public:
		virtual void initialize() = 0;
		virtual void deinitialize(bool force) = 0;
	};

	void registerInitializer(Initializer *initializer);
	void unregisterInitializer(Initializer *initializer);
	void initializeAll();
	void deinitializeAll();


	// WrapSetProperty class by Eric.
	// purpose : engine can call SetProperty before calling Init
	// 
	
	class WrapSetProperty
	{
	public:
		static const int MAX_PROPERTY_VALUE_PAIR = 4;
		typedef enum {
			NONE_EXIST = 0,
			ID_ONLY_EXIST,
			USERDATA_EXIST
		} CheckRet;

		typedef enum {
			NONE = 0,
			VALID_PAIR
		} PopRet;
		
		typedef struct _properties_per_mc_
		{
			//void* hMC;
			NXUINT32 uProperty;
			NXINT64 qValue;
		} _PROPERTY;

		typedef struct _encoder_mc_id_tbl_
		{
			void *id;
			void *pMC;
			_PROPERTY property_value[4];
			int nCount;
			int nPopIndex;
		} _PROPERTY_PER_CODEC;
	
		WrapSetProperty();
		~WrapSetProperty();
		CheckRet CheckClientIdExist(void* ClientId);
		int RegisterClientId(void* ClientId);
		int RegisterUserData(void* pUserData, void* ClientId);
		int UnregisterUserData(void* pUserData);
		int PushPropertyAndValuePair( void* ClientId, NXUINT32 uProperty, NXINT64 qValue );
		PopRet PopPropertyAndValuePair( void* ClientId, NXUINT32 *puProperty, NXINT64 *pqValue );

	private:
		int IncreaseEntry();
	private:
		_PROPERTY_PER_CODEC* pEncoderMCs;
		int nMaxEncoderMCs;
	};

	
}; // namespace Utils
}; // namespace Nex_MC
//using namespace Nex_MC::Utils;

#endif //#ifndef _NEXCRAL_MC_UTILS_H_

