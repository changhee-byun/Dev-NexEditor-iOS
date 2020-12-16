//
//  NXTNodeSequence.cpp
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 8/7/14.
//
//

#include "NXTNodeSequence.h"

void NXTNodeSequence::renderBegin( IRenderContext& renderer ) {
    if( type_ == NXT_SeqType_Rand || type_ == NXT_SeqType_RandMix ) {
        NXT_SRandom(&cur_seed, seed1, seed2);
    } else if( type_ == NXT_SeqType_VRand || type_ == NXT_SeqType_VRandMix ) {
        int clip_index = renderer.getCurrentClipIndex();
        int clip_count = renderer.getTotalClipCount();
        NXT_SRandom(&cur_seed, seed1 + (clip_index * 0x4A8F0E1), seed2 + (clip_count * 0x1329) + (clip_index * 0x29BC8C4) );
    }
    repeats_done = 0;
    processUpdateTargets();
}

NXTNode::Action NXTNodeSequence::renderEnd( IRenderContext &renderer ) {
    ++repeats_done;
    if( repeats_done < count_ ) {
        processUpdateTargets();
        return Action::Repeat;
    } else {
        return Action::Next;
    }
}

void NXTNodeSequence::processUpdateTargets() {
    NXTVector v;
    int i, j;
    float t = 0;
    
    switch( type_ ) {
        case NXT_SeqType_Interp:
            t = (float)repeats_done / (float)(count_-1);
            break;
        case NXT_SeqType_Exp:
            t = (float)repeats_done / (float)(count_-1);
            t = t*t;
            break;
        case NXT_SeqType_Rand:
        case NXT_SeqType_RandMix:
            t = NXT_FRandom(&cur_seed);
            break;
        case NXT_SeqType_VRand:
        case NXT_SeqType_VRandMix:
            t = NXT_FRandom(&cur_seed);
            break;
    }
    
    v[0] = start_[0] + (end_[0]-start_[0])*t;
    if( type_==NXT_SeqType_RandMix ) t = NXT_FRandom(&cur_seed);
    v[1] = start_[1] + (end_[1]-start_[1])*t;
    if( type_==NXT_SeqType_RandMix ) t = NXT_FRandom(&cur_seed);
    v[2] = start_[2] + (end_[2]-start_[2])*t;
    if( type_==NXT_SeqType_RandMix ) t = NXT_FRandom(&cur_seed);
    v[3] = start_[3] + (end_[3]-start_[3])*t;
    
    for( auto target: updateTargets ) {
        for( int i=0; i<target.numElements; i++ ) {
            target.targetVector[i] = v[i + target.srcIndex] * target.factor + target.bias;
        }
        if( target.notifyNode ) {
            target.notifyNode->updateNotify(target.notifyNodeWhat);
        }
    }
    
}
