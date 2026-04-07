# OFDD — Ordered Functional Decision Diagram

A compact C++ library for **ordered functional decision diagrams** using the **positive Davio** decomposition. It follows the same overall design as RicBDD-style BDD packages: a manager (`OfddMgr`), reference-counted nodes (`OfddNode`), a unique table, and apply caches for Boolean operations.

## Decomposition

At an internal node labeled by variable `x`, a Boolean function is represented as

\[
f \;=\; f|_{x=0} \;\oplus\; \bigl(x \wedge (f|_{x=0} \oplus f|_{x=1})\bigr)
\]

- **Left child** — \(f|_{x=0}\) (constant part when `x = 0`).
- **Right child** — \(f|_{x=0} \oplus f|_{x=1}\) (Boolean XOR of the two Shannon cofactors).

**Reduction:** if the right child denotes the constant **0**, the node is removed and the diagram reduces to the left child.

Complemented edges use the same encoding as typical BDD packages: a flag on the pointer flips the represented function (constant **1** / **0** share one terminal node).

## Features

- Unique table keyed by `(low, high, variable index)`.
- Computed caches for **XOR**, **AND**, and **OR** (Davio-based apply).
- `getCof0` / `getCof1` — Shannon cofactors on the OFDD graph (for recursion and `evalCube`).
- `evalCube` — evaluate a function on a bit pattern (same variable order convention as the RicBDD-style tests: pattern index `i` corresponds to support `i+1`).
- Optional Graphviz-style output via `drawOfdd` (DOT).

## Build

Requires a C++ compiler with C++98-era features (as in the original RicBDD teaching code).

```bash
make
./testOfdd
```

`make clean` removes objects and the `testOfdd` binary.

## Layout

| File        | Role                                      |
|------------|-------------------------------------------|
| `ofddNode.h`, `ofddNode.cpp` | Node handles, cofactors, `& \| ^ ~`, print/draw |
| `ofddMgr.h`, `ofddMgr.cpp`   | Manager, `uniquify`, `ofddXor` / `ofddAnd` / `ofddOr`, `evalCube` |
| `myHash.h`   | Open hash + cache templates               |
| `myString.cpp` | `myStr2Int` helper for named nodes     |
| `testOfdd.cpp` | Benchmarks: adder, multiplier, counter, random logic, divider |

## Relation to RicBDD

This package was structured as a parallel to the **RicBDD** teaching BDD code (Shannon expansion + ITE), but uses **positive Davio** instead of Shannon at each node. The test driver intentionally mirrors the RicBDD `testBdd` scenarios so you can compare sizes and behavior.

## License

Use and redistribution are subject to your course or project requirements; add a `LICENSE` file in the repo if you need a standard open-source terms.
