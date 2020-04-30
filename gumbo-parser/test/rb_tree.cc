#include "rb_tree.h"
#include "test_utils.h"
#include "gtest/gtest.h"
#include <stddef.h>

namespace {

class TreeWalker {
  rb_node *node_;

public:
  explicit TreeWalker(rb_node *node) : node_(node) { }

  TreeWalker Left() const {
    EXPECT_NE(nullptr, node_);
    return TreeWalker(node_->left);
  }

  TreeWalker Right() const {
    EXPECT_NE(nullptr, node_);
    return TreeWalker(node_->right);
  }

  TreeWalker const &IsBlack(const std::string &str) const {
    EXPECT_NE(nullptr, node_);
    EXPECT_FALSE(rb_is_red(node_));
    EXPECT_EQ(str, std::string(node_->key, rb_key_length(node_)));
    return *this;
  }

  TreeWalker const &IsRed(const std::string &str) const {
    EXPECT_NE(nullptr, node_);
    EXPECT_TRUE(rb_is_red(node_));
    EXPECT_EQ(str, std::string(node_->key, rb_key_length(node_)));
    return *this;
  }

  TreeWalker const &IsLeaf() const {
    EXPECT_NE(nullptr, node_);
    EXPECT_EQ(nullptr, node_->left);
    EXPECT_EQ(nullptr, node_->right);
    return *this;
  }
};

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

  TreeWalker Walker() const { return TreeWalker(root_); }
};

TEST_F(RBTreeTest, RR_RootUpdate) {
  Clear();
  EXPECT_TRUE(Add("10"));
  EXPECT_TRUE(Add("20"));
  EXPECT_TRUE(Add("30"));

  Walker().IsBlack("20")
    .Left().IsRed("10").IsLeaf();
  Walker().Right().IsRed("30").IsLeaf();
}

TEST_F(RBTreeTest, LL_RootUpdate) {
  Clear();

  EXPECT_TRUE(Add("30"));
  EXPECT_TRUE(Add("20"));
  EXPECT_TRUE(Add("10"));

  Walker().IsBlack("20")
    .Left().IsRed("10").IsLeaf();
  Walker().Right().IsRed("30").IsLeaf();
}

TEST_F(RBTreeTest, RL_RootUpdate) {
  Clear();

  EXPECT_TRUE(Add("10"));
  EXPECT_TRUE(Add("30"));
  EXPECT_TRUE(Add("20"));

  Walker().IsBlack("20")
    .Left().IsRed("10").IsLeaf();
  Walker().Right().IsRed("30").IsLeaf();
}

TEST_F(RBTreeTest, LR_RootUpdate) {
  Clear();

  EXPECT_TRUE(Add("30"));
  EXPECT_TRUE(Add("10"));
  EXPECT_TRUE(Add("20"));

  Walker().IsBlack("20")
    .Left().IsRed("10").IsLeaf();
  Walker().Right().IsRed("30").IsLeaf();
}

TEST_F(RBTreeTest, RecolorRoot) {
  Clear();

  EXPECT_TRUE(Add("10"));
  EXPECT_TRUE(Add("20"));
  EXPECT_TRUE(Add("30"));

  Walker().IsBlack("20")
    .Left().IsRed("10");
  Walker().Right().IsRed("30");

  // Try to insert a duplicate key. This doesn't change the tree, but since
  // the two leaves are red, they will be recolored black. The root remains
  // black because that's a RB-tree invariant.
  EXPECT_FALSE(Add("10"));
  Walker().IsBlack("20")
    .Left().IsBlack("10");
  Walker().Right().IsBlack("30");
}

TEST_F(RBTreeTest, RR) {
  Clear();

  EXPECT_TRUE(Add("20"));
  EXPECT_TRUE(Add("10"));
  EXPECT_TRUE(Add("30"));
  EXPECT_TRUE(Add("40"));
  EXPECT_TRUE(Add("50"));

  TreeWalker node = Walker().IsBlack("20").Right().IsBlack("40");
  node.Left().IsRed("30");
  node.Right().IsRed("50");
}

TEST_F(RBTreeTest, LL) {
  Clear();

  EXPECT_TRUE(Add("20"));
  EXPECT_TRUE(Add("10"));
  EXPECT_TRUE(Add("50"));
  EXPECT_TRUE(Add("40"));
  EXPECT_TRUE(Add("30"));

  TreeWalker node = Walker().IsBlack("20").Right().IsBlack("40");
  node.Left().IsRed("30");
  node.Right().IsRed("50");
}

TEST_F(RBTreeTest, RL) {
  Clear();

  EXPECT_TRUE(Add("20"));
  EXPECT_TRUE(Add("10"));
  EXPECT_TRUE(Add("30"));
  EXPECT_TRUE(Add("50"));
  EXPECT_TRUE(Add("40"));

  TreeWalker node = Walker().IsBlack("20").Right().IsBlack("40");
  node.Left().IsRed("30");
  node.Right().IsRed("50");
}

TEST_F(RBTreeTest, LR) {
  Clear();

  EXPECT_TRUE(Add("20"));
  EXPECT_TRUE(Add("10"));
  EXPECT_TRUE(Add("50"));
  EXPECT_TRUE(Add("30"));
  EXPECT_TRUE(Add("40"));

  TreeWalker node = Walker().IsBlack("20").Right().IsBlack("40");
  node.Left().IsRed("30");
  node.Right().IsRed("50");
}

TEST_F(RBTreeTest, SmallTree) {
  Clear();

  EXPECT_TRUE(Add("10"));
  EXPECT_TRUE(Add("85"));
  EXPECT_TRUE(Add("15"));
  EXPECT_TRUE(Add("70"));
  EXPECT_TRUE(Add("20"));
  EXPECT_TRUE(Add("60"));
  EXPECT_TRUE(Add("30"));
  EXPECT_TRUE(Add("50"));
  EXPECT_TRUE(Add("65"));
  EXPECT_TRUE(Add("80"));
  EXPECT_TRUE(Add("90"));
  EXPECT_TRUE(Add("40"));
  EXPECT_TRUE(Add("05"));
  EXPECT_TRUE(Add("55"));

  Walker().IsBlack("30")
    .Left().IsBlack("15")
    .Left().IsBlack("10")
    .Left().IsRed("05").IsLeaf();
  Walker().Left().Right().IsBlack("20").IsLeaf();
  Walker().Right().IsBlack("70")
    .Left().IsRed("60")
    .Left().IsBlack("50")
    .Left().IsRed("40").IsLeaf();
  Walker().Right().Left().Left().Right().IsRed("55").IsLeaf();
  Walker().Right().Left().Right().IsBlack("65").IsLeaf();
  Walker().Right().Right().IsBlack("85")
    .Left().IsRed("80").IsLeaf();
  Walker().Right().Right().Right().IsRed("90").IsLeaf();
}

TEST_F(RBTreeTest, TestInsert) {
  Clear();

  EXPECT_TRUE(Add("foo"));
  EXPECT_TRUE(Add("bar"));
  EXPECT_TRUE(Add("baz"));
  EXPECT_TRUE(Add(""));
  EXPECT_TRUE(Add("longer"));
  EXPECT_TRUE(Add("another new one"));

  EXPECT_FALSE(Add("foo"));
  EXPECT_FALSE(Add("bar"));
  EXPECT_FALSE(Add("baz"));
  EXPECT_FALSE(Add(""));
  EXPECT_FALSE(Add("longer"));
  EXPECT_FALSE(Add("another new one"));
}

} // namespace
