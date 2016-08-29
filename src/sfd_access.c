#include "sfd_access.h"
#include "sfd_file_mgr.h"
#include "sfd_buffer.h"

#define INIT_FAI(fai , n , p , s, da) \
{ \
    fai.name = n; \
    fai.pos = p; \
    fai.size = s; \
    fai.data_addr = da; \
}

int init_sfdb()
{
    sfd_file_mgr_init();

    sfd_buf_init();

    return 0;
}

int fini_sfdb()
{
    sfd_file_mgr_fini();
    return 0;
}

int create_data_file(const char *name , uint size , uint record_size)
{
    return sfd_file_mgr_create_file(name , size , record_size);
}

int delete_data_file(const char *name)
{
    return sfd_file_mgr_delete_file(name);
}

int add_record(const char *name , void *data_addr , uint pos , uint size)
{
    struct file_access_info fai;
    struct file_info *fi;

    fi = sfd_get_file_info(name);
    if (!fi)
        return -1;

    INIT_FAI(fai , name , pos , size , data_addr);

    return sfd_add(fi , &fai);
}

int modify_record(const char *name , void *data_addr , uint pos , uint size)
{
    struct file_access_info fai;
    struct file_info *fi;

    fi = sfd_get_file_info(name);
    if (!fi)
        return -1;

    INIT_FAI(fai , name , pos , size , data_addr);

    return sfd_modify(fi , &fai);

}

int del_record(const char *name , uint pos , uint size)
{
    struct file_access_info fai;
    struct file_info *fi;

    fi = sfd_get_file_info(name);
    if (!fi)
        return -1;

    INIT_FAI(fai , name , pos , size , 0);

    return sfd_del(fi , &fai);

}

int read_record(const char *name , void *data_addr , uint pos , uint size)
{
    struct file_access_info fai;
    struct file_info *fi;

    fi = sfd_get_file_info(name);
    if (!fi)
        return -1;

    INIT_FAI(fai , name , pos , size , data_addr);

    return sfd_read(fi , &fai);
}

