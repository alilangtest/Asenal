#include "bada_util.h"

extern void qf_free(void *ptr)
{
    if (NULL == ptr)
        return;
    free(ptr);
}

