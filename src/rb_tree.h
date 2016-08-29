#ifndef _RB_TREE_H_
#define _RB_TREE_H_

struct rb_node;

struct rb_tree
{
    struct rb_node *root;
    int (*rb_key_cmp)(void *lkey , void *rkey);
};

extern int rb_tree_init(struct rb_tree *rb , int (*rb_key_cmp)(void *lkey , void *rkey));

extern void * rb_find(struct rb_tree *rb , void *key);
extern int rb_insert(struct rb_tree *rb , void *key , void *value);
extern int rb_delete(struct rb_tree *rb , void *key , void **d_k , void **d_v);
extern int rb_for_each(struct rb_tree *rb ,int (*process)(void *key , void *value));

#ifdef DEBUG
extern void display_rb_tree(struct rb_tree *rb);
#endif
#endif//_RB_TREE_H_

