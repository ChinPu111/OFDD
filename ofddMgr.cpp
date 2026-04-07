/****************************************************************************
  FileName     [ ofddMgr.cpp ]
  PackageName  [ OFDD ]
  Synopsis     [ OFDD manager: uniquify + Davio apply (xor/and/or) ]
****************************************************************************/

#include <fstream>
#include <cassert>
#include "ofddNode.h"
#include "ofddMgr.h"

using namespace std;

OfddMgr* ofddMgr = new OfddMgr;

bool myStr2Int(const string&, int&);

static OfddNode getOfddNodeByName(const string& bddName)
{
   int id;
   if (myStr2Int(bddName, id))
      return ofddMgr->getOfddNode(id);
   return ofddMgr->getOfddNode(bddName);
}

void
OfddMgr::init(size_t nin, size_t h, size_t c)
{
   reset();
   _uniqueTable.init(h);
   _computedTable.init(c);

   OfddNode::setOfddMgr(this);
   OfddNodeInt::_terminal = uniquify(0, 0, 0);
   OfddNode::_one = OfddNode(OfddNodeInt::_terminal, OFDD_POS_EDGE);
   OfddNode::_zero = OfddNode(OfddNodeInt::_terminal, OFDD_NEG_EDGE);

   _supports.reserve(nin + 1);
   _supports.push_back(OfddNode::_one);
   for (size_t i = 1; i <= nin; ++i)
      _supports.push_back(OfddNode(OfddNode::_zero(), OfddNode::_one(), i));
}

void
OfddMgr::restart()
{
   size_t nin = _supports.size() - 1;
   size_t h   = _uniqueTable.numBuckets();
   size_t c   = _computedTable.size();
   init(nin, h, c);
}

void
OfddMgr::reset()
{
   _supports.clear();
   _ofddArr.clear();
   _ofddMap.clear();
   OfddHash::iterator bi = _uniqueTable.begin();
   for (; bi != _uniqueTable.end(); ++bi)
      delete (*bi).second;
   _uniqueTable.reset();
   _computedTable.reset();
}

OfddNodeInt*
OfddMgr::uniquify(size_t l, size_t r, unsigned i)
{
   if (i > 0 && r == OfddNode::_zero()) {
      OfddNode L(l);
      OfddNodeInt* p = L.getOfddNodeInt();
      assert(p != 0);
      return p;
   }

   OfddHashKey k(l, r, i);
   OfddNodeInt* n = 0;
   if (!_uniqueTable.check(k, n)) {
      n = new OfddNodeInt(l, r, i);
      _uniqueTable.forceInsert(k, n);
   }
   return n;
}

unsigned
OfddMgr::topVar(const OfddNode& f, const OfddNode& g) const
{
   if (f.isTerminal() && g.isTerminal()) return 0;
   if (f.isTerminal()) return g.getLevel();
   if (g.isTerminal()) return f.getLevel();
   return (f.getLevel() > g.getLevel()) ? f.getLevel() : g.getLevel();
}

OfddNode
OfddMgr::ofddXor(OfddNode f, OfddNode g)
{
   if (f == g) return OfddNode::_zero;
   if (f == OfddNode::_zero) return g;
   if (g == OfddNode::_zero) return f;

   OfddCacheKey k(f(), g(), OFDD_OP_XOR);
   size_t ret_t;
   if (_computedTable.read(k, ret_t))
      return ret_t;

   unsigned v = topVar(f, g);
   assert(v > 0);

   OfddNode f0 = f.getCof0(v), f1 = f.getCof1(v);
   OfddNode g0 = g.getCof0(v), g1 = g.getCof1(v);
   OfddNode n0 = ofddXor(f0, g0);
   OfddNode fd = ofddXor(f0, f1);
   OfddNode gd = ofddXor(g0, g1);
   OfddNode nd = ofddXor(fd, gd);

   OfddNode ret = (nd == OfddNode::_zero) ? n0 : OfddNode(n0(), nd(), v);
   _computedTable.write(k, ret());
   return ret;
}

OfddNode
OfddMgr::ofddAnd(OfddNode f, OfddNode g)
{
   if (f == g) return f;
   if (f == OfddNode::_zero || g == OfddNode::_zero) return OfddNode::_zero;
   if (f == OfddNode::_one) return g;
   if (g == OfddNode::_one) return f;

   OfddCacheKey k(f(), g(), OFDD_OP_AND);
   size_t ret_t;
   if (_computedTable.read(k, ret_t))
      return ret_t;

   unsigned v = topVar(f, g);
   assert(v > 0);

   OfddNode f0 = f.getCof0(v), f1 = f.getCof1(v);
   OfddNode g0 = g.getCof0(v), g1 = g.getCof1(v);
   OfddNode a = ofddAnd(f0, g0);
   OfddNode t = ofddAnd(f1, g1);
   OfddNode nh = ofddXor(a, t);

   OfddNode ret = (nh == OfddNode::_zero) ? a : OfddNode(a(), nh(), v);
   _computedTable.write(k, ret());
   return ret;
}

OfddNode
OfddMgr::ofddOr(OfddNode f, OfddNode g)
{
   if (f == g) return f;
   if (f == OfddNode::_zero) return g;
   if (g == OfddNode::_zero) return f;
   if (f == OfddNode::_one || g == OfddNode::_one) return OfddNode::_one;

   OfddCacheKey k(f(), g(), OFDD_OP_OR);
   size_t ret_t;
   if (_computedTable.read(k, ret_t))
      return ret_t;

   unsigned v = topVar(f, g);
   assert(v > 0);

   OfddNode f0 = f.getCof0(v), f1 = f.getCof1(v);
   OfddNode g0 = g.getCof0(v), g1 = g.getCof1(v);
   OfddNode a = ofddOr(f0, g0);
   OfddNode t = ofddOr(f1, g1);
   OfddNode nh = ofddXor(a, t);

   OfddNode ret = (nh == OfddNode::_zero) ? a : OfddNode(a(), nh(), v);
   _computedTable.write(k, ret());
   return ret;
}

OfddNode
OfddMgr::ofddNot(OfddNode f)
{
   return ofddXor(f, OfddNode::_one);
}

bool
OfddMgr::addOfddNode(unsigned id, size_t n)
{
   if (id >= _ofddArr.size()) {
      unsigned origSize = _ofddArr.size();
      _ofddArr.resize(id + 1);
      for (unsigned i = origSize; i < _ofddArr.size(); ++i)
         _ofddArr[i] = 0;
   } else if (_ofddArr[id] != 0)
      return false;
   _ofddArr[id] = n;
   return true;
}

OfddNode
OfddMgr::getOfddNode(unsigned id) const
{
   if (id >= _ofddArr.size())
      return size_t(0);
   return _ofddArr[id];
}

bool
OfddMgr::addOfddNode(const string& str, size_t n)
{
   return _ofddMap.insert(OfddMapPair(str, n)).second;
}

void
OfddMgr::forceAddOfddNode(const string& str, size_t n)
{
   _ofddMap[str] = n;
}

OfddNode
OfddMgr::getOfddNode(const string& name) const
{
   OfddMapConstIter bi = _ofddMap.find(name);
   if (bi == _ofddMap.end()) return size_t(0);
   return (*bi).second;
}

int
OfddMgr::evalCube(const OfddNode& node, const string& pattern) const
{
   size_t v = node.getLevel();
   size_t n = pattern.size();
   if (n < v) {
      cerr << "Error: " << pattern << " too short!!" << endl;
      return -1;
   }

   OfddNode next = node;
   for (int i = v - 1; i >= 0; --i) {
      char c = pattern[i];
      if (c == '1')
         next = next.getCof1(i + 1);
      else if (c == '0')
         next = next.getCof0(i + 1);
      else {
         cerr << "Illegal pattern: " << c << "(" << i << ")" << endl;
         return -1;
      }
   }
   return (next == OfddNode::_one) ? 1 : 0;
}

bool
OfddMgr::drawOfdd(const string& name, const string& fileName) const
{
   OfddNode node = ::getOfddNodeByName(name);
   if (node() == 0) {
      cerr << "Error: \"" << name << "\" is not a legal OFDD node!!" << endl;
      return false;
   }

   ofstream ofile(fileName.c_str());
   if (!ofile) {
      cerr << "Error: cannot open file \"" << fileName << "\"!!" << endl;
      return false;
   }

   node.drawOfdd(name, ofile);
   return true;
}
