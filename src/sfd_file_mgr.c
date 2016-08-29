/** glibc file head */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

/** sfd internal file head */
#include "rb_tree.h"
#include "sfd_file_mgr.h"
#include "sfd_file_info.h"

struct file_info_storage
{
    int f_size;
    int r_size;
    int r_num;
    char f_name[244];
};

/* --------------------- file info base -------------------------*/
struct file_info_mgr
{
    struct rb_tree file_info_base;
    int max_persistent_index;
    int fd;
    const char *mgr_dat_file_name;
#define BUF_LEN 256
    char buff[BUF_LEN];
};

const char *mgr_info_file_name = "./sfdb_data/mgr_info.dat";
static struct file_info_mgr fim;

static int __buf_write(struct file_info *fi)
{
    struct file_info_storage *fis;
    int fis_len;

    fis_len = sizeof(struct file_info_storage);

    fis = (struct file_info_storage*)fim.buff;
    fis->f_size = fi->size;
    fis->r_size = fi->record_size;
    fis->r_num = fi->curr_records;
    strncpy(fis->f_name , fi->name.data , fi->name.len);
    fis->f_name[fi->name.len] = '\0';

    return write(fim.fd , fim.buff , fis_len );
}

static int __buf_read(struct file_info *fi)
{
    struct file_info_storage *fis;
    int fis_len;
    ssize_t r_len;

    fis_len = sizeof(struct file_info_storage);
    while(1){
        r_len = read(fim.fd , fim.buff , fis_len);
        if (r_len == -1){
            if(errno == EINTR)
                continue;
            else 
                return -1;
        }

        if(r_len == 0)
            return -1;
    }

    fis = (struct file_info_storage*)fim.buff;

    fi->size = fis->f_size;
    fi->record_size = fis->r_size;
    fi->curr_records = fis->r_num;
    sfd_str_assign(&fi->name , fis->f_name);

    return 0;
}

static inline int __file_name_cmp(void *l , void *r)
{
    const char *lname;
    const char *rname;

    lname = (const char *)l;
    rname = (const char *)r;

    return strcmp(lname , rname);
}

static inline int __index_rebuild(void *k , void *v)
{
    struct file_info *fi;

    fi = (struct file_info *)v;

    fi->persistent_index = fim.max_persistent_index++;

    return 0;
}

static inline int __file_info_for_each_write(void *k , void *v)
{
    struct file_info *fi;

    fi = (struct file_info *)v;
    __buf_write(fi);

    return 0;
}

static inline void __file_info_append(struct file_info *fi)
{
    lseek(fim.fd , 0 , SEEK_END);
    __buf_write(fi);
}
static inline void __file_info_load()
{
    struct file_info *fi;
    int retval;

    lseek(fim.fd , 0 , SEEK_SET);
    while(1){
        fi = (struct file_info*)malloc(sizeof(struct file_info));
        if(!fi)
            return;

        retval = __buf_read(fi);
        if (retval != 0)
            return;

        fi->persistent_index = fim.max_persistent_index++;

        rb_insert(&fim.file_info_base , fi->name.data , fi);
    }
}

static inline void __file_info_store()
{
    ftruncate(fim.fd, 0);
    lseek(fim.fd , 0 , SEEK_SET);
    rb_for_each(&fim.file_info_base ,__file_info_for_each_write);
}

/* ----------------------external functions --------------------------*/
int sfd_file_mgr_init()
{
    fim.mgr_dat_file_name = mgr_info_file_name;
    fim.fd = open(fim.mgr_dat_file_name , O_RDWR);
    if (!fim.fd){
        printf("open mgr info file failed!name[%s] , errno:%d\n",mgr_info_file_name , errno);
        return -1;
    }

    rb_tree_init(&fim.file_info_base , __file_name_cmp);

    fim.max_persistent_index = 1;
    __file_info_load();

    return 0;
}
int sfd_file_mgr_fini()
{
    __file_info_store();
    close(fim.fd);

    return 0;
}
int sfd_file_mgr_create_file(const char *name , uint size , uint record_size)
{
    char *k_name;
    int retval;
    struct file_info *fi;

    retval = 0;

    fi = (struct file_info *)malloc(sizeof(struct file_info));
    if (!fi)
        return -1;

    FILE_INFO_INIT(fi);

    sfd_str_assign(&fi->name , name);
    fi->size = size;
    fi->record_size = record_size;
    fi->persistent_index = fim.max_persistent_index++;

    k_name = fi->name.data; 

    __file_info_append(fi);

    return rb_insert(&fim.file_info_base , k_name , fi);
}

int sfd_file_mgr_delete_file(const char *name)
{
    char *d_k;
    struct file_info *d_v;
    int retval;

    d_k = NULL;
    d_v = NULL;
    retval = rb_delete(&fim.file_info_base , (void *)name , (void*)&d_k , (void*)&d_v);

    if (retval != 0)
        return -1;

    if(d_v){
        sfd_str_dealloc(&d_v->name); 
        free(d_v);
        d_v = NULL;
    }

    fim.max_persistent_index = 1;

    rb_for_each(&fim.file_info_base , __index_rebuild);

    __file_info_store();

    return 0;
}


struct file_info * sfd_file_mgr_get_file_info(const char *name)
{
    return rb_find(&fim.file_info_base , (void *)name);;
}


#ifdef DEBUG
int p_file_info(void *k , void *v)
{
    struct file_info *fi;
    char *n;

    n = (char *)k;
    fi = (struct file_info *)v;

    printf("<%s , <n:%s , s:%d , cr:%d , rs:%d , pi:%d>>\n" , n , fi->name.data , fi->size , fi->curr_records , fi->record_size,fi->persistent_index);

    return 0;
}
int sfd_file_mgr_display()
{
    rb_for_each(&fim.file_info_base , p_file_info);
    return 0;
}
#endif

