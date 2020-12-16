#ifndef _MEMORY_BUFFER_H_INCLUDED_
#define _MEMORY_BUFFER_H_INCLUDED_

#include "NexSAL_Internal.h"
#include "nexSALClasses/nexSALClasses.h"

// max is 4gb
class MemoryBuffer
{
public:
	/** MemoryBuffer
	 * default constructor
	 * malloc style buffer construction
	 */
	MemoryBuffer(unsigned int size) : uSize(size), uTotalRead(0), uTotalWritten(0) { pBuffer = malloc(size); pRead = pBuffer; pWrite = pBuffer; }

	/** MemoryBuffer
	 * constructor
	 * calloc style buffer construction
	 */
	MemoryBuffer(unsigned int num, unsigned int size) : uSize(num*size), uTotalRead(0), uTotalWritten(0) { pBuffer = calloc(num, size); pRead = pBuffer; pWrite = pBuffer; }
	virtual ~MemoryBuffer() { if (NULL != pBuffer) free(pBuffer); }

	virtual void PrintInternals()
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "MemoryBuffer: uSize(%u) pBuffer(%p) pRead(%p) pWrite(%p) uTotalRead(%llu) uTotalWritten(%llu)\n", uSize, pBuffer, pRead, pWrite, uTotalRead, uTotalWritten );
	}

	/** Reset
	 * resets the buffer to state after construction
	 */
	virtual void Reset() { pRead = pBuffer; pWrite = pBuffer; uTotalRead = 0; uTotalWritten = 0; }

	/** Size
	 * returns number of bytes written into this buffer
	 */
	virtual unsigned int Size() { return (char*)pWrite - (char*)pRead; }

	/** Capacity
	 * returns the max number of bytes that may fill this buffer
	 */
	virtual unsigned int Capacity() { return uSize; }

	/** TotalRead
	 * returns the total number of bytes read from this buffer since last reset (or initialization)
	 */
	virtual unsigned long long TotalRead() { return uTotalRead; }

	/** TotalWritten
	 * returns the total number of bytes written to this buffer since last reset (or initialization)
	 */
	virtual unsigned long long TotalWritten() { return uTotalWritten; }

	/** Read
	 * read _bytesToRead_ bytes from this buffer.
	 * this function is a no-op if _outputBuffer_ is pRead (as returned by GetReadBuffer)
	 * returns number of bytes read (which could be less than _bytesToRead_ but greater than 0)
	 */
	virtual unsigned int Read(void *outputBuffer, unsigned int bytesToRead)
	{
		if (NULL == outputBuffer || 0 == bytesToRead)
			return 0;

		if ((char*)pRead + bytesToRead > pWrite)
			bytesToRead = (char*)pWrite - (char*)pRead;
		if (0 >= bytesToRead)
			return 0;

		if (pRead != outputBuffer)
			memcpy(outputBuffer, pRead, bytesToRead);

		pRead = (char*)pRead + bytesToRead;
		uTotalRead += bytesToRead;
		return bytesToRead;
	}

	/** Write
	 * write _bytesToWrite_ bytes to this buffer
	 * this function is a no-op if _inputBuffer_ is pWrite (as returned by GetWriteBuffer)
	 * returns number of bytes written (either _bytesToWrite_ or 0)
	 */
	virtual unsigned int Write(const void *inputBuffer, unsigned int bytesToWrite)
	{
		if (NULL == inputBuffer || 0 == bytesToWrite || (char*)pWrite + bytesToWrite > (char*)pBuffer + uSize)
			return 0;

		if (pWrite != inputBuffer)
			memcpy(pWrite, inputBuffer, bytesToWrite);

		pWrite = (char*)pWrite + bytesToWrite;
		uTotalWritten += bytesToWrite;
		return bytesToWrite;
	}

	/** GetReadBuffer
	 * get a buffer suitable for reading _bytesToRead_ bytes.
	 * the memory pointed by the returned buffer is not count as read until Read is called with _inputBuffer_ set to the buffer returned by this function.
	 * returns NULL if there are less than _bytesToRead_ bytes available for reading
	 */
	virtual void * GetReadBuffer(unsigned int bytesToRead) { return ((char*)pRead + bytesToRead <= pWrite) ? pRead : NULL; }

	/** GetWriteBuffer
	 * get a buffer suitable for writing _bytesToWrite_ bytes.
	 * the memory pointed by the returned buffer is not count as written until Write is called with _outputBuffer_ set to the buffer returned by this function.
	 * returns NULL if there are less than _bytesToWrite_ bytes available for writing
	 */
	virtual void * GetWriteBuffer(unsigned int bytesToWrite) { return ((char*)pWrite + bytesToWrite <= (char*)pBuffer + uSize) ? pWrite : NULL; }
protected:
	unsigned int 		uSize;
	void*				pBuffer;
	void*				pRead;
	void*				pWrite;
	unsigned long long	uTotalRead;
	unsigned long long	uTotalWritten;
};

class CircularMemoryBuffer : public MemoryBuffer
{
public:
	CircularMemoryBuffer(unsigned int size) : MemoryBuffer(size) { pEnd = (char*)pBuffer + size; pReadableEnd = pWrite; pMutex = new CSALMutex(); }
	CircularMemoryBuffer(unsigned int num, unsigned int size, unsigned int overflowNum=0) : MemoryBuffer(num+overflowNum, size) { pEnd = (char*)pBuffer + num*size; pReadableEnd = pWrite;; pMutex = new CSALMutex(); }
	virtual ~CircularMemoryBuffer() { if (NULL != pMutex) delete pMutex; pMutex = NULL; }

	virtual void PrintInternals()
	{
		MemoryBuffer::PrintInternals();
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer: pReadableEnd(%p) pEnd(%p)\n", pReadableEnd, pEnd );
	}

	virtual void Reset()
	{
		{
			CSALMutex::Auto autoLock(pMutex);
			MemoryBuffer::Reset();
			pReadableEnd = pWrite;
		}
	}

	virtual unsigned int Size()
	{
		unsigned int ret = 0;
		if ( pReadableEnd != pWrite )
		{
			ret += ((char*)pReadableEnd - (char*)pRead);
			ret += ((char*)pWrite - (char*)pBuffer);
		}
		else
		{
			ret += ((char*)pWrite - (char*)pRead);
		}
		return ret;
	}

	virtual unsigned int Capacity() { return (char*)pEnd - (char*)pBuffer; }

	virtual unsigned int Read(void *outputBuffer, unsigned int bytesToRead)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer[%d]:Read+ bytesToRead(%u) uTotalRead(%llu)\n", __LINE__, bytesToRead, uTotalRead);
		if (NULL == outputBuffer || 0 == bytesToRead)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer[%d]:Read- bytesRead(%u)\n", __LINE__, 0);
			return 0;
		}

		void* pCachedRead = NULL;
		void* pCachedWrite = NULL;
		void* pCachedReadableEnd = NULL;
		unsigned int cachedSize = 0;

		{
			CSALMutex::Auto autoLock(pMutex);

			pCachedRead = pRead;
			pCachedWrite = pWrite;
			pCachedReadableEnd = pReadableEnd;
			cachedSize = Size();
		}

		unsigned int ret = 0;
		int tempRead;
		void * pBoundary;

		while (0 < bytesToRead && (pCachedReadableEnd != pCachedWrite || pCachedRead != pCachedWrite))
		{
			pBoundary = (pCachedReadableEnd != pCachedWrite) ? pCachedReadableEnd : pCachedWrite;

			tempRead = ((char*)pCachedRead + bytesToRead > pBoundary) ? (char*)pBoundary - (char*)pCachedRead : bytesToRead;
			if (0 >= tempRead && pBoundary == pCachedWrite)
				break;

			if (0 < tempRead && pCachedRead != (char*)outputBuffer + ret)
				memcpy((char*)outputBuffer+ret, pCachedRead, tempRead);
			bytesToRead -= tempRead;
			ret += tempRead;
			pCachedRead = (char*)pCachedRead + tempRead;
			if (pCachedRead == pBoundary && pCachedWrite < pBoundary)
			{
				pCachedRead = pBuffer;
				pCachedReadableEnd = pCachedWrite;
			}
		}

		{
			CSALMutex::Auto autoLock(pMutex);

			pRead = pCachedRead;
			pReadableEnd = (pCachedReadableEnd < pWrite) ? pWrite : pCachedReadableEnd;
			uTotalRead += ret;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer[%d]:Read- bytesRead(%u) uTotalRead(%llu)\n", __LINE__, ret, uTotalRead);
		return ret;
	}

	virtual unsigned int Write(const void *inputBuffer, unsigned int bytesToWrite)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer[%d]:Write+ bytesToWrite(%u) uTotalWritten(%llu)\n", __LINE__, bytesToWrite, uTotalWritten);
		void* pCachedRead = NULL;
		void* pCachedWrite = NULL;
		void* pCachedReadableEnd = NULL;
		unsigned int cachedSize = 0;

		{
			CSALMutex::Auto autoLock(pMutex);

			pCachedRead = pRead;
			pCachedWrite = pWrite;
			pCachedReadableEnd = pReadableEnd;
			cachedSize = Size();
		}

		// first check if there is sufficient memory
		if (NULL == inputBuffer || 0 == bytesToWrite || Capacity() - cachedSize < bytesToWrite)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer[%d]:Write- bytesWritten(%u) uTotalWritten(%llu)\n", __LINE__, 0, uTotalWritten);
			return 0;
		}

		// if pWrite < pRead, then there's always enough room, so only need to check against pEnd
		unsigned int written = ((unsigned int)((char*)pEnd - (char*)pCachedWrite) > bytesToWrite) ? bytesToWrite : (char*)pEnd - (char*)pCachedWrite;

		if (pCachedWrite != inputBuffer && 0 != written)
			memcpy(pCachedWrite, inputBuffer, written);

		if (pCachedReadableEnd == pCachedWrite)
			pCachedReadableEnd = (char*)pCachedReadableEnd + written;
		bytesToWrite -= written;
		if (0 == bytesToWrite)
		{
			pCachedWrite = (char*)pCachedWrite + written;
		}
		else
		{
			pCachedWrite = pBuffer;
			memcpy(pCachedWrite, (char*)inputBuffer+written, bytesToWrite);
			written += bytesToWrite;
			pCachedWrite = (char*)pCachedWrite + bytesToWrite;
		}

		{
			CSALMutex::Auto autoLock(pMutex);

			pWrite = pCachedWrite;
			pReadableEnd = (pReadableEnd < pCachedWrite) ? pCachedWrite : pReadableEnd;
			uTotalWritten += written;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer[%d]:Write- bytesWritten(%u) uTotalWritten(%llu)\n", __LINE__, written, uTotalWritten);
		return written;
	}

	virtual void * GetReadBuffer(unsigned int bytesToRead)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer[%d]:GetReadBuffer+ bytesToRead(%u) uTotalRead(%llu)\n", __LINE__, bytesToRead, uTotalRead);
		void* pCachedRead = NULL;
		void* pCachedWrite = NULL;
		void* pCachedReadableEnd = NULL;
		unsigned int cachedSize = 0;

		{
			CSALMutex::Auto autoLock(pMutex);

			pCachedRead = pRead;
			pCachedWrite = pWrite;
			pCachedReadableEnd = pReadableEnd;
			cachedSize = Size();
		}

		if (cachedSize < bytesToRead)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer[%d]:GetReadBuffer- ret(%p) bytesToRead(%u) uTotalRead(%llu)\n", __LINE__, NULL, bytesToRead, uTotalRead);
			return NULL;
		}

		if (pCachedRead < pCachedWrite || (unsigned int)((char*)pCachedReadableEnd - (char*)pCachedRead) >= bytesToRead)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer[%d]:GetReadBuffer- ret(%p) bytesToRead(%u) uTotalRead(%llu)\n", __LINE__, pCachedRead, bytesToRead, uTotalRead);
			return pCachedRead;
		}

		if ((unsigned int)((char*)pBuffer + uSize - (char*)pCachedRead) >= bytesToRead)
		{
			memcpy(pCachedReadableEnd, pBuffer, bytesToRead - ((char*)pCachedReadableEnd - (char*)pCachedRead));
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer[%d]:GetReadBuffer- ret(%p) bytesToRead(%u) uTotalRead(%llu)\n", __LINE__, pCachedRead, bytesToRead, uTotalRead);
			return pCachedRead;
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer[%d]:GetReadBuffer- ret(%p) bytesToRead(%u) uTotalRead(%llu)\n", __LINE__, NULL, bytesToRead, uTotalRead);
		return NULL;
	}

	virtual void * GetWriteBuffer(unsigned int bytesToWrite)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer[%d]:GetWriteBuffer+ bytesToWrite(%u) uTotalWritten(%llu)\n", __LINE__, bytesToWrite, uTotalWritten);
		void* pCachedRead = NULL;
		void* pCachedWrite = NULL;
		void* pCachedReadableEnd = NULL;
		void* ret = NULL;

		{
			CSALMutex::Auto autoLock(pMutex);

			pCachedRead = pRead;
			pCachedWrite = pWrite;
			pCachedReadableEnd = pReadableEnd;
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 6, "CircularMemoryBuffer[%d]:GetWriteBuffer: pCachedRead(%p) pCachedWrite(%p) pCachedReadableEnd(%p) pEnd(%p) pBuffer(%p)\n"
			, __LINE__, pCachedRead, pCachedWrite, pCachedReadableEnd, pEnd, pBuffer);
		if (pCachedReadableEnd != pCachedWrite)
		{
			ret = ((char*)pCachedWrite + bytesToWrite <= pCachedRead) ? pCachedWrite : NULL;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer[%d]:GetWriteBuffer- ret(%p) bytesToWrite(%u) uTotalWritten(%llu)\n", __LINE__, ret, bytesToWrite, uTotalWritten);
			return ret;
		}
		else if ((char*)pCachedWrite + bytesToWrite <= pEnd)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer[%d]:GetWriteBuffer- ret(%p) bytesToWrite(%u) uTotalWritten(%llu)\n", __LINE__, pCachedWrite, bytesToWrite, uTotalWritten);
			return pCachedWrite;
		}
		else if ((char*)pBuffer + bytesToWrite <= pCachedRead)
		{
			pCachedWrite = pBuffer;
			{
				CSALMutex::Auto autoLock(pMutex);
				pWrite = pCachedWrite;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer[%d]:GetWriteBuffer- ret(%p) bytesToWrite(%u) uTotalWritten(%llu)\n", __LINE__, pCachedWrite, bytesToWrite, uTotalWritten);
			return pCachedWrite;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "CircularMemoryBuffer[%d]:GetWriteBuffer- ret(%p) bytesToWrite(%u) uTotalWritten(%llu)\n", __LINE__, NULL, bytesToWrite, uTotalWritten);
		return NULL;
	}
protected:
	void*		pReadableEnd;
	void*		pEnd;
	CSALMutex	*pMutex;
};

#endif
