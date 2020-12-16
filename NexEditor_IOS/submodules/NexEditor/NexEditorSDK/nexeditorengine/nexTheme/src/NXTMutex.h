//
//  NXTMutex.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 8/21/14.
//
//

#ifndef NexVideoEditor_NXTMutex_h
#define NexVideoEditor_NXTMutex_h
#include <pthread.h>

class NXTMutex {
    
public:
    NXTMutex( const NXTMutex& ) = delete;
    NXTMutex& operator=( const NXTMutex& ) = delete;
    
    NXTMutex() {
        pthread_mutex_init(&lock_,NULL);
    }
    
    ~NXTMutex(){
        pthread_mutex_destroy(&lock_);
    }
    
    void lock() {
        pthread_mutex_lock(&lock_);
    }
    
    void unlock() {
        pthread_mutex_unlock(&lock_);
    }
    
private:
    pthread_mutex_t lock_;
};

template <typename T>
class NXTAutoLock {
    
public:
    NXTAutoLock( const NXTAutoLock& ) = delete;
    NXTAutoLock& operator=( const NXTAutoLock& ) = delete;
    
    NXTAutoLock(T& lock) {
        lock_ptr_ = &lock;
        if( lock_ptr_ ) {
            lock_ptr_->lock();
        }
    }
    
    void retain_lock() {
        lock_ptr_ = nullptr;
    }
    
    ~NXTAutoLock() {
        if( lock_ptr_ ) {
            lock_ptr_->unlock();
        }
    }
    
private:
    T* lock_ptr_;
    
};

typedef NXTAutoLock<NXTMutex> NXTMutexAutoLock;

#endif
