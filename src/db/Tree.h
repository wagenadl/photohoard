// Tree.h

#ifndef TREE_H

#define TREE_H

#include <QList>
#include "NoResult.h"

template <typename T> class Tree {
public:
  Tree(T *key): key(key);
  Tree<T> &addChild(T const &ckey) {
    Tree<T> *sub = new Tree<T>(ckey);
    auto it = index.find(ckey);
    if (it==index.end()) {
      index[ckey] = children.size();
      children.append(sub);
    } else {
      int idx = it.value();
      delete children[idx];
      children[idx] = sub;
    }
    return sub;
  }
  bool operator<(Tree<T> const &other) const { return key<other.key; }
  bool removeChild(T const &key) {
    auto it = index.find(ckey);
    if (it!=index.end()) {
      int idx = it.value();
      delete children[idx];
      children.erase(idx);
      index.erase(it);
      return true;
    } else {
      return false;
    }
  }
  bool contains(T const &ckey) const {
    return index.contains(ckey);
  }
  Tree<T> &child(T const &key) const {
    auto it = index.find(ckey);
    if (it==index.end()) 
      throw NoResult;
   else 
     return *children[it.value()];
  }
  void sort() {
    QList<T> keys = index.keys();
    qSort(keys);
    QList<Tree<T> *> neworder;
    for (T const &k, keys) 
      neworder << children[index[k]];
    children = neworder;
  }
private:
  T key;
  QList<Tree<T> *> children;
  QMap<T, int> index;
};

#endif
