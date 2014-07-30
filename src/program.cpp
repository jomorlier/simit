#include "program.h"

#include <memory>

#include "ir.h"
#include "frontend.h"
#include "codegen.h"
#include "logger.h"
#include "util.h"
#include "errors.h"

using namespace simit;
using namespace std;

namespace simit {
namespace internal {
class Program {
 public:
  Program(const std::string &name)
      : name(name), frontend(new Frontend()), codegen(new LLVMCodeGen()) {}
  const std::string &name;
  std::unique_ptr<Frontend> frontend;
  std::unique_ptr<LLVMCodeGen> codegen;
  std::list<std::shared_ptr<IRNode>> programNodes;
  std::list<simit::Error> errors;
  std::list<simit::Test> tests;
};
}
}

/* Program */
Program::Program(const std::string &name) : impl(new internal::Program(name)) {
}

Program::~Program() {
  delete impl;
}

std::string Program::getName() const {
  return impl->name;
}

int Program::loadString(const string &programString) {
  int errorCode = impl->frontend->parseString(programString, impl->programNodes,
                                              impl->errors, impl->tests);
  return errorCode;
}

int Program::loadFile(const std::string &filename) {
  int errorCode = impl->frontend->parseFile(filename, impl->programNodes,
                                            impl->errors, impl->tests);
  return errorCode;
}

int Program::compile() {
//  cout << (*this) << endl;
  return 0;
}

int Program::run() {
  return 0;
}

string Program::getErrorString() {
  return util::join(impl->errors, "\n");
}

std::list<Error> &Program::getErrors() {
  return impl->errors;
}

std::ostream &simit::operator<<(std::ostream &os, const Program &program) {
  os << util::join(program.impl->programNodes, "\n\n");
  return os;
}
