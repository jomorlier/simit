#include "stencils.h"

#include "ir.h"
#include <iostream>

using namespace std;

namespace simit {
namespace ir {

std::string StencilLayout::getStencilFunc() const {
  return ptr->assemblyFunc;
}

std::string StencilLayout::getStencilVar() const {
  return ptr->targetVar;
}

map<vector<int>, int> StencilLayout::getLayout() const {
  return ptr->layout;
}

map<int, vector<int>> StencilLayout::getLayoutReversed() const {
  map<vector<int>, int> &layout = ptr->layout;
  map<int, vector<int>> reversed;
  for (auto &kv : layout) {
    reversed[kv.second] = kv.first;
  }
  return reversed;
}

bool StencilLayout::hasGridSet() const {
  return ptr->gridSet.defined();
}

Var StencilLayout::getGridSet() const {
  simit_iassert(ptr->gridSet.defined());
  return ptr->gridSet;
}

std::ostream& operator<<(std::ostream& os, const StencilLayout& stencil) {
  os << "stencil";
  if (stencil.hasGridSet()) {
    os << "(" << stencil.getGridSet() << ")";
  }
  os << endl;

  if (stencil.defined()) {
    for (auto &kv : stencil.getLayout()) {
      os << "\t";
      bool first = true;
      for (int off : kv.first) {
        if (!first) os << ",";
        first = false;
        os << off;
      }
      os << ": " << kv.second << endl;
    }
  }
  return os;
}

vector<int> getOffsets(vector<Expr> offsets) {
  vector<int> out;
  for (Expr off : offsets) {
    simit_iassert(isa<Literal>(off));
    out.push_back(to<Literal>(off)->getIntVal(0));
  }
  return out;
}

StencilContent* buildStencil(Func kernel, Var stencilVar, Var gridSet) {
  std::map<vector<int>, int> layout; // layout of stencil for the storage
  int stencilSize = 0;
  Var tensorVar;
  match(kernel,
        function< void(const TensorWrite*,Matcher*) >(
            [&](const TensorWrite* op, Matcher* ctx) {
              tensorVar = Var();
              ctx->match(op->tensor);
              // Found a write to the stencil-assembled variable
              if (tensorVar.defined() && tensorVar == stencilVar) {
                simit_tassert(op->indices.size() == 2)
                    << "Stencil assembly must be of matrix";
                auto row = op->indices[0];
                auto col = op->indices[1];
                simit_iassert(row.type().isElement() &&
                        col.type().isElement());
                simit_iassert(kernel.getArguments().size() >= 2)
                    << "Kernel must have element and grid edge set "
                    << "as arguments";
                // The first argument to the kernel is an alias for points[0,0,...]
                Var origin = kernel.getArguments()[0];
                Var edges = kernel.getArguments()[kernel.getArguments().size()-1];
                simit_iassert(edges.getType().isGridSet());
                int dims = edges.getType().toGridSet()->dimensions;
                // We assume row index normalization has been performed already
                simit_iassert((isa<VarExpr>(row)
                               && to<VarExpr>(row)->var == origin)
                              || (isa<SetRead>(row) && util::isAllZeros(
                                  getOffsets(to<SetRead>(row)->indices))));
                // col index determines stencil structure
                vector<int> offsets;
                if (isa<VarExpr>(col)) {
                  simit_iassert(to<VarExpr>(col)->var == origin);
                  offsets = vector<int>(dims);
                }
                else {
                  simit_iassert(isa<SetRead>(col));
                  offsets = getOffsets(to<SetRead>(col)->indices);
                }
                // Add new offset to stencil
                if (!layout.count(offsets)) {
                  layout[offsets] = stencilSize;
                  stencilSize++;
                }
              }
            }),
        function< void(const VarExpr*) >(
            [&tensorVar](const VarExpr* v) {
              tensorVar = v->var;
            })
        );
  StencilContent *content = new StencilContent;
  content->layout = layout;
  content->gridSet = gridSet;
  return content;
}

}} // namespace simit::ir
