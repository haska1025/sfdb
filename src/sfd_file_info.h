#ifndef _SFD_FILE_INFO_H_
#define _SFD_FILE_INFO_H_

#include "sfd_string.h"
#include "sfd_types.h"

struct page_num_set_t
{
    long min;
    long max;
};

struct file_info
{
    struct file_info *prev;
    struct file_info *next;
    sfd_str_t  name;//file name
    int fd;//file description
    size_t size;//file length
    uint curr_records;//the count of current records
    uint record_size;//per-record size; 

#define BUFFER_PAGE_NUM 4 
    char *buffer;
    struct page_num_set_t bpns;

    int cached_flag;//0:not in cache , 1:in cache
    int cache_index;//the index in the cache array

    int persistent_index;
};

#define LRU_LIST_INIT(head) \
{ \
    (head).prev = &(head); \
    (head).next = &(head); \
}

#define FILE_INFO_INIT(fi) \
{ \
    fi->name.data = 0; \
    fi->name.len = 0; \
    fi->fd = -1; \
    fi->size = 0; \
    fi->curr_records = 0; \
    fi->record_size = 0; \
    fi->buffer = 0; \
    fi->bpns.min = -1; \
    fi->bpns.max = -1; \
    fi->cached_flag = 0; \
    fi->cache_index = -1; \
    fi->persistent_index; \
}

#define lru_node_first(head) (head)->next 

static inline void lru_node_insert(struct file_info *prev , struct file_info *next , struct file_info *new_node)
{
    prev->next = new_node;
    next->prev = new_node;
    new_node->prev = prev;
    new_node->next = next;
}

static inline void lru_node_add_tail(struct file_info *head , struct file_info *new_node)
{
    lru_node_insert(head->prev , head , new_node);
}

static inline void lru_node_add_head(struct file_info *head , struct file_info *new_node)
{
    lru_node_insert(head , head->next , new_node);
}

static inline void lru_node_del(struct file_info *del_node)
{
    struct file_info *prev , *next;

    prev = del_node->prev;
    next = del_node->next;

    prev->next = next;
    next->prev = prev;
}
#endif//_SFD_FILE_INFO_H_

