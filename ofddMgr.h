/****************************************************************************
  FileName     [ ofddMgr.h ]
  PackageName  [ OFDD ]
  Synopsis     [ OFDD Manager: unique table + apply caches ]
****************************************************************************/

#ifndef OFDD_MGR_H
#define OFDD_MGR_H

#include <map>
#include "myHash.h"
#include "ofddNode.h"

using namespace std;

class OfddNode;

typedef vector<size_t>                        OfddArr;
typedef map<string, size_t>                   OfddMap;
typedef pair<string, size_t>                  OfddMapPair;
typedef map<string, size_t>::const_iterator   OfddMapConstIter;

extern OfddMgr* ofddMgr;

class OfddHashKey
{
public:
   OfddHashKey(size_t l, size_t r, unsigned i) : _l(l), _r(r), _i(i) {}
   size_t operator() () const { return ((_l << 3) + (_r << 3) + _i); }
   bool operator == (const OfddHashKey& k) const {
      return (_l == k._l) && (_r == k._r) && (_i == k._i); }

private:
   size_t     _l;
   size_t     _r;
   unsigned   _i;
};

enum OfddApplyOp { OFDD_OP_XOR = 0, OFDD_OP_AND = 1, OFDD_OP_OR = 2 };

class OfddCacheKey
{
public:
   OfddCacheKey() : _f(0), _g(0), _op(0) {}
   OfddCacheKey(size_t f, size_t g, unsigned op) : _f(f), _g(g), _op(op) {}
   size_t operator() () const { return ((_f << 3) + (_g << 3) + (_op << 5)); }
   bool operator == (const OfddCacheKey& k) const {
      return (_f == k._f) && (_g == k._g) && (_op == k._op); }

private:
   size_t       _f;
   size_t       _g;
   unsigned     _op;
};

class OfddMgr
{
typedef Hash<OfddHashKey, OfddNodeInt*> OfddHash;
typedef Cache<OfddCacheKey, size_t>     OfddCache;

public:
   OfddMgr(size_t nin = 64, size_t h = 8009, size_t c = 30011)
   { init(nin, h, c); }
   ~OfddMgr() { reset(); }

   void init(size_t nin, size_t h, size_t c);
   void restart();

   OfddNode ofddXor(OfddNode f, OfddNode g);
   OfddNode ofddAnd(OfddNode f, OfddNode g);
   OfddNode ofddOr(OfddNode f, OfddNode g);
   OfddNode ofddNot(OfddNode f);

   const OfddNode& getSupport(size_t i) const { return _supports[i]; }
   size_t getNumSupports() const { return _supports.size(); }

   OfddNodeInt* uniquify(size_t l, size_t r, unsigned i);

   bool addOfddNode(unsigned id, size_t nodeV);
   OfddNode getOfddNode(unsigned id) const;
   bool addOfddNode(const string& nodeName, size_t nodeV);
   void forceAddOfddNode(const string& nodeName, size_t nodeV);
   OfddNode getOfddNode(const string& nodeName) const;

   int evalCube(const OfddNode& node, const string& vector) const;
   bool drawOfdd(const string& nodeName, const string& dotFile) const;

private:
   vector<OfddNode>  _supports;
   OfddHash          _uniqueTable;
   OfddCache         _computedTable;

   OfddArr           _ofddArr;
   OfddMap           _ofddMap;

   void reset();
   unsigned topVar(const OfddNode& f, const OfddNode& g) const;
};

#endif // OFDD_MGR_H
