#include "rb_tree.h"
#include "stdio.h"

int int_cmp(void *l , void *r)
{
    int li , ri;
    li = *((int*)l);
    ri = *((int*)r);

    return li == ri ? 0 : (li < ri ? -1 : 1);
}

int data_process(void *key , void *value)
{
    int k , v;

    k = *((int*)key);
    v = *((int*)value);

    printf("<%d , %d>\n" , k , v);

    return 0;
}
int main()
{
    struct rb_tree rb;

    int i1 ,i2 , i3 , i4 , i5 ,i6;

    i1 = 100;
    i2 = 98;
    i3 = 77;
    i4 = 222;
    i5 = 20;
    i6 =1;

    rb_tree_init(&rb , int_cmp);

    rb_insert(&rb , &i1 , &i1); 
    rb_insert(&rb , &i2 , &i2); 
    rb_insert(&rb , &i3 , &i3); 
    rb_insert(&rb , &i4 , &i4); 
    rb_insert(&rb , &i5 , &i5); 
    rb_insert(&rb , &i6 , &i6); 

    rb_for_each(&rb , data_process);

#ifdef DEBUG
    display_rb_tree(&rb);
#endif

    rb_delete(&rb , &i2);


    rb_for_each(&rb , data_process);

    return 0;
}
