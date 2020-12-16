#pragma once

class Mutex
{   /* placeholder for code to create, use, and free a mutex */
};

class Lock
{
public:
	Lock(Mutex& m) : mutex(m) { /* placeholder code to acquire the mutex */ }
	~Lock() { /* placeholder code to release the mutex */ }
private:
	Mutex & mutex;
};

template<typename T>
class Singleton{

public:
	static T* getInstance(int id = 0){

		Lock lock(mutex);

		static Singleton<T> inst;

		return inst.data_[id];
	}
private:
	static T* data_[2];
	static Mutex mutex;
	explicit Singleton(){

		data_[0] = new T;
		data_[1] = new T;
	}

	~Singleton(){

		delete data_[0];
		delete data_[1];
	}
};

template<typename T>
T* Singleton<T>::data_[2] = {0, 0};

template<typename T>
Mutex Singleton<T>::mutex;