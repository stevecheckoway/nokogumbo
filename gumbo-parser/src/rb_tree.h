#ifndef RB_TREE_H
#define RB_TREE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rb_node rb_node;

// Perform a top-down insertion into the tree. Returns true if the key was
// inserted into the tree and false otherwise. A copy of the key is not
// created so the key must outlive the tree.
//
// Usage:
// rb_node *root = 0;
// rb_insert(&root, 3, "foo");  // returns true
// rb_insert(&root, 4, "fizz"); // returns true
// rb_insert(&root, 3, "foo");  // return false
bool rb_insert(rb_node **root_ptr, size_t key_length, char const *key);

// Free the tree. Since the tree does not own the keys, they are not freed.
// Users must free the keys themselves.
void rb_free(rb_node *root);

#ifdef __cplusplus
}
#endif

#endif
