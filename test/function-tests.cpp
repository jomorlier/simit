#include "simit-test.h"

#include "tensor.h"
#include "graph.h"
#include "ir.h"
#include "lower/index_expressions/lower_scatter_workspace.h"

using namespace simit::ir;

TEST(Function, bindSet) {
  Type vertexType = ElementType::make("vertex", {Field("field", Int)});
  Type vertexSetType = SetType::make(vertexType, {});
  Var V("V", vertexSetType);
  Var i("i", Int);
  Stmt neg =
      ForRange::make(i, 0, Length::make(IndexSet(V)),
                     Store::make(FieldRead::make(V, "field"), i,
                                 -Load::make(FieldRead::make(V, "field"), i)));

  // Create environment and compile
  Environment env;
  env.addExtern(V);
  simit::Function function = getTestBackend()->compile(neg, env);

  // Create and bind arguments
  simit::Set VArg;
  auto field = VArg.addField<int>("field");
  simit::ElementRef p0 = VArg.add();
  simit::ElementRef p1 = VArg.add();
  simit::ElementRef p2 = VArg.add();
  field(p0) = 42;
  field(p1) = 43;
  field(p2) = 44;
  function.bind("V", &VArg);

  // Run and check output
  function.runSafe();
  SIMIT_ASSERT_FLOAT_EQ(-42, field(p0));
  SIMIT_ASSERT_FLOAT_EQ(-43, field(p1));
  SIMIT_ASSERT_FLOAT_EQ(-44, field(p2));
}

TEST(Function, bindScalar) {
  Var a("a", Int);
  Var b("b", Int);
  Stmt neg = AssignStmt::make(a, -b);

  // Create environment and compile
  Environment env;
  env.addExtern(a);
  env.addExtern(b);
  simit::Function function = getTestBackend()->compile(neg, env);

  // Create and bind arguments
  simit::Tensor<int> aArg = 0;
  simit::Tensor<int> bArg = 42;
  function.bind("a", &aArg);
  function.bind("b", &bArg);

  // Run and check output
  function.runSafe();
  ASSERT_EQ(-42, aArg);
  ASSERT_EQ(42, bArg);
}

TEST(Function, bindVector) {
  Var a("a", Vec3i);
  Var b("b", Vec3i);

  Var i("i", Int);
  Stmt neg = ForRange::make(i, 0, 3, Store::make(a, i, -Load::make(b, i)));

  // Create environment and compile
  Environment env;
  env.addExtern(a);
  env.addExtern(b);
  simit::Function function = getTestBackend()->compile(neg, env);

  // Create and bind arguments
  simit::Tensor<int,3> aArg = {0, 0, 0};
  simit::Tensor<int,3> bArg = {42, 43, 44};
  function.bind("a", &aArg);
  function.bind("b", &bArg);

  // Run and check output
  function.runSafe();
  simit::Tensor<int,3> aExpected = {-42, -43, -44};
  simit::Tensor<int,3> bExpected = { 42,  43,  44};
  ASSERT_EQ(aExpected, aArg);
  ASSERT_EQ(bExpected, bArg);
}

TEST(DISABLED_Function, bindSparseTensor) {
  Type vertexType = ElementType::make("vertex", {});
  Type vertexSetType = SetType::make(vertexType, {});
  Var V("V", vertexSetType);

  IndexDomain dim({V});
  IndexVar i("i", dim);
  IndexVar j("j", dim);

  Type tensorType = TensorType::make(ScalarType::Float, {dim,dim});
  Var A("A", tensorType);
  Expr negExpr = IndexExpr::make({i,j}, -VarExpr::make(A)(i,j));

  // Create environment and compile
  Environment env;
  env.addExtern(V);
  env.addExtern(A);
  Stmt neg = lowerScatterWorkspace(A, to<IndexExpr>(negExpr), &env);
  std::cout << neg << std::endl << std::endl;
  simit::Function function = getTestBackend()->compile(neg, env);

  // Create and bind arguments
  simit::Set Varg;
  Varg.add();
  Varg.add();
  Varg.add();
  function.bind("V", &Varg);

  // 1.0 2.0 0.0
  // 0.0 3.0 4.0
  // 0.0 0.0 0.0
  int A_row_ptr[4] = {0, 2, 4, 4};
  int A_col_ind[4] = {0, 1, 1, 2};
  double A_vals[4] = {1.0, 2.0, 3.0, 4.0};
  std::cout << "A: ";
  for (int i=0; i < 4; ++i) {
    std::cout << A_vals[i] << ",";
  }
  std::cout << std::endl;
  function.bind("A", A_row_ptr, A_col_ind, A_vals);


  // Run and check output
  function.runSafe();

  std::cout << "A:  " << std::endl;
  for (int i=0; i < 4; ++i) {
    std::cout << A_vals[i] << ",";
  }
  std::cout << std::endl;
}