#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "sfd_buffer.h"
#include "sfd_file_info.h"
#include "sfd_file_mgr.h"

struct file_cache_t
{
#define CACHE_FILES_NUM 10
    struct file_info lru_head;
    struct file_info *cache_files[CACHE_FILES_NUM]; 
    uint cache_files_size;
    uint curr_cache_files_size;
};


static struct file_cache_t file_cache;
static long sfd_page_size;

static inline void cache_init()
{
    LRU_LIST_INIT(file_cache.lru_head);
    bzero(file_cache.cache_files, sizeof(struct file_info*) * CACHE_FILES_NUM); 
    file_cache.cache_files_size = CACHE_FILES_NUM;
    file_cache.curr_cache_files_size = 0;
}

static inline int cache_array_full()
{
    return (file_cache.cache_files_size == file_cache.curr_cache_files_size);
}
static inline void cache_array_insert(struct file_info *fi , int index)
{
    if ( index < 0){
        index = file_cache.curr_cache_files_size;
    }
    fi->cached_flag = 1;
    fi->cache_index = index;
    file_cache.cache_files[index] = fi;
    file_cache.curr_cache_files_size++;
}

static inline void cache_array_delete(int index)
{
    file_cache.cache_files[index] = NULL;
    file_cache.curr_cache_files_size--;
}
static inline struct file_info * cache_array_find(const char *name)
{
  struct file_info *pi;
    uint i;

    for (i = 0; i < file_cache.cache_files_size; ++i){
        pi = file_cache.cache_files[i];
        if (pi && strcmp(pi->name.data , name) == 0)
            return pi;
    }
   
    return NULL;
}

static inline int sfd_open_file(struct file_info *fi)
{
    int fd;

    fd = open(fi->name.data , O_CREAT | O_RDWR , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1){
        printf("open file [%s] failed! errno [%d] \n" , fi->name.data ,errno);
        return -1;
    }

    fi->fd = fd;

    ftruncate(fd , fi->size);

    return 0;
}

static inline void buf_hit_stat(
        struct page_num_set_t *bpns , 
        struct page_num_set_t *rpns , 
        struct page_num_set_t *hit , 
        struct page_num_set_t *nohit1 , 
        struct page_num_set_t *nohit2)
{
    int page_num = 0;
    int min_hit , min_nohit1 , min_nohit2;
    int nohit1_flag;

    min_hit = 1;
    min_nohit1 = 1;
    min_nohit2 = 1;
    nohit1_flag = 1;
    for (page_num = rpns->min; page_num <= rpns->max; ++page_num){
        if( page_num >= bpns->min && page_num <= bpns->max ){
            if (min_hit){
                hit->min = page_num;
                hit->max = page_num;
                min_hit = 0;
            } else{
                hit->max = page_num;
            }
            nohit1_flag = 0;
        } else {
            if(nohit1_flag){
                if(min_nohit1){
                    nohit1->min = page_num;
                    nohit1->max = page_num;
                    min_nohit1 = 0;
                }else{
                    nohit1->max = page_num;
                }
            }else{
                if(min_nohit2){
                    nohit2->min = page_num;
                    nohit2->max = page_num;
                    min_nohit2 = 0;
                }else{
                    nohit2->max = page_num;
                }

            }
        }
    }
}
static int sfd_mmap(struct file_info *fi , uint offset)
{
    uint length;

    length = sfd_page_size * BUFFER_PAGE_NUM;
    fi->buffer = (char *)mmap(NULL , length , PROT_READ | PROT_WRITE , MAP_SHARED , fi->fd , offset); 
    if (MAP_FAILED == fi->buffer){
        printf("mmap failed pathname[%s] , fd[%d] , len[%d] , offset[%d].errno:%d\n", fi->name.data ,fi->fd , length , offset , errno);
        return -1;
    }

    fi->bpns.min = offset / sfd_page_size;
    fi->bpns.max = fi->bpns.min + BUFFER_PAGE_NUM;

    return 0;
}

static int buf_hit_deal(struct file_info *fi , 
        struct file_access_info *fai , 
        struct page_num_set_t *hpns , 
        void (*data_process)(void *dst_addr , void *src_addr , uint size),
        struct file_access_info *fai_l , 
        struct file_access_info *fai_r)
{
    uint original_pos;
    uint original_size;
    uint data_size;
    void *original_data_addr , *data_addr;
    long original_page_num_min , original_page_num_max;
    char *file_addr;
    char *pos_addr;

    original_page_num_min = fai->pos / sfd_page_size;
    original_page_num_max = (fai->pos + fai->size) / sfd_page_size + 1;

    original_pos = fai->pos;
    original_size = fai->size;
    original_data_addr = fai->data_addr;

    file_addr = fi->buffer - fi->bpns.min * sfd_page_size;

    if (original_page_num_min >= hpns->min){
        pos_addr = file_addr + original_pos;
        data_addr = original_data_addr;
    }else{
        pos_addr = file_addr + hpns->min * sfd_page_size; 
        fai_l->pos = original_pos;
        fai_l->size = pos_addr - (file_addr + fai_l->pos);
        fai_l->data_addr = original_data_addr; 
        data_addr = original_data_addr + fai_l->size;
    }

    if (original_page_num_max <= hpns->max){
        data_size = (file_addr + original_pos + original_size) - pos_addr;
    }else {
        data_size = (file_addr + hpns->max * sfd_page_size) - pos_addr;  
        fai_r->pos = hpns->max * sfd_page_size;
        fai_r->size = original_pos + original_size - fai_r->pos;
        fai_r->data_addr = original_data_addr + (hpns->max * sfd_page_size - original_pos);

    }

    data_process(pos_addr , data_addr , data_size);

    return 0;
}

static int buf_miss_deal( struct file_info *fi , 
        struct file_access_info *fai , 
        struct page_num_set_t *rpns_ptr , 
        void (*data_process)(void *dst_addr , void *src_addr , uint size))
{

    struct page_num_set_t rpns;
    void *data_addr;
    uint pos;
    uint size;

    rpns.min = rpns_ptr->min;
    rpns.max = rpns_ptr->max;

    data_addr = fai->data_addr;
    pos = fai->pos;
    size = fai->size;

    for(;;){
        char *file_addr;
        char *pos_addr;
        uint data_size;
        int retval;

        retval = sfd_mmap(fi , rpns.min * sfd_page_size );
        if (retval != 0)
            return retval;

        file_addr = fi->buffer - fi->bpns.min * sfd_page_size;
        pos_addr = file_addr + pos;

        if (rpns.max <= fi->bpns.max){
            data_size = size;
            data_process(pos_addr , data_addr, data_size);
            break;
        }else{
            data_size =  fi->bpns.max * sfd_page_size - pos;
            data_process(pos_addr , data_addr, data_size);
            size -= data_size; 
            rpns.min = fi->bpns.max;
            pos += data_size;
            data_addr += data_size;
        }

    }

    return 0;
}
static int data_io(struct file_info *fi , 
        struct file_access_info *fai , 
        void (*data_process)(void *dst_addr , void *src_addr , uint size))
{
    struct page_num_set_t bpns , rpns;

    bpns.min = fi->bpns.min;
    bpns.max = fi->bpns.max; 

    rpns.min = fai->pos / sfd_page_size;
    rpns.max = (fai->pos + fai->size) / sfd_page_size + 1;


    if (rpns.max < bpns.min || rpns.min > bpns.max){
        return buf_miss_deal(fi , fai , &rpns , data_process);
    }else{
        struct page_num_set_t hit , nohit1 , nohit2;
        struct file_access_info fai_l , fai_r;
        hit.min = hit.max = -1;
        nohit1.min = nohit1.max = -1;
        nohit2.min = nohit2.max = -1;
        buf_hit_stat(&bpns , &rpns , &hit , &nohit1 , &nohit2);

        //proecss hit
        if (hit.min != -1 && hit.max != -1){
            buf_hit_deal(fi , fai , &hit , data_process , &fai_l , &fai_r);  
        }
        //process nohit1
        if (nohit1.min != -1 && nohit1.max != -1){
            buf_miss_deal(fi , &fai_l , &nohit1 , data_process);
        }
        //process nohit2
        if (nohit2.min != -1 && nohit2.max != -1){
            buf_miss_deal(fi , &fai_r , &nohit2 , data_process);
        }
    }

    return 0;
}

static void data_write(void *dst_addr , void *src_addr , uint size)
{
    memcpy(dst_addr , src_addr , size);
}

static void data_zero(void *dst_addr , void *src_addr , uint size)
{
    memset(dst_addr , size , 0);
}

static void data_read(void *dst_addr , void *src_addr , uint size)
{
    memcpy(src_addr , dst_addr , size);
}


int sfd_buf_init()
{
    cache_init();

    sfd_page_size = sysconf( _SC_PAGESIZE );

    return 0;
}
int sfd_buf_fini()
{
    return 0;
}

struct file_info * sfd_get_file_info(const char *name)
{
    struct file_info *fi;
    int index;
    int retval;

    fi = cache_array_find(name); 
    if (fi)
        return fi;

    fi = sfd_file_mgr_get_file_info(name);
    if(!fi)
        return NULL;

    do{
        retval = sfd_open_file(fi);
        if (retval != 0)
            break;

        retval = sfd_mmap(fi , 0);
        if (retval != 0)
            break;
    }while(0);

    if (cache_array_full()){
        struct file_info *old_fi;
        old_fi = lru_node_first(&file_cache.lru_head);
        index = old_fi->cache_index;
        lru_node_del(old_fi);
    }else{
        index = -1;
    }

    lru_node_add_tail(&file_cache.lru_head , fi);
    cache_array_insert(fi , index);

    return fi;
}

int sfd_add(struct file_info *fi , struct file_access_info *fai)
{
    return data_io(fi , fai , data_write);
}
 
int sfd_modify(struct file_info *fi , struct file_access_info *fai)
{
    return data_io(fi , fai , data_write);
}
 
int sfd_del(struct file_info *fi , struct file_access_info *fai)
{
    return data_io(fi , fai , data_zero);
}

int sfd_read(struct file_info *fi , struct file_access_info *fai)
{
    return data_io(fi , fai , data_read);
}

