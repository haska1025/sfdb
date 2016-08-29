#include "sfd_buffer.h"
#include <stdio.h>
#include <string.h>

typedef struct stu
{
    int id;
    int sex;
    int age;
    char name[20];
}stu_t;

void show_stu(stu_t *s)
{
    printf("id[%d], sex[%d], age[%d], name[%s]\n", s->id , s->sex , s->age , s->name);
}

#define file_path "/root/test/sfdbase/test_db.dat"
int main(int argc , char *argv[])
{
    int retval;
    struct file_info *fi;
    struct file_access_info fai;
    stu_t s1 , s11;

    init_sfdb();
    retval = sfd_create_file(file_path , 100 * 1024 , 32);
    if (retval != 0)
        return -1;

    fi = sfd_get_file_info(file_path);
    if (!fi)
        return -1;

    s1.id = 1;
    s1.sex = 1;
    s1.age = 18;
    strncpy(s1.name , "james gals" , 20);
    s1.name[19] = '\0';

    fai.pos = 1024;
    fai.size = 32;
    fai.data_addr = &s1;
    show_stu(&s1);
    sfd_add(fi , &fai); 
    fai.data_addr = &s11;
    sfd_read(fi , &fai);
    show_stu(&s11);

    return 0;
}

