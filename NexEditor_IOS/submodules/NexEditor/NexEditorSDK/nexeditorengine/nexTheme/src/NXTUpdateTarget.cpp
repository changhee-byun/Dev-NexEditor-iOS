//
//  NXTUpdateTarget.cpp
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/11/14.
//
//

#include "NXTUpdateTarget.h"

void NXTUpdateTarget::parseParams( NXTNode* pNode, int what, NXTNode* paramsBaseNode, std::string paramString ) {
    notifyNode = pNode;
    notifyNodeWhat = what;
    paramsBaseNode->animFloatVectorFromString( paramString, params );
}

// getParams?

template<int N>
void NXTUpdateTarget::publishUpdate(float (&srcVector)[N]) {
    
    if( targetVector==nullptr )
        return;
    
    for( int i=0; i<numElements; i++ ) {
        targetVector[i] = ((srcIndex+i) < N?srcVector[srcIndex+i]:0) * factor + bias;
    }
    
    if( notifyNode ) {
        notifyNode->updateNotify(notifyNodeWhat);
    }
}

void NXTUpdateTarget::publishUpdate(const NXTVector& srcVector) {
    if( targetVector==nullptr )
        return;
    
    for( int i=0; i<numElements; i++ ) {
        targetVector[i] = ((srcIndex+i) < srcVector.size()?srcVector[srcIndex+i]:0) * factor + bias;
    }
    
    if( notifyNode ) {
        notifyNode->updateNotify(notifyNodeWhat);
    }
}