#ifndef EnvelopDetector_H
#define EnvelopDetector_H

#define MAX_CHANNEL	7

class EnvelopDetector
{
	int m_nSamplingRate;
	int m_nChannels;

	int m_Envelop[MAX_CHANNEL];
	/*
	int m_LEnvelop;
	int m_REnvelop;
	*/
	int m_LR_Envelop;
	int  m_coef_;

	int m_EnvelopOut[MAX_CHANNEL];
	/*
	int m_LEnvelopOut;
	int m_REnvelopOut;
	*/
	int m_LR_EnvelopOut;
	int m_Outputcoef;


public:
	int Init(int nChannels, int nSamplingRate);
	//int EnvelopDetectProcess(short * pIn,float * pEnvelopRMS, float * pEnvelopLeft, float * pEnvelopRight, int nSize); // nSize 는 채널당 sample 수
	int EnvelopDetectProcess(short * pIn, float * pEnvelopRMS, float * pEnvelop, int nSize); // nSize 는 채널당 sample 수	
	int AudioThumbProcess(short * pIn, float * pEnvelopRMS, float * pEnvelop, int nSize); // nSize 는 채널당 sample 수

};

#endif // EnvelopDetector_H
