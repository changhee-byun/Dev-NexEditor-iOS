#include "nexQueue.h"

template<> int Queue<int>::dequeue()
{
	int ret;
	sem_wait(&sem);
	if (nFilled)
	{
		nFilled--;
		ret = q[head];
		q[head++] = -1;
		head %= maxSize;
	}
	else
	{
		ret = -1;
	}
	sem_post(&sem);
	return ret;
}

template<> int Queue<int>::peek()
{
	int ret;
	sem_wait(&sem);
	if (nFilled)
	{
		ret = q[head];
	}
	else
	{
		ret = -1;
	}
	sem_post(&sem);
	return ret;
}

template<> unsigned int Queue<unsigned int>::dequeue()
{
	int ret;
	sem_wait(&sem);
	if (nFilled)
	{
		nFilled--;
		ret = q[head];
		q[head++] = -1;
		head %= maxSize;
	}
	else
	{
		ret = -1;
	}
	sem_post(&sem);
	return ret;
}

template<> unsigned int Queue<unsigned int>::peek()
{
	int ret;
	sem_wait(&sem);
	if (nFilled)
	{
		ret = q[head];
	}
	else
	{
		ret = -1;
	}
	sem_post(&sem);
	return ret;
}

