#ifndef __NPE_ERR_CODES__
#define __NPE_ERR_CODES__
#include <assert.h>

#define NPE_GEM_RESPONSE_OK                  ((uint32_t) 0)
#define NPE_GEM_RESPONSE_TIMEOUT_OUT         ((uint32_t) 1)
#define NPE_GEM_RESPONSE_RETRIES_EXHAUSTED   ((uint32_t) 2)
#define NPE_GEM_RESPONSE_UNABLE_TO_WRITE     ((uint32_t) 3)
#define NPE_GEM_RESPONSE_SERIAL_NO_COMPORT   ((uint32_t) 4)
#define NPE_GEM_RESPONSE_SERIAL_OPEN_FAIL    ((uint32_t) 5)
#define NPE_GEM_RESPONSE_SERIAL_CONFIG_FAIL  ((uint32_t) 6)
#define NPE_GEM_RESPONSE_INVALID_PARAMETER   ((uint32_t) 7)


#endif