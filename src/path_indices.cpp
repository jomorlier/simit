#include "path_indices.h"

#include <iostream>
#include "path_expressions.h"
#include "graph.h"

namespace simit {
namespace pe {

// class PathIndex
std::ostream &operator<<(std::ostream &os, const PathIndex &pi) {
  if (pi.ptr != nullptr) {
    os << *pi.ptr;
  }
  else {
    os << "empty path idex";
  }
  return os;
}

// class SetEndpointPathIndex
SetEndpointPathIndex::SetEndpointPathIndex(const Set &edgeSet)
    : edgeSet{edgeSet} {
  // TODO: Generalize to support gaps in the future
  iassert(edgeSet.isHomogeneous())
      << "Must be homogeneous because otherwise there are gaps";
}

unsigned SetEndpointPathIndex::numElements() const {
    return edgeSet.getSize();
}

unsigned SetEndpointPathIndex::numNeighbors(const ElementRef &elem) const {
  return edgeSet.getCardinality();
}

unsigned SetEndpointPathIndex::numNeighbors() const {
  return numElements() * edgeSet.getCardinality();
}

class SetEndpointElementIterator : public PathIndexImpl::ElementIterator::Base {
public:
  SetEndpointElementIterator(const Set::ElementIterator &setElemIterator)
      : setElemIterator(setElemIterator) {}

  void operator++() {++setElemIterator;}
  ElementRef& operator*() {return *setElemIterator;}
  Base* clone() const {return new SetEndpointElementIterator(*this);}

protected:
  bool equal(const Base& o) const {
    const SetEndpointElementIterator *other =
        static_cast<const SetEndpointElementIterator*>(&o);
    return setElemIterator == other->setElemIterator;
  }

private:
  Set::ElementIterator setElemIterator;
};

SetEndpointPathIndex::ElementIterator SetEndpointPathIndex::begin() const {
  return new SetEndpointElementIterator(edgeSet.begin());
}

SetEndpointPathIndex::ElementIterator SetEndpointPathIndex::end() const {
  return new SetEndpointElementIterator(edgeSet.end());
}

SetEndpointPathIndex::Neighbors
SetEndpointPathIndex::neighbors(const ElementRef &elem) const {
  class SetEndpointNeighbors : public PathIndexImpl::Neighbors::Base {
    class Iterator : public PathIndexImpl::Neighbors::Iterator::Base {
    public:
      Iterator(const Set::Endpoints::Iterator &epit) : epit(epit) {}

      void operator++() {++epit;}
      ElementRef& operator*() {return *epit;}
      Base* clone() const {return new Iterator(*this);}

    protected:
      bool equal(const Base& o) const {
        const Iterator *other = static_cast<const Iterator*>(&o);
        return epit == other->epit;
      }

    private:
      Set::Endpoints::Iterator epit;
    };

  public:
    SetEndpointNeighbors(const Set::Endpoints &endpoints)
        : endpoints(endpoints) {}
    Neighbors::Iterator begin() const {return new Iterator(endpoints.begin());}
    Neighbors::Iterator end() const {return new Iterator(endpoints.end());}

  private:
    Set::Endpoints endpoints;
  };

  return new SetEndpointNeighbors(edgeSet.getEndpoints(elem));
}

void SetEndpointPathIndex::print(std::ostream &os) const {
  os << "SetEndpointPathIndex:";
  for (auto &e : *this) {
    os << "\n" << "  " << e << ": ";
    for (auto &ep : neighbors(e)) {
      os << ep << " ";
    }
  }
}

// class SegmentedPathIndex
SegmentedPathIndex::ElementIterator SegmentedPathIndex::begin() const {
}

SegmentedPathIndex::ElementIterator SegmentedPathIndex::end() const {
}

SegmentedPathIndex::Neighbors
SegmentedPathIndex::neighbors(const ElementRef &elem) const {
}

void SegmentedPathIndex::print(std::ostream &os) const {
  os << "SegmentedPathIndex:";
  std::cout << std::endl;
  for (size_t i=0; i < numElements()+1; ++i) {
    std::cout << nbrsStart[i] << " ";
  }
  std::cout << std::endl;
  for (size_t i=0; i < numNeighbors(); ++i) {
    std::cout << nbrs[i] << " ";
  }
}


// class PathIndexBuilder
PathIndex PathIndexBuilder::buildSegmented(const PathExpression &pe,
                                     unsigned sourceEndpoint,
                                     std::map<ElementVar,const Set&> bindings) {
  // Check if we have memoized the path index for this path expression, starting
  // at this sourceEndpoint, bound to these sets.
  // TODO

  // Interpret the path expression, starting at sourceEndpoint, over the graph.
  // That is given an element, the find its neighbors through the paths
  // described by the path expression.
  class PathNeighborVisitor : public PathExpressionVisitor {
  public:
    PathNeighborVisitor(const std::map<ElementVar,const Set&> &bindings)
        : bindings(bindings) {}

    PathIndex build(const PathExpression &pe) {
      pe.accept(this);
      PathIndex pit = pi;
      pi = nullptr;
      return pit;
    }

  private:
    /// If it is an EV path expression we return an EndpointPathIndex that wraps
    /// the Edge set.
    void visit(const EV *ev) {
      const Set &edgeSet = bindings.at(ev->getPathEndpoint(0));
      iassert(edgeSet.getCardinality() > 0) << "not an edge set";
      pi = new SetEndpointPathIndex(edgeSet);
    };

    void visit(const VE *ve) {
      const Set &edgeSet = bindings.at(ve->getPathEndpoint(1));
      iassert(edgeSet.getCardinality() > 0) << "not an edge set";
      std::map<ElementRef,std::vector<ElementRef>> neighbors;
      for (auto &e : edgeSet) {
        for (auto &ep : edgeSet.getEndpoints(e)) {
          neighbors[ep].push_back(e);
        }
      }

      unsigned numNeighbors = 0;
      for (auto &p : neighbors) {
        numNeighbors += p.second.size();
      }

      unsigned numElements = neighbors.size();
      unsigned *nbrsStart = new unsigned[numElements + 1];
      unsigned *nbrs = new unsigned[numNeighbors];

      int currElem = 0;
      int currNbrsStart = 0;
      for (auto &p : neighbors) {
        nbrsStart[p.first.getIdent()] = currNbrsStart;
        memcpy(&nbrs[currNbrsStart], p.second.data(),
               p.second.size() * sizeof(unsigned));
        currElem += 1;
        currNbrsStart += p.second.size();
      }
      nbrsStart[numElements] = currNbrsStart;

      pi = new SegmentedPathIndex(numElements, nbrsStart, nbrs);
    }

    void visit(const Predicate *p) {
      pi = new SegmentedPathIndex();
    }

    PathIndex pi;
    const std::map<ElementVar,const Set&> &bindings;
  };

  return PathNeighborVisitor(bindings).build(pe);
}

}}