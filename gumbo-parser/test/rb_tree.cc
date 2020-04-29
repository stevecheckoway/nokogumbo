#include "rb_tree.h"
#include "test_utils.h"
#include "gtest/gtest.h"
#include <stddef.h>

namespace {

class RBTreeTest : public GumboTest {
protected:
  RBTreeTest() : root_(nullptr) {}
  ~RBTreeTest() { Clear(); }

  rb_node *root_;

  void Clear() {
    rb_free(root_);
    root_ = nullptr;
  }

  bool Add(char const *str) { return rb_insert(&root_, strlen(str), str); }
};

TEST_F(RBTreeTest, TestInsert) {
  Clear();

  EXPECT_EQ(true, Add("foo"));
  EXPECT_EQ(true, Add("bar"));
  EXPECT_EQ(true, Add("baz"));
  EXPECT_EQ(true, Add(""));
  EXPECT_EQ(true, Add("longer"));
  EXPECT_EQ(true, Add("another new one"));

  EXPECT_EQ(false, Add("foo"));
  EXPECT_EQ(false, Add("bar"));
  EXPECT_EQ(false, Add("baz"));
  EXPECT_EQ(false, Add(""));
  EXPECT_EQ(false, Add("longer"));
  EXPECT_EQ(false, Add("another new one"));
}

} // namespace
