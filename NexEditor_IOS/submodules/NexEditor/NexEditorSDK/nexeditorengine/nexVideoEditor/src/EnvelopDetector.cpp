#include "EnvelopDetector.h"
#include <math.h>


int EnvelopDetector::Init(int nChannels, int nSamplingRate)
{
	m_nSamplingRate = nSamplingRate;
	m_nChannels = nChannels;

	if (nChannels < 1 || nChannels > MAX_CHANNEL)
		return -1;

	for (int i = 0; i < nChannels; i++)
	{
		m_Envelop[i] = 0; // -90 db
		m_EnvelopOut[i] = 0; // -90 db
	}

	m_LR_Envelop = 0; // -90 db

	m_coef_ = (int)((exp( -1000.0 / ( 0.1 * nSamplingRate ) ))*16384);

	m_LR_EnvelopOut = 0; // -90 db

	m_Outputcoef = (int)((exp( -1000.0 / ( 0.1 * nSamplingRate ) ))*16384);

	return 0;
}
// 
//  0 ~ 1 까지의 값을 리턴 한다. 0 은 -90 dB로 무음이고, 1 은 0 dB 로 가장 큰 음이다.

//int EnvelopDetector::EnvelopDetectProcess(short * pIn,float * pEnvelopRMS, float * pEnvelopLeft, float * pEnvelopRight, int nSize) // nSize 는 채널당 sample 수
int EnvelopDetector::EnvelopDetectProcess(short * pIn, float * pEnvelopRMS, float * pEnvelop, int nSize) // nSize 는 채널당 sample 수
{
	
	int nCh = 0;
	//short nInLeft;
	//short nInRight;
	short nInput;
	int	  nInAve;

	short nInPeak[MAX_CHANNEL] = {0,};
	//short nInLeftPeak = 0;
	//short nInRightPeak = 0;
	int	  nInAvePeak = 0;

	for(int i = 0; i < nSize; i++)
	{
		nInAve = 0;
		for (nCh = 0; nCh < m_nChannels; nCh++)
		{
			nInput = *pIn++;
			
			nInAve += nInput;
			
			if (nInput <0)
				nInput = -nInput;

			if (nInPeak[nCh] < nInput)
				nInPeak[nCh] = nInput;

			m_Envelop[nCh] = nInPeak[nCh] + (((m_Envelop[nCh] - nInPeak[nCh])*m_coef_) >> 14);
		}

		nInAve = nInAve / m_nChannels;

		if (nInAve <0)
			nInAve = -nInAve;

		if(nInAvePeak < nInAve)
			nInAvePeak = nInAve;

		m_LR_Envelop = nInAvePeak + (((m_LR_Envelop - nInAvePeak)*m_coef_) >> 14);
		/*
		if(m_nChannels == 2)
		{
			nInLeft = *pIn++;
			nInRight = *pIn++;

			nInAve = (nInLeft+nInRight)>>1;

			if(nInLeft <0)
				nInLeft = -nInLeft;
			if(nInRight <0)
				nInRight = -nInRight;
			if(nInAve <0)
				nInAve = -nInAve;

			if(nInLeftPeak < nInLeft)
				nInLeftPeak = nInLeft;
			if(nInRightPeak < nInRight)
				nInRightPeak = nInRight;
			if(nInAvePeak < nInAve)
				nInAvePeak = nInAve;

			m_LEnvelop =   nInLeftPeak  + (((m_LEnvelop   - nInLeftPeak  )*m_coef_)>>14);
			m_REnvelop =   nInRightPeak + (((m_REnvelop   - nInRightPeak )*m_coef_)>>14);
			m_LR_Envelop = nInAvePeak   + (((m_LR_Envelop - nInAvePeak   )*m_coef_)>>14);
		
		}
		else if(m_nChannels == 1)
		{
			nInLeft = *pIn++;

			if(nInLeft <0)
				nInLeft = -nInLeft;

			if(nInLeftPeak < nInLeft)
				nInLeftPeak = nInLeft;

			m_LEnvelop =   nInLeftPeak  + (((m_LEnvelop   - nInLeftPeak  )*m_coef_)>>14);
			

		}
		*/
	}
	if (pEnvelop != 0)
	{

		for (nCh = 0; nCh < m_nChannels; nCh++)
		{
			m_EnvelopOut[nCh] = m_Envelop[nCh] + (((m_EnvelopOut[nCh] - m_Envelop[nCh])*m_Outputcoef) >> 14);
			pEnvelop[nCh] = m_EnvelopOut[nCh] / 32768.f;
		}
	}
	/*
	if(m_nChannels == 2)
	{
		m_LEnvelopOut	=   m_LEnvelop		+ (((m_LEnvelopOut   - m_LEnvelop	)*m_Outputcoef)>>14);
		m_REnvelopOut	=   m_REnvelop		+ (((m_REnvelopOut   - m_REnvelop	)*m_Outputcoef)>>14);
		m_LR_EnvelopOut =	m_LR_Envelop	+ (((m_LR_EnvelopOut - m_LR_Envelop	)*m_Outputcoef)>>14);
		
		*pEnvelopLeft  = m_LEnvelopOut/16384.f;
		*pEnvelopRight = m_REnvelopOut/16384.f;
		*pEnvelopRMS   = m_LR_EnvelopOut/16384.f;
	}
	else if(m_nChannels == 1)
	{
		m_LEnvelopOut	=   m_LEnvelop		+ (((m_LEnvelopOut   - m_LEnvelop	)*m_Outputcoef)>>14);

		*pEnvelopLeft  = m_LEnvelopOut/16384.f;
		*pEnvelopRight = *pEnvelopLeft;
		*pEnvelopRMS   = *pEnvelopLeft;
	}	
	*/
	m_LR_EnvelopOut = m_LR_Envelop + (((m_LR_EnvelopOut - m_LR_Envelop)*m_Outputcoef) >> 14);
	if (pEnvelopRMS)
		*pEnvelopRMS = m_LR_EnvelopOut / 32768.f;
	
	return 0;
}
int EnvelopDetector::AudioThumbProcess(short * pIn, float * pEnvelopRMS, float * pEnvelop, int nSize) // nSize 는 채널당 sample 수
{
	
	int nCh = 0;
	short nInput;
	int	  nInAve;
	int	  nInAvePeak = 0;

	for(int i = 0; i < nSize; i++)
	{
		nInAve = 0;
		for (nCh = 0; nCh < m_nChannels; nCh++)
		{
			nInput = *pIn++;
			nInAve += nInput;
		}

		nInAve = nInAve / m_nChannels;

		if (nInAve <0)
			nInAve = -nInAve;

		if(nInAvePeak < nInAve)
			nInAvePeak = nInAve;

		m_LR_Envelop = nInAvePeak;
	}
	m_LR_EnvelopOut = m_LR_Envelop;// + (((m_LR_EnvelopOut - m_LR_Envelop)*m_Outputcoef) >> 14);
	if (pEnvelopRMS)
		*pEnvelopRMS = m_LR_EnvelopOut / 32768.f;
	
	return 0;
}

