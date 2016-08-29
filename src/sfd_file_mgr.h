#ifndef _SFD_FILE_MGR_H
#define _SFD_FILE_MGR_H

#include "sfd_types.h"

struct file_info;

extern int sfd_file_mgr_init();
extern int sfd_file_mgr_fini();
extern struct file_info * sfd_file_mgr_get_file_info(const char *name);
extern int sfd_file_mgr_create_file(const char *name , uint size , uint record_size);
extern int sfd_file_mgr_delete_file(const char *name);

#ifdef DEBUG
extern int sfd_file_mgr_display();
#endif
#endif//_SFD_FILE_MGR_H

