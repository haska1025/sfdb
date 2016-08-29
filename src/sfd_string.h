#ifndef _SFD_STRING_H_
#define _SFD_STRING_H_

#include <string.h>
#include <stdlib.h>
#include "sfd_types.h"

typedef struct sfd_str
{
    char *data;
    uint len;
}sfd_str_t;

static inline uint sfd_str_len(sfd_str_t *str)
{
    return str->len;
}

static inline sfd_str_t * sfd_str_create(char *s)
{
    sfd_str_t *pstr;
    pstr = (sfd_str_t *)malloc(sizeof(sfd_str_t));
    if(!pstr)
        return NULL;

    pstr->len = strlen(s);

    pstr->data = (char *)malloc(pstr->len + 1);
    if (!pstr->data){
        free(pstr);
        return NULL;
    }

    strncpy(pstr->data , s , pstr->len);
    pstr->data[pstr->len] = '\0';

    return pstr;
}

static inline void sfd_str_destroy(sfd_str_t *str)
{
    free(str->data);
    free(str);
}

static inline sfd_str_t * sfd_str_alloc(sfd_str_t *str , uint size)
{
    str->data = (char *)malloc(size);
    str->len = size;
    return str;
}

static inline void sfd_str_dealloc(sfd_str_t *str)
{
    free(str->data);
    str->data = 0;
    str->len = 0;
}

static inline sfd_str_t * sfd_str_dup(sfd_str_t *dst , sfd_str_t *src)
{
    strncpy(dst->data , src->data , src->len);
    dst->data[src->len -1] = '\0';
    dst->len = src->len;

    return dst;
}

static inline sfd_str_t * sfd_str_assign(sfd_str_t *dst , const char *name)
{
    uint name_len = strlen(name);
    sfd_str_alloc(dst , name_len + 1);
    
    strncpy(dst->data , name , name_len);
    dst->data[name_len] = '\0';
    dst->len = name_len + 1;

    return dst;
}

#endif//_SFD_STRING_H_

