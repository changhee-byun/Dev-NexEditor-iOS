/******************************************************************************
 * File Name   : IPodLibrarySource.hpp
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#ifndef IPodLibrarySource_hpp
#define IPodLibrarySource_hpp

#include <NexTypeDef.h>
#include <AudioToolbox/AudioToolbox.h>
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////////////////////////

// default output sample rate
#define NFAF_DEF_SAMPLERATE                                  44100
#define NFAF_DEF_CHANNEL                                     2


/////////////////////////////////////////////////////////////////////////////////
// Types
/////////////////////////////////////////////////////////////////////////////////
// sound sample type
typedef unsigned short SndSample;


class IPodLibraryReader
{
public:
    IPodLibraryReader();
    ~IPodLibraryReader();
    
    bool openURL(char* fileURL);
    void close();
    bool read(unsigned int* samples, SndSample* pcm);
    bool seek(int* samplePosition);
    bool ftell(int *position);
    
    
    bool dumpOpen();
    bool dumpClose();
    bool dump(int samples, uint8_t* pcm, int length);
    int getNumOfChannels();
    double getSampleRate();
    long long getTotalSize();
    float getDuration();
    int getBitrate();
    unsigned int getTotalFrameCount();
    
    void resetBufferSize();
private:
    void updateWAVHeaderToFile();
    
public:
    unsigned int numSamples;
    int fileChannels;
    int outputChannels;
    double fileSampleRate;
    double outputSampleRate;
    double srRatio;
    
private:
    ExtAudioFileRef audioFile;
    AudioBufferList *ablBuffer;
    char* filePath;
//    SndSample *sample[8];
    SndSample *sampleL;
    SndSample *sampleR;
    

};

#endif /* IPodLibrarySource_hpp */
