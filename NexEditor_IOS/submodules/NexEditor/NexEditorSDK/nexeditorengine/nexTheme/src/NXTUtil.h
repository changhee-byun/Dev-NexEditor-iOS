//
//  NXTUtil.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/23/14.
//
//

#ifndef __NexVideoEditor__NXTUtil__
#define __NexVideoEditor__NXTUtil__

#include <iostream>

template<typename C, typename E>
bool contains( C container, E element ) {
    for( auto e: container ) {
        if( e==element ) return true;
    }
    return false;
};

#endif /* defined(__NexVideoEditor__NXTUtil__) */
