/****************************************************************************
  FileName     [ ofddNode.h ]
  PackageName  [ OFDD ]
  Synopsis     [ Ordered Functional Decision Diagram (positive Davio OFDD) ]
  Description  [
    Each internal node at variable x represents
      f = f|0  XOR  ( x AND ( f|0 XOR f|1 ) )
    i.e. positive Davio: f = f0 XOR (x AND f1') with f0 = left, f1' = right
    where right = f|0 XOR f|1 (Boolean XOR / derivative w.r.t. x).
    Reduction: if right == 0 then f = f0 (no node).
  ]
****************************************************************************/

#ifndef OFDD_NODE_H
#define OFDD_NODE_H

#include <vector>
#include <map>
#include <iostream>
#include <fstream>

using namespace std;

#define OFDD_EDGE_BITS      2
#define OFDD_NODE_PTR_MASK  ((~(size_t(0)) >> OFDD_EDGE_BITS) << OFDD_EDGE_BITS)

class OfddMgr;
class OfddNodeInt;

enum OFDD_EDGE_FLAG
{
   OFDD_POS_EDGE = 0,
   OFDD_NEG_EDGE = 1,
   OFDD_EDGE_DUMMY
};

class OfddNode
{
   friend class OfddMgr;
public:
   static OfddNode          _one;
   static OfddNode          _zero;
   static bool              _debugOfddAddr;
   static bool              _debugRefCount;

   OfddNode() : _node(0) {}
   OfddNode(size_t l, size_t r, size_t i, OFDD_EDGE_FLAG f = OFDD_POS_EDGE);
   OfddNode(const OfddNode& n);
   OfddNode(OfddNodeInt* n, OFDD_EDGE_FLAG f = OFDD_POS_EDGE);
   OfddNode(size_t v);
   ~OfddNode();

   const OfddNode& getLeft() const;
   const OfddNode& getRight() const;
   /** Shannon cofactor f|_{xi=0} */
   OfddNode getCof0(unsigned i) const;
   /** Shannon cofactor f|_{xi=1} */
   OfddNode getCof1(unsigned i) const;
   unsigned getLevel() const;
   unsigned getRefCount() const;
   bool isNegEdge() const { return (_node & OFDD_NEG_EDGE); }
   bool isPosEdge() const { return !isNegEdge(); }

   size_t operator () () const { return _node; }
   OfddNode operator ~ () const { return (_node ^ OFDD_NEG_EDGE); }
   OfddNode& operator = (const OfddNode& n);
   OfddNode operator & (const OfddNode& n) const;
   OfddNode& operator &= (const OfddNode& n);
   OfddNode operator | (const OfddNode& n) const;
   OfddNode& operator |= (const OfddNode& n);
   OfddNode operator ^ (const OfddNode& n) const;
   OfddNode& operator ^= (const OfddNode& n);
   bool operator == (const OfddNode& n) const { return (_node == n._node); }
   bool operator != (const OfddNode& n) const { return (_node != n._node); }
   bool operator < (const OfddNode& n) const;
   bool operator <= (const OfddNode& n) const;
   bool operator > (const OfddNode& n) const { return !((*this) <= n); }
   bool operator >= (const OfddNode& n) const { return !((*this) < n); }

   string toString() const;

   friend ostream& operator << (ostream& os, const OfddNode& n);

   void drawOfdd(const string&, ofstream&) const;
   string getLabel() const;

   static void setOfddMgr(OfddMgr* m) { _OfddMgr = m; }

private:
   size_t                  _node;
   static OfddMgr*         _OfddMgr;

   OfddNodeInt* getOfddNodeInt() const {
      return (OfddNodeInt*)(_node & OFDD_NODE_PTR_MASK); }
   bool isTerminal() const;
   void print(ostream&, size_t, size_t&) const;
   void unsetVisitedRecur() const;
   void drawOfddRecur(ofstream&) const;
};

class OfddNodeInt
{
   friend class OfddNode;
   friend class OfddMgr;

   OfddNodeInt() : _level(0), _refCount(0), _visited(0) {}
   OfddNodeInt(size_t l, size_t r, unsigned ll)
   : _left(l), _right(r), _level(ll), _refCount(0), _visited(0) {}

   const OfddNode& getLeft() const { return _left; }
   const OfddNode& getRight() const { return _right; }
   unsigned getLevel() const { return _level; }
   unsigned getRefCount() const { return _refCount; }
   void incRefCount() { ++_refCount; }
   void decRefCount() { --_refCount; }
   bool isVisited() const { return (_visited == 1); }
   void setVisited() { _visited = 1; }
   void unsetVisited() { _visited = 0; }

   OfddNode              _left;
   OfddNode              _right;
   unsigned              _level    : 16;
   unsigned              _refCount : 15;
   unsigned              _visited  : 1;

   static OfddNodeInt*   _terminal;
};

#endif // OFDD_NODE_H
