// Implements a simple red-black tree to hold strings.

#include "rb_tree.h"

#include <assert.h>
#include <string.h>

#include "util.h"

// Don't put this in the header because there's no reason to expose it but we
// do need its value in the header so just duplicate the code.
#define COLOR_MASK ((size_t){1} << (sizeof(size_t) * CHAR_BIT - 1))

static void rb_set_red(rb_node *node) {
  if (node)
    node->key_length |= COLOR_MASK;
}

static void rb_set_black(rb_node *node) {
  if (node)
    node->key_length &= ~COLOR_MASK;
}

static rb_node *rb_new_node(size_t key_length, char const *key) {
  rb_node *node = gumbo_alloc(sizeof *node);
  node->left = 0;
  node->right = 0;
  node->key_length = key_length;
  node->key = key;
  return node;
}

// Returns < 0 if node's key is lexicographically before key.
// Returns 0 if node's key is equal to key.
// Returns > 0 if node's key is lexicographically after key.
static int rb_compare(rb_node const *node, size_t key_length,
                      char const key[key_length]) {
  assert(node);
  size_t node_key_length = rb_key_length(node);
  if (node_key_length < key_length)
    return -1;
  if (node_key_length > key_length)
    return 1;
  return memcmp(node->key, key, key_length);
}

// Perform one of the four tree rotations based on the relationship between
// the current, parent, and grandparent. If great_grandparent is not NULL,
// then the appropriate child pointer is updated to point to the rotated
// subtree's root. Otherwise, the grandparent must be the root and *root_ptr
// is set to the rotated subtree's root instead.
// Returns the root of the rotated subtree.
static rb_node *rb_rotate(rb_node *const current, rb_node *const parent,
                          rb_node *const grandparent,
                          rb_node *const great_grandparent,
                          rb_node **const root_ptr) {
  rb_node *rotated_subtree_root;
  rb_node *new_parent = parent;

  assert(current);
  assert(parent);
  assert(grandparent);
  assert(rb_is_red(current));
  assert(rb_is_red(parent));
  assert(!rb_is_red(grandparent));
  // Unfortunately, there are many cases to consider here. Let's take them one
  // at a time. In the diagrams, a bare letter will denote a black node, a
  // letter in parentheses will denote a red node.
  // X = current
  // P = parent
  // S = parent's sibling
  // G = grandparent
  // A, B, C = (possibly empty) subtrees (where relevant).
  if (parent == grandparent->left) {
    if (current == parent->left) {
      // Case 1. Left-left rotation.
      // |          G                 P         |
      // |        /   \             /   \       |
      // |     (P)      S   ->   (X)     (G)    |
      // |    /   \                     /   \   |
      // |  (X)    A                   A     S  |
      rb_node *sibling = grandparent->right;
      rb_node *a_subtree = parent->right;

      // Update the pointers but defer updating the colors.
      rotated_subtree_root = parent;
      parent->left = current;
      parent->right = grandparent;
      grandparent->left = a_subtree;
      grandparent->right = sibling;
    } else {
      // Case 2. Left-right rotation.
      // |         G                 X        |
      // |       /   \             /   \      |
      // |    (P)      S   ->   (P)     (G)   |
      // |   /   \              / \     / \   |
      // |  A    (X)           A   B   C   S  |
      // |       / \                          |
      // |      B   C                         |
      rb_node *b_subtree = current->left;
      rb_node *c_subtree = current->right;

      // Update the pointers. The parent's left child and the grandparent's
      // right child don't change.
      rotated_subtree_root = current;
      current->left = parent;
      current->right = grandparent;
      parent->right = b_subtree;
      grandparent->left = c_subtree;

      // Current's new parent is the (old) great grandparent.
      new_parent = great_grandparent;
    }
  } else {
    if (current == parent->left) {
      // Case 3. Right-left rotation. (Mirror image of case 2.)
      // |      G                   X        |
      // |    /   \               /   \      |
      // |  S      (P)    ->   (G)     (P)   |
      // |        /   \        / \     / \   |
      // |      (X)    A      S   C   B   A  |
      // |      / \                          |
      // |     C   B                         |
      rb_node *b_subtree = current->right;
      rb_node *c_subtree = current->left;

      // Update the pointers. The parent's right child and the grandparent's
      // left child don't change.
      rotated_subtree_root = current;
      current->right = parent;
      current->left = grandparent;
      parent->left = b_subtree;
      grandparent->right = c_subtree;

      // Current's new parent is the (old) great grandparent.
      new_parent = great_grandparent;
    } else {
      // Case 4. Right-right rotation. (Mirror image of case 1.)
      // |      G                    P       |
      // |    /   \                /   \     |
      // |  S      (P)    ->    (G)     (X)  |
      // |        /   \        /   \         |
      // |       A    (X)     S     A        |
      rb_node *sibling = grandparent->left;
      rb_node *a_subtree = parent->left;

      // Update the pointers.
      rotated_subtree_root = parent;
      parent->right = current;
      parent->left = grandparent;
      grandparent->right = a_subtree;
      grandparent->left = sibling;
    }
  }

  // In all cases, the new root of our rotated subtree is black and the
  // (old) grandparent is red.
  rb_set_black(rotated_subtree_root);
  rb_set_red(grandparent);

  // The last step is to update the pointer in the great grandparent, if it
  // exists; otherwise, the old grandparent was the root so we need to update
  // the root pointer.
  if (great_grandparent) {
    if (grandparent == great_grandparent->left)
      great_grandparent->left = rotated_subtree_root;
    else
      great_grandparent->right = rotated_subtree_root;
  } else {
    // If there is no great grandparent, then the grandparent must be the
    // root.
    assert(grandparent == *root_ptr);
    *root_ptr = rotated_subtree_root;
  }

  return new_parent;
}

// Perform a top-down insertion into the tree. Returns true if the key was
// inserted into the tree and false otherwise. A copy of the key is not
// created so the key must outlive the tree.
bool rb_insert(rb_node **root_ptr, size_t key_length, char const *key) {
  assert(root_ptr);
  assert(key_length < COLOR_MASK);
  rb_node *current = *root_ptr;
  rb_node *parent = 0;
  rb_node *grandparent = 0;
  rb_node *great_grandparent = 0;

  if (!current) {
    // The tree is empty, create a new root.
    *root_ptr = rb_new_node(key_length, key);
    return true;
  }

  // The root should be black.
  assert(!rb_is_red(current));

  int cmp;
  while ((cmp = rb_compare(current, key_length, key)) != 0) {
    // If both of current's children are red, swap the colors of current and
    // its children and fix up the parents if necessary.
    if (rb_is_red(current->left) && rb_is_red(current->right)) {
      assert(!rb_is_red(current));
      // If parent is NULL, then current is the root and we should never set
      // the root to be red. The check if the parent is red will be false
      // below in this case so we won't try to do any rotates.
      if (parent)
        rb_set_red(current);
      rb_set_black(current->left);
      rb_set_black(current->right);

      if (rb_is_red(parent)) {
        parent = rb_rotate(current, parent, grandparent, great_grandparent,
                           root_ptr);
        // grandparent and great_grandparent are no longer valid, but we can't
        // rotate again for two more levels by which time parent will have
        // become great_grandparent and current will have become grandparent.
        grandparent = 0;
        great_grandparent = 0;
      }
    }

    // Descend one level.
    great_grandparent = grandparent;
    grandparent = parent;
    parent = current;
    // If cmp < 0, move right; otherwise, move left.
    current = cmp < 0 ? current->right : current->left;

    if (!current)
      break;
  }

  if (cmp == 0) {
    // The key is already in the tree.
    return false;
  }

  current = rb_new_node(key_length, key);
  rb_set_red(current);
  if (cmp < 0)
    parent->right = current;
  else
    parent->left = current;
  if (rb_is_red(parent))
    parent = rb_rotate(current, parent, grandparent, great_grandparent,
                       root_ptr);
  return true;
}

// Free the tree. Since the tree does not own the keys, they are not freed.
// Users must free the keys themselves.
void rb_free(rb_node *root) {
  if (!root)
    return;
  rb_free(root->left);
  rb_free(root->right);
  gumbo_free(root);
}
