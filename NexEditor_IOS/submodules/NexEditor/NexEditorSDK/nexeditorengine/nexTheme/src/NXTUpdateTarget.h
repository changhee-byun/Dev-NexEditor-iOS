//
//  NXTUpdateTarget.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/11/14.
//
//

#ifndef __NexVideoEditor__NXTUpdateTarget__
#define __NexVideoEditor__NXTUpdateTarget__

#include <iostream>
#include "NXTNode.h"
#include "NXTVector.h"

class NXTNode;

// notifyNode is used when a SEQUENCE updates an element used in a FUNCTION parameter

/**
 
  The source node generates data in some way; for example, AnimatedValue,
  UserField, Sequence, Function, etc.
 
  The target node receives a copy of the generated data every time the generated
  data changes.
 
  Multiple target nodes can receive data from the same source node.
 
  The source node contians an array of NXTUpdateTarget objects which describe each of
  the target nodes that must be updated when the source node's data changes.
 
  The target node may IN TURN be a source node for another target.  Currently, the
  only nodes that do this are Function nodes.
 
 */

class NXTUpdateTarget {
    
public:
    float           *targetVector;
    int             numElements;
    int             srcIndex        = 0;
    float           factor          = 1;
    float           bias            = 0;
    int             numParams       = 0;
    float           params[4]       = {0};
    NXTNode         *notifyNode     = nullptr;
    int             notifyNodeWhat  = 0;

public:
    template<int N>
    NXTUpdateTarget(float (&vector)[N], int index, int len) {
        
        int maxLen = N-index;
        if( maxLen < 0 )
            maxLen = 0;
        
        numElements = len > maxLen ? maxLen : len;
        targetVector = (&vector) + (numElements<1?0:index);
        notifyNode = nullptr;
    }
    
    void setNotifyNode( NXTNode *notifyNode, int what ) {
        this->notifyNode = notifyNode;
        this->notifyNodeWhat = what;
    }
    
    void setSrcIndex( int n ) {
        srcIndex = n;
    }
    
    void parseParams( NXTNode* pNode, int what, NXTNode* paramsBaseNode, std::string paramString );
    // getParams?
    
    template<int N>
    void publishUpdate(float (&srcVector)[N]);
    void publishUpdate(const NXTVector& srcVector);
    
};

#endif /* defined(__NexVideoEditor__NXTUpdateTarget__) */
