#ifndef _SFD_MGR_H_
#define _SFD_MGR_H_

#include "sfd_types.h"

struct file_info;
struct file_access_info
{
    const char *name;
    uint pos;
    uint size;
    void *data_addr;
};

extern int sfd_buf_init();
extern int sfd_buf_fini();

extern struct file_info * sfd_get_file_info(const char *name);
extern int sfd_add(struct file_info *fi ,struct file_access_info *fai); 
extern int sfd_modify(struct file_info *fi ,struct file_access_info *fai); 
extern int sfd_del(struct file_info *fi ,struct file_access_info *fai);
extern int sfd_read(struct file_info *fi ,struct file_access_info *fai);


#endif//_SFD_MGR_H_

