/****************************************************************************
  FileName     [ ofddNode.cpp ]
  PackageName  [ OFDD ]
  Synopsis     [ OFDD node operations and Davio cofactors ]
****************************************************************************/

#include <sstream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <string>
#include "ofddNode.h"
#include "ofddMgr.h"

OfddMgr* OfddNode::_OfddMgr = 0;
OfddNodeInt* OfddNodeInt::_terminal = 0;
OfddNode OfddNode::_one;
OfddNode OfddNode::_zero;
bool OfddNode::_debugOfddAddr = false;
bool OfddNode::_debugRefCount = false;

OfddNode::OfddNode(size_t l, size_t r, size_t i, OFDD_EDGE_FLAG f)
{
   OfddNodeInt* n = _OfddMgr->uniquify(l, r, i);
   assert(n != 0);
   _node = size_t(n) + f;
   n->incRefCount();
}

OfddNode::OfddNode(const OfddNode& n) : _node(n._node)
{
   OfddNodeInt* t = getOfddNodeInt();
   if (t)
      t->incRefCount();
}

OfddNode::OfddNode(OfddNodeInt* n, OFDD_EDGE_FLAG f)
{
   assert(n != 0);
   _node = size_t(n) + f;
   n->incRefCount();
}

OfddNode::OfddNode(size_t v) : _node(v)
{
   OfddNodeInt* n = getOfddNodeInt();
   if (n)
      n->incRefCount();
}

OfddNode::~OfddNode()
{
   OfddNodeInt* n = getOfddNodeInt();
   if (n)
      n->decRefCount();
}

const OfddNode&
OfddNode::getLeft() const
{
   assert(getOfddNodeInt() != 0);
   return getOfddNodeInt()->getLeft();
}

const OfddNode&
OfddNode::getRight() const
{
   assert(getOfddNodeInt() != 0);
   return getOfddNodeInt()->getRight();
}

OfddNode
OfddNode::getCof0(unsigned i) const
{
   assert(getOfddNodeInt() != 0);
   assert(i > 0);
   if (i > getLevel()) return (*this);
   if (i == getLevel())
      return isNegEdge()? ~getLeft() : getLeft();
   OfddNode t = getLeft().getCof0(i);
   OfddNode e = getRight().getCof0(i);
   if (t == e) return isNegEdge()? ~t : t;
   OFDD_EDGE_FLAG f = (isNegEdge() ^ t.isNegEdge())?
                      OFDD_NEG_EDGE: OFDD_POS_EDGE;
   if (t.isNegEdge()) { t = ~t; e = ~e; }
   return OfddNode(t(), e(), getLevel(), f);
}

OfddNode
OfddNode::getCof1(unsigned i) const
{
   assert(getOfddNodeInt() != 0);
   assert(i > 0);
   if (i > getLevel()) return (*this);
   if (i == getLevel()) {
      OfddNode l = isNegEdge()? ~getLeft() : getLeft();
      OfddNode r = isNegEdge()? ~getRight() : getRight();
      return _OfddMgr->ofddXor(l, r);
   }
   OfddNode t = getLeft().getCof1(i);
   OfddNode e = getRight().getCof1(i);
   if (t == e) return isNegEdge()? ~t : t;
   OFDD_EDGE_FLAG f = (isNegEdge() ^ t.isNegEdge())?
                      OFDD_NEG_EDGE: OFDD_POS_EDGE;
   if (t.isNegEdge()) { t = ~t; e = ~e; }
   return OfddNode(t(), e(), getLevel(), f);
}

unsigned
OfddNode::getLevel() const
{
   return getOfddNodeInt()->getLevel();
}

unsigned
OfddNode::getRefCount() const
{
   return getOfddNodeInt()->getRefCount();
}

OfddNode&
OfddNode::operator = (const OfddNode& n)
{
   OfddNodeInt* t = getOfddNodeInt();
   if (t)
      t->decRefCount();
   _node = n._node;
   t = getOfddNodeInt();
   if (t)
      t->incRefCount();
   return (*this);
}

OfddNode
OfddNode::operator & (const OfddNode& n) const
{
   return _OfddMgr->ofddAnd((*this), n);
}

OfddNode&
OfddNode::operator &= (const OfddNode& n)
{
   (*this) = (*this) & n;
   return (*this);
}

OfddNode
OfddNode::operator | (const OfddNode& n) const
{
   return _OfddMgr->ofddOr((*this), n);
}

OfddNode&
OfddNode::operator |= (const OfddNode& n)
{
   (*this) = (*this) | n;
   return (*this);
}

OfddNode
OfddNode::operator ^ (const OfddNode& n) const
{
   return _OfddMgr->ofddXor((*this), n);
}

OfddNode&
OfddNode::operator ^= (const OfddNode& n)
{
   (*this) = (*this) ^ n;
   return (*this);
}

bool
OfddNode::operator < (const OfddNode& n) const
{
   unsigned l1 = getLevel();
   unsigned l2 = n.getLevel();
   return ((l1 < l2) ||
          ((l1 == l2) && (_node < n._node)));
}

bool
OfddNode::operator <= (const OfddNode& n) const
{
   unsigned l1 = getLevel();
   unsigned l2 = n.getLevel();
   return ((l1 < l2) ||
          ((l1 == l2) && (_node <= n._node)));
}

bool
OfddNode::isTerminal() const
{
   return (getOfddNodeInt() == OfddNodeInt::_terminal);
}

string
OfddNode::toString() const
{
   ostringstream os;
   os << "<OFDD level=" << getLevel() << ">";
   return os.str();
}

ostream&
operator << (ostream& os, const OfddNode& n)
{
   size_t nNodes = 0;
   n.print(os, 0, nNodes);
   n.unsetVisitedRecur();
   os << endl << endl << "==> Total #OfddNodes : " << nNodes << endl;
   return os;
}

void
OfddNode::print(ostream& os, size_t indent, size_t& nNodes) const
{
   for (size_t i = 0; i < indent; ++i)
      os << ' ';
   OfddNodeInt* n = getOfddNodeInt();
   os << '[' << getLevel() << "](" << (isNegEdge()? '-' : '+') << ") ";
   if (_debugOfddAddr)
      os << n << " ";
   if (_debugRefCount)
      os << "(" << n->getRefCount() << ")";
   if (n->isVisited()) {
      os << " (*)";
      return;
   }
   else ++nNodes;
   n->setVisited();
   if (!isTerminal()) {
      os << endl;
      os << string(indent + 2, ' ') << "f|0:" << endl;
      n->getLeft().print(os, indent + 4, nNodes);
      os << endl;
      os << string(indent + 2, ' ') << "f|0^f|1:" << endl;
      n->getRight().print(os, indent + 4, nNodes);
   }
}

void
OfddNode::unsetVisitedRecur() const
{
   OfddNodeInt* n = getOfddNodeInt();
   if (!n->isVisited()) return;
   n->unsetVisited();
   if (!isTerminal()) {
      n->getLeft().unsetVisitedRecur();
      n->getRight().unsetVisitedRecur();
   }
}

void
OfddNode::drawOfdd(const string& name, ofstream& ofile) const
{
   ofile << "digraph {" << endl;
   ofile << "   node [shape = plaintext];" << endl;
   ofile << "   ";
   for (unsigned l = getLevel(); l > 0; --l)
      ofile << l << " -> ";
   ofile << "0 [style = invis];" << endl;
   ofile << "   { rank = source; \"" << name << "\"; }" << endl;
   ofile << "   node [shape = ellipse];" << endl;
   ofile << "   \"" << name << "\" -> \"" << getLabel()
         << "\" [color = blue]";
   ofile << (isNegEdge()? " [arrowhead = odot]" : ";") << endl;

   drawOfddRecur(ofile);

   ofile << "   { rank = same; 0; \"One\"; }" << endl;
   ofile << "}" << endl;

   unsetVisitedRecur();
}

void
OfddNode::drawOfddRecur(ofstream& ofile) const
{
   OfddNodeInt* n = getOfddNodeInt();
   if (n->isVisited())
      return;
   n->setVisited();
   if (isTerminal())
      return;
   OfddNode left = getLeft();
   OfddNode right = getRight();

   ofile << "   { rank = same; " << getLevel() << "; \"" << getLabel()
         << "\"; }\n";

   ofile << "   \"" << getLabel() << "\" -> \"" << left.getLabel()
         << "\" [label=\"f|0\"]";
   ofile << ((left.isNegEdge())? " [arrowhead=odot]" : ";") << endl;

   ofile << "   \"" << getLabel() << "\" -> \"" << right.getLabel()
         << "\" [style=dotted,color=red,label=\"f|0^f|1\"]";
   ofile << ((right.isNegEdge())? " [arrowhead=odot]" : ";") << endl;

   left.drawOfddRecur(ofile);
   right.drawOfddRecur(ofile);
}

string
OfddNode::getLabel() const
{
   if (isTerminal())
      return "One";

   ostringstream str;
   str << getOfddNodeInt();

   return str.str();
}
