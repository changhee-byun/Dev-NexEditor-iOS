#ifndef _NEX_CALBODY_QUEUE_H_
#define	_NEX_CALBODY_QUEUE_H_

#include <stdlib.h>
#include <semaphore.h>
#include <linux/threads.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <android/log.h>


#define  LOG_TAG    "NEXEDITOR"
#include <android/log.h>

#ifndef LOGE
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif

#ifndef LOGI
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#endif

template<typename T> struct Queue
{
	unsigned int maxSize;
	volatile unsigned int nFilled;
	unsigned int head;
	sem_t sem;
	const char *name;
	T q[64];
	int init(const char* n = "Unnamed")
	{
		name = n;
		return sem_init(&sem, 0, 1);
	}
	int deinit()
	{
		return sem_destroy(&sem);
	}
	bool contains(T element)
	{
		sem_wait(&sem);
		bool ret = false;
		int j = head;
		for (int i = 0; i < nFilled; ++i)
		{
			if (q[j++] == element)
			{
				ret = true;
				break;
			}
			j %= maxSize;
		}
		sem_post(&sem);
		return ret;
	}
	bool empty()
	{
		sem_wait(&sem);
		LOGI("[Queue.h %d] \"%s\" f%u h%u",  __LINE__, name, nFilled, head);
		bool ret = 0 == nFilled;
		sem_post(&sem);
		return ret;
	}
	bool atmost(unsigned int n)
	{
		sem_wait(&sem);
		LOGI("[Queue.h %d] \"%s\" f%u h%u",  __LINE__, name, nFilled, head);
		bool ret = n >= nFilled;
		sem_post(&sem);
		return ret;
	}
	bool full()
	{
		sem_wait(&sem);
		LOGI("[Queue.h %d] \"%s\" f%u h%u",  __LINE__, name, nFilled, head);
		bool ret = maxSize == nFilled;
		sem_post(&sem);
		return ret;
	}
	int queue(T element)
	{
		int ret = 0;
		sem_wait(&sem);
		LOGI("[Queue.h %d] \"%s\" pre: f%u h%u",  __LINE__, name, nFilled, head);
		if (maxSize == nFilled)
		{
			ret = -1;
		}
		else
		{
			q[(head + nFilled) % maxSize] = element;
			nFilled++;
		}
		LOGI("[Queue.h %d] \"%s\" post: f%u/%u h%u",  __LINE__, name, nFilled, maxSize, head);
		sem_post(&sem);
		return ret;
	}
	T dequeue()
	{
		T ret;
		sem_wait(&sem);
		LOGI("[Queue.h %d] \"%s\" pre: f%u h%u",  __LINE__, name, nFilled, head);
		if (nFilled)
		{
			nFilled--;
			ret = q[head];
			q[head++] = static_cast<T> (0);
			head %= maxSize;
		}
		else
		{
			ret = static_cast<T> (0);
		}
		LOGI("[Queue.h %d] \"%s\" post: f%u/%u h%u",  __LINE__, name, nFilled, maxSize, head);
		sem_post(&sem);
		return ret;
	}
	T peek()
	{
		T ret;
		sem_wait(&sem);
		LOGI("[Queue.h %d] \"%s\" pre: f%u h%u",  __LINE__, name, nFilled, head);
		if (nFilled)
		{
			ret = q[head];
		}
		else
		{
			ret = static_cast<T> (0);
		}
		LOGI("[Queue.h %d] \"%s\" post: f%u h%u",  __LINE__, name, nFilled, head);
		sem_post(&sem);
		return ret;
	}
};

#endif	// _NEX_CALBODY_QUEUE_H_

