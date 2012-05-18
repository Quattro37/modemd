#include <stdlib.h>
#include <string.h>

#include "re.h"

/*------------------------------------------------------------------------*/

int regmatch_atoi(const char* src, const regmatch_t* re_subs)
{
    int res = 0;
    size_t len;
    char* buf;

    len = re_subs->rm_eo - re_subs->rm_so;

    if(!(buf = malloc(len + 1)))
        return(res);

    memcpy(buf, src + re_subs->rm_so, len);
    buf[len] = 0;

    res = atoi(buf);

    free(buf);

    return(res);
}

/*------------------------------------------------------------------------*/

char* regmatch_ncpy(char* dst, size_t n, const char* src, const regmatch_t* re_subs)
{
    size_t len;

    len = re_subs->rm_eo - re_subs->rm_so;
    len = (len > n ? n - 1 : len);

    memcpy(dst, src + re_subs->rm_so, len);
    dst[len] = 0;

    return(dst + len);
}

/*------------------------------------------------------------------------*/

char* regmatch_strdup(const char* src, const regmatch_t* re_subs)
{
    size_t len;
    char* res;

    len = re_subs->rm_eo - re_subs->rm_so;

    if(!(res = malloc(len + 1)))
        return(res);

    memcpy(res, src + re_subs->rm_so, len);
    res[len] = 0;

    return(res);
}
