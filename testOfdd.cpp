/****************************************************************************
  FileName     [ testOfdd.cpp ]
  Synopsis     [ Exercise OFDD (positive Davio) parallel to RicBDD testBdd ]
****************************************************************************/
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "ofddNode.h"
#include "ofddMgr.h"

using namespace std;

OfddMgr om;

static void initOfdd(size_t nSupports, size_t hashSize, size_t cacheSize);

static void buildAdder2(OfddNode& s0, OfddNode& s1, OfddNode& carryOut);
static void buildMultiplier2(OfddNode& p0, OfddNode& p1, OfddNode& p2, OfddNode& p3);
static void buildCounter2(OfddNode& n0, OfddNode& n1);
static void buildRandomLogic(OfddNode& f);
static void buildDivider2(OfddNode& q1, OfddNode& q0, OfddNode& r1, OfddNode& r0);

int
main()
{
   initOfdd(5, 127, 61);

   OfddNode s0, s1, carryOut;
   buildAdder2(s0, s1, carryOut);

   cout << "=== 2-bit ripple-carry adder (OFDD / positive Davio) ===" << endl;
   cout << "Variables: a0 b0 a1 b1 cin" << endl;
   cout << "sum bit 0 (s0):" << endl << s0 << endl;
   cout << "sum bit 1 (s1):" << endl << s1 << endl;
   cout << "carry out (cout):" << endl << carryOut << endl;

   OfddNode p0, p1, p2, p3;
   buildMultiplier2(p0, p1, p2, p3);
   cout << "=== 2-bit multiplier (OFDD) ===" << endl;
   cout << "Variables: a0 b0 a1 b1" << endl;
   cout << "product bit 0 (p0):" << endl << p0 << endl;
   cout << "product bit 1 (p1):" << endl << p1 << endl;
   cout << "product bit 2 (p2):" << endl << p2 << endl;
   cout << "product bit 3 (p3):" << endl << p3 << endl;

   OfddNode n0, n1;
   buildCounter2(n0, n1);
   cout << "=== 2-bit counter next-state (OFDD) ===" << endl;
   cout << "Variables: q0 q1 en rst cin" << endl;
   cout << "next q0 (n0):" << endl << n0 << endl;
   cout << "next q1 (n1):" << endl << n1 << endl;

   OfddNode f;
   buildRandomLogic(f);
   cout << "=== Random logic (OFDD) ===" << endl;
   cout << "Variables: x1 x2 x3 x4 x5" << endl;
   cout << "f:" << endl << f << endl;

   OfddNode q1, q0, r1, r0;
   buildDivider2(q1, q0, r1, r0);
   cout << "=== 2-bit divider (OFDD) ===" << endl;
   cout << "Variables: n0 n1 d0 d1" << endl;
   cout << "quotient bit 1 (q1):" << endl << q1 << endl;
   cout << "quotient bit 0 (q0):" << endl << q0 << endl;
   cout << "remainder bit 1 (r1):" << endl << r1 << endl;
   cout << "remainder bit 0 (r0):" << endl << r0 << endl;

   cout << "\n=== evalCube checks (OFDD vs expected Boolean) ===" << endl;
   const char* patAdd = "10101";
   int evS0 = om.evalCube(s0, patAdd);
   cout << "adder s0 eval " << patAdd << " => " << evS0 << " (expect 0: 1^0^1)" << endl;
   const char* patCnt = "11000";
   int evN0 = om.evalCube(n0, patCnt);
   cout << "counter n0 eval " << patCnt << " => " << evN0
        << " (expect 1: hold en=0 => n0=q0=1)" << endl;

   return 0;
}

static void
initOfdd(size_t nin, size_t h, size_t c)
{
   OfddNode::_debugOfddAddr = true;
   OfddNode::_debugRefCount = true;
   om.init(nin, h, c);
}

static void
buildAdder2(OfddNode& s0, OfddNode& s1, OfddNode& carryOut)
{
   OfddNode a0(om.getSupport(1));
   OfddNode b0(om.getSupport(2));
   OfddNode a1(om.getSupport(3));
   OfddNode b1(om.getSupport(4));
   OfddNode cin(om.getSupport(5));

   OfddNode axb0 = a0 ^ b0;
   s0 = axb0 ^ cin;
   OfddNode c1 = (a0 & b0) | (cin & axb0);

   OfddNode axb1 = a1 ^ b1;
   s1 = axb1 ^ c1;
   carryOut = (a1 & b1) | (c1 & axb1);
}

static void
buildMultiplier2(OfddNode& p0, OfddNode& p1, OfddNode& p2, OfddNode& p3)
{
   OfddNode a0(om.getSupport(1));
   OfddNode b0(om.getSupport(2));
   OfddNode a1(om.getSupport(3));
   OfddNode b1(om.getSupport(4));

   OfddNode pp00 = a0 & b0;
   OfddNode pp01 = a0 & b1;
   OfddNode pp10 = a1 & b0;
   OfddNode pp11 = a1 & b1;

   p0 = pp00;
   p1 = pp01 ^ pp10;
   OfddNode c1 = pp01 & pp10;
   p2 = pp11 ^ c1;
   p3 = pp11 & c1;
}

static void
buildCounter2(OfddNode& n0, OfddNode& n1)
{
   OfddNode q0(om.getSupport(1));
   OfddNode q1(om.getSupport(2));
   OfddNode en(om.getSupport(3));
   OfddNode rst(om.getSupport(4));

   OfddNode incN0 = ~q0;
   OfddNode incN1 = q1 ^ q0;
   OfddNode holdN0 = q0;
   OfddNode holdN1 = q1;

   OfddNode n0NoRst = (en & incN0) | (~en & holdN0);
   OfddNode n1NoRst = (en & incN1) | (~en & holdN1);
   n0 = ~rst & n0NoRst;
   n1 = ~rst & n1NoRst;
}

static void
buildRandomLogic(OfddNode& f)
{
   OfddNode x1(om.getSupport(1));
   OfddNode x2(om.getSupport(2));
   OfddNode x3(om.getSupport(3));
   OfddNode x4(om.getSupport(4));
   OfddNode x5(om.getSupport(5));

   f = (x1 & ~x2) | (x3 ^ x4) | (x5 & (x2 | ~x1));
}

static void
buildDivider2(OfddNode& q1, OfddNode& q0, OfddNode& r1, OfddNode& r0)
{
   OfddNode n0(om.getSupport(1));
   OfddNode n1(om.getSupport(2));
   OfddNode d0(om.getSupport(3));
   OfddNode d1(om.getSupport(4));
   OfddNode vars[4] = { n0, n1, d0, d1 };

   q1 = OfddNode::_zero;
   q0 = OfddNode::_zero;
   r1 = OfddNode::_zero;
   r0 = OfddNode::_zero;

   for (int b0 = 0; b0 <= 1; ++b0) {
      for (int b1 = 0; b1 <= 1; ++b1) {
         for (int b2 = 0; b2 <= 1; ++b2) {
            for (int b3 = 0; b3 <= 1; ++b3) {
               int bits[4] = { b0, b1, b2, b3 };
               int num = (b1 << 1) | b0;
               int den = (b3 << 1) | b2;
               int q = 0;
               int r = num;
               if (den != 0) {
                  q = num / den;
                  r = num % den;
               }

               OfddNode cube = OfddNode::_one;
               for (int i = 0; i < 4; ++i)
                  cube = cube & (bits[i] ? vars[i] : ~vars[i]);

               if ((q >> 1) & 1) q1 = q1 | cube;
               if (q & 1) q0 = q0 | cube;
               if ((r >> 1) & 1) r1 = r1 | cube;
               if (r & 1) r0 = r0 | cube;
            }
         }
      }
   }
}
