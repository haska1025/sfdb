#include "../sfd_file_mgr.h"

int main()
{
    int retval;
    retval = sfd_file_mgr_init();

    if (retval != 0){
        printf("init file mgr failed!\n");
        return -1;
    }

    sfd_file_mgr_create_file("./user_data/test_2.data" , 4096,28);
    sfd_file_mgr_create_file("./user_data/test_3.data" , 4096,28);
    sfd_file_mgr_display();
    sfd_file_mgr_create_file("./user_data/test_1.data" , 4096,28);

    sfd_file_mgr_display();
    sfd_file_mgr_delete_file("./user_data/test_2.data");
///    sfd_file_mgr_delete_file("./user_data/test_3.data");

    sfd_file_mgr_display();
    return 0;
}
