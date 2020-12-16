#ifndef UTILS_UTILS_H_
#define UTILS_UTILS_H_

#define EXPORTED __attribute__((__visibility__("default")))

#ifdef NEX_EXPORT_MORE
#define EXPORTED2 __attribute__((__visibility__("default")))
#else
#define EXPORTED2
#endif

#define NELEM(x) (sizeof(x) / sizeof(x[0]))

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef ABS
#define ABS(x) ((x)>0?(x):(-(x)))
#endif

#define xstr(s) str(s)
#define str(s) #s

#define CALL_MEMBER_FN(object,ptrToMember) ((object).*(ptrToMember))

#include <time.h>
#include <sys/time.h>
#include <stdio.h>

namespace {
long long getMonotonicClockInNanos()
{
	struct timespec ts;
	clock_gettime( CLOCK_MONOTONIC, &ts );
	return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}
}

namespace Nex_AR {
namespace Utils {
class Initializer {
protected:
	virtual ~Initializer() {}
public:
	virtual void initialize() = 0;
	virtual void deinitialize() = 0;
};

void registerInitializer(Initializer *initializer);
void initializeAll();
void deinitializeAll();

int GetSystemProperty(const char*,char*);
} // namespace Utils
} // namespace Nex_AR

#endif /* UTILS_UTILS_H_ */
