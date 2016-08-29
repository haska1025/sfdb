#include <stdlib.h>
#include "rb_tree.h"


struct rb_node
{
    struct rb_node *parent;
    struct rb_node *lchild;
    struct rb_node *rchild;
    int color;
    void *key;
    void *value;
};
enum
{
    RED = 1,
    BLACK = 2
};

static struct rb_node nil_node;

#define parent(node) (node)->parent
#define left(node) (node)->lchild
#define right(node) (node)->rchild
#define set_parent(n , v) (n)->parent=v
#define set_left(n , v) (n)->lchild=v
#define set_right(n , v) (n)->rchild=v
#define set_color(n , c) (n)->color=c

#define INIT_RB_NODE(node , key , value) \
{ \
    node->parent = &nil_node; \
    node->lchild = &nil_node; \
    node->rchild = &nil_node; \
    node->key = key; \
    node->value = value; \
    node->color = RED; \
}

static inline int is_red(struct rb_node *node)
{
    return node->color == RED;
}
static inline int is_black(struct rb_node *node)
{
    return node->color == BLACK;
}
static inline int is_nill(struct rb_node *node)
{
    return node == &nil_node;
}
static inline int is_root(struct rb_node *node)
{
    return (node->parent == &nil_node);
}
static inline int is_left(struct rb_node *node)
{
    struct rb_node *p;
    p = parent(node);

    return node == left(p);
}

static inline int is_right(struct rb_node *node)
{
    struct rb_node *p;
    p = parent(node);

    return node  == right(p);
}
/*------------------------------- internal functions --------------------------*/
static struct rb_node * __rb_minimum(struct rb_node *node)
{
    while(!is_nill(left(node)))
        node = left(node);
    
    return node;
}
static struct rb_node * __rb_maximum(struct rb_node *node)
{
    while(!is_nill(right(node)))
        node = right(node);
    
    return node;
}
static struct rb_node * __rb_predecessor(struct rb_node *node)
{
    if (!is_nill(left(node)))
        return __rb_maximum(left(node));

    while(parent(node) && is_left(node)){
        node = parent(node);
    }

    return parent(node);
}
static struct rb_node * __rb_successor(struct rb_node *node)
{
    if (!is_nill(right(node)))
        return __rb_minimum(right(node));

    while(parent(node) && is_right(node)){
        node = parent(node);
    }

    return parent(node);
}

static inline void __rb_left_rotate(struct rb_tree *rb , struct rb_node *node)
{
    struct rb_node *x , *y;

    x = node;
    y = right(x);
    set_right(x , left(y));
    if (!is_nill(left(y)))
        set_parent(left(y) , x);

    set_left(y , x);
    set_parent(y , parent(x));
    if(is_root(x)){
        rb->root = y;
    }else if(is_left(x)){
        set_left(parent(x) , y);
    }else{
        set_right(parent(x) , y);
    }

    set_parent(x , y);
}

static inline void __rb_right_rotate(struct rb_tree *rb , struct rb_node *node)
{
    struct rb_node *x , *y;

    x = node;
    y = left(x);
    set_left(x , right(y)); 
    if (!is_nill(right(y)))
        set_parent(right(y) , x);

    set_right(y , x);
    set_parent(y , parent(x));
    if(is_root(x)){
        rb->root = y;
    }else if(is_left(x)){
        set_left(parent(x) , y);
    }else{
        set_right(parent(x) , y);
    }

    set_parent(x , y);
}

static inline struct rb_node * __rb_find(struct rb_tree *rb , void *key)
{
struct rb_node *n;
    int r;
    
    n = rb->root;
    while(!is_nill(n)){
        r = rb->rb_key_cmp(key , n->key);
        if( r == 0)
            return n;
        else if (r < 0)
            n = n->lchild;
        else 
            n = n->rchild;
    }

    return NULL;
}

static void __rb_insert_fixed(struct rb_tree *rb , struct rb_node *node)
{
    struct rb_node *z, *p , *pp , *y;

    z = node;
    while(is_red(parent(z))){
        p = parent(z);
        pp = parent(p);
        if (is_left(p)){
            y = right(pp);
            if (is_red(y)){
               set_color(y , BLACK);
               set_color(p , BLACK);
               set_color(pp , RED);
               z = pp;
            }else{
                if (is_right(z)){
                    z = p;
                    __rb_left_rotate(rb , z);
                }
                p = parent(z);
                pp = parent(p);
                set_color(p , BLACK);
                set_color(pp , RED);
                __rb_right_rotate(rb , pp);
            }
        }else{
            y = left(pp);
            if (is_red(y)){
                set_color(y , BLACK);
                set_color(p , BLACK);
                set_color(pp , RED);
                z = pp;
            }else{
                if (is_left(z)){
                    z = p;
                    __rb_right_rotate(rb , z);
                }
                p = parent(z);
                pp = parent(p);
                set_color(p , BLACK);
                set_color(pp , RED);
                __rb_left_rotate(rb , pp);
            }
        }
    }

    set_color(rb->root , BLACK);
}

static void __rb_delete_fixed(struct rb_tree *rb , struct rb_node *node)
{
    struct rb_node *x , *w , *p;

    x = node;

    while(!is_root(x) && is_black(x)){
        p = parent(x);
        if(is_left(x)){
            w = right(p);
            if(is_red(w)){
                set_color(p , RED);
                set_color(w , BLACK);
                __rb_left_rotate(rb , p);
                w = right(p);
            }

            if(is_black(left(w)) && is_black(right(w))){
                set_color(w , RED);
                x = p;
            }else{
                if(is_black(right(w))){
                    set_color(w , RED);
                    set_color(left(w) , BLACK);
                    __rb_right_rotate(rb , w);
                    w = right(p);
                }
                set_color(p , BLACK);
                set_color(right(w) , BLACK);
                __rb_left_rotate(rb , p);
                x = rb->root;
            }

        }else{
            w = left(p);
            if(is_red(w)){
                set_color(p , RED);
                set_color(w , BLACK);
                __rb_right_rotate(rb , p);
                w = left(p);
            }

            if(is_black(left(w)) && is_black(right(w))){
                set_color(w , RED);
                x = p;
            }else{
                if(is_black(left(w))){
                    set_color(w , RED);
                    set_color(right(w) , BLACK);
                    __rb_left_rotate(rb , w);
                    w = left(p);
                }

                set_color(p , BLACK);
                set_color(left(w) , BLACK);
                __rb_right_rotate(rb , p);
                x = rb->root;
            }
        }
    }

    set_color(x , BLACK);
}

/*------------------------------  external functions -------------------------*/

int rb_tree_init(struct rb_tree *rb , int (*rb_key_cmp)(void *lkey , void *rkey))
{
    rb->rb_key_cmp = rb_key_cmp;
    rb->root = &nil_node;
    nil_node.color = BLACK;

    return 0;
}
void * rb_find(struct rb_tree *rb , void *key)
{
    struct rb_node *n;
    
    n = __rb_find(rb , key);
    if (n)
        return n->value;

    return NULL;
}

int rb_insert(struct rb_tree *rb , void *key , void *value)
{
    struct rb_node *n , *p;
    struct rb_node **ps;
    struct rb_node *new_node;
    int r;

    n = rb->root;
    ps = &(rb->root);
    p = &nil_node;
    while(!is_nill(n)){
        p = n;
        r = rb->rb_key_cmp(key , n->key);
        if( r == 0){
            n->value = value;
            return 0;
        }else if (r < 0){
            ps = &(n->lchild);
            n = n->lchild;
        } else{
            ps = &(n->rchild);
            n = n->rchild;
        }
    }

    new_node = (struct rb_node*)malloc(sizeof(struct rb_node));
    if(!new_node)
        return -1;

    INIT_RB_NODE(new_node , key ,value);

    set_parent(new_node , p);
    *ps = new_node;

    __rb_insert_fixed(rb , new_node);

    return 0;
}

int rb_delete(struct rb_tree *rb , void *key , void **d_k , void **d_v)
{
    struct rb_node *d;
    struct rb_node *x , *y;

    d = __rb_find(rb , key);
    if (!d)
        return -1;

    if (is_nill(left(d)) || is_nill(right(d))){
        y = d;
    }else{
        y = __rb_successor(d);
    }

    if (!is_nill(left(y))){
        x = left(y);
    }else{
        x = right(y);
    }

    set_parent(x , parent(y));
    if (is_root(y)){
        rb->root = x;
    }else if (is_left(y)){
        set_left(parent(y) , x);
    }else{
        set_right(parent(y) , x);
    }

    *d_k = d->key;
    *d_v = d->value;

    if ( y != d){
        d->key = y->key;
        d->value = y->value;
    }

    if (is_black(y))
        __rb_delete_fixed(rb , x);


    free(y);
    y = NULL;

    return 0;

}


int rb_for_each(struct rb_tree *rb ,int (*process)(void *key , void *value))
{
    struct rb_node *p;
    
    if(is_nill(rb->root))
        return 0;

    p = __rb_minimum(rb->root);

    while( !is_nill(p)){
        process(p->key , p->value);
        p = __rb_successor(p);
    }

    return 0;
}

#ifdef DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void __display_tree(struct rb_node *r , const char *prefix_str)
{
    char *space_str = "|   ";
    char c[10] = {0};
    int k , v;
    int space_len;
    int p_len;
    char *new_prefix;


    if (is_nill(r)){
        printf("%s\n" , prefix_str);
        return;
    }

    k = *((int *)r->key);
    v = *((int *)r->value);

    if (r->color == RED)
        strcpy(c , "RED");
    else
        strcpy(c , "BLACK");

    printf("%s <%d ,%d,%s>\n" , prefix_str , k , v , c); 

    space_len = strlen(space_str);
    p_len = strlen(prefix_str);
    new_prefix = (char *)malloc(space_len + p_len + 1);
    strncpy(new_prefix , space_str , space_len);
    strncpy(new_prefix + space_len , prefix_str , p_len);

    new_prefix[space_len + p_len] = '\0';


    __display_tree(left(r) , new_prefix);
    __display_tree(right(r) , new_prefix);
}

void display_rb_tree(struct rb_tree *rb)
{
    char *underline_str = "|___";

    __display_tree(rb->root , underline_str);
}
#endif

