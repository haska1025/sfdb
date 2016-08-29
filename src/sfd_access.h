#ifndef _SFD_ACCESS_H_
#define _SFD_ACCESS_H_

#include "sfd_types.h"


extern int init_sfdb();
extern int fini_sfdb();

extern int create_data_file(const char *name , uint size , uint record_size);
extern int delete_data_file(const char *name);

/**
 * @
 * @param[in] name, the file name 
 * @param[in] pos , the bytes relative to the begin of the file.
 * @param[in] data_addr , the data address need to store 
 * @param[in] size , the data size need to store
 * @return int , if call success ,return 0;other , return error code.
 */
extern int add_record(const char *name , void *data_addr , uint pos , uint size);
extern int modify_record(const char *name , void *data_addr , uint pos , uint size);
extern int del_record(const char *name , uint pos , uint size);
extern int read_record(const char *name , void *data_addr , uint pos , uint size);

#endif//_SFD_ACCESS_H_

