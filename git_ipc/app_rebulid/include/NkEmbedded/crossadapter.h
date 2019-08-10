
#include <NkUtils/types.h>

#if !defined(NK_EMB_CROSS_ADAPTER_H_)
#define NK_EMB_CROSS_ADAPTER_H_
NK_CPP_EXTERN_BEGIN

#if defined(_WIN32)

/**
 * Encapsulation "gmtime_s".
 */
#define GMTIME(__time_utc, __tm) \
	do {\
		time_t const __utc = __time_utc;\
		gmtime_s(&(__tm), &__utc);\
	} while (0)

#else
/**
 * Headers
 */
#include <unistd.h>

/**
 * Encapsulation "gmtime_r".
 */
#define GMTIME(__time_utc, __tm) \
	do {\
		time_t const __utc = __time_utc;\
		gmtime_r(&__utc, &(__tm));\
	} while (0)

#endif


NK_CPP_EXTERN_END
#endif /* NK_EMB_CROSS_ADAPTER_H_ */
