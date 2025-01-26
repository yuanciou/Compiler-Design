#ifndef SEMA_ERROR_HPP
#define SEMA_ERROR_HPP

#include <string>

#include "AST/PType.hpp"
#include "AST/ast.hpp"
#include "AST/operator.hpp"

/// @brief Represents an error that occurs during semantic analysis.
/// The message of each kind of error is defined by concrete Errors.
/// @note This is the abstract base class for all errors.
class Error {
 public:
  Location getLocation() const;
  virtual std::string getMessage() const = 0;

  Error(Location);
  virtual ~Error() = default;

  //
  // Delete copy/move operations to avoid slicing.
  //

  Error(const Error &) = delete;
  Error &operator=(const Error &) = delete;
  Error(Error &&) = delete;
  Error &operator=(Error &&) = delete;

 private:
  Location m_location;
};

//
// Symbol Table
//

class SymbolRedeclarationError : public Error {
 public:
  std::string getMessage() const override;

  SymbolRedeclarationError(Location, std::string p_symbol_name);

 private:
  std::string m_symbol_name;
};

//
// Variable Declaration
//

/// @brief Dimensions in the array declaration should be greater than 0.
class NonPositiveArrayDimensionError : public Error {
 public:
  std::string getMessage() const override;

  NonPositiveArrayDimensionError(Location, std::string p_symbol_name);

 private:
  std::string m_symbol_name;
};

//
// Variable Reference
//

/// @brief The identifier has to be in symbol tables.
class UndeclaredSymbolError : public Error {
 public:
  std::string getMessage() const override;

  UndeclaredSymbolError(Location, std::string p_symbol_name);

 private:
  std::string m_symbol_name;
};

/// @brief The kind of symbol has to be a parameter, variable, loop_var, or
/// constant.
class NonVariableSymbolError : public Error {
 public:
  std::string getMessage() const override;

  NonVariableSymbolError(Location, std::string p_symbol_name);

 private:
  std::string m_symbol_name;
};

/// @brief Each index of an array reference must be of the integer type.
class NonIntegerArrayIndexError : public Error {
 public:
  using Error::Error;

  std::string getMessage() const override;
};

class OverArraySubscriptError : public Error {
 public:
  std::string getMessage() const override;

  OverArraySubscriptError(Location, std::string p_symbol_name);

 private:
  std::string m_symbol_name;
};

//
// Binary/Unary Operator
//

class InvalidBinaryOperandError : public Error {
 public:
  std::string getMessage() const override;

  InvalidBinaryOperandError(Location, Operator, const PType *p_lhs,
                            const PType *p_rhs);

 private:
  Operator m_op;
  const PType *m_lhs;
  const PType *m_rhs;
};

class InvalidUnaryOperandError : public Error {
 public:
  std::string getMessage() const override;

  InvalidUnaryOperandError(Location, Operator, const PType * p_operand);

 private:
  Operator m_op;
  const PType *m_operand;
};

//
// Function Invocation
//

/// @brief The kind of symbol has to be function.
class NonFunctionSymbolError : public Error {
 public:
  std::string getMessage() const override;

  NonFunctionSymbolError(Location, std::string p_symbol_name);

 private:
  std::string m_symbol_name;
};

/// @brief The number of arguments must be the same as one of the parameters.
class ArgumentNumberMismatchError : public Error {
 public:
  std::string getMessage() const override;

  ArgumentNumberMismatchError(Location, std::string p_function_name);

 private:
  std::string m_function_name;
};

/// @brief The type of the result of the expression (argument) must be the same
/// type of the corresponding parameter after appropriate type coercion.
class IncompatibleArgumentTypeError : public Error {
 public:
  std::string getMessage() const override;

  IncompatibleArgumentTypeError(Location, const PType *p_expected,
                                const PType *p_actual);

 private:
  const PType *m_expected;
  const PType *m_actual;
};

//
// Print and Read
//

/// @brief The type of the expression (target) must be scalar type.
class PrintOutNonScalarTypeError : public Error {
 public:
  using Error::Error;

  std::string getMessage() const override;
};

/// @brief The type of the variable reference must be scalar type.
class ReadToNonScalarTypeError : public Error {
 public:
  using Error::Error;

  std::string getMessage() const override;
};

/// @brief The kind of symbol of the variable reference cannot be constant or
/// loop_var.
class ReadToConstantOrLoopVarError : public Error {
 public:
  using Error::Error;

  std::string getMessage() const override;
};

//
// Assignment
//

class AssignWithArrayTypeError : public Error {
 public:
  using Error::Error;

  std::string getMessage() const override;
};

// One might want to use the following aliases to make the error more
// expressive, although they have the same description.

/// @brief The type of the result of the variable reference cannot be an array
/// type.
using AssignToArrayTypeError = AssignWithArrayTypeError;
/// @brief The type of the result of the expression cannot be an array type.
using AssignByArrayTypeError = AssignWithArrayTypeError;

/// @brief The variable reference cannot be a reference to a constant variable.
class AssignToConstantError : public Error {
 public:
  std::string getMessage() const override;

  AssignToConstantError(Location, std::string p_symbol_name);

 private:
  std::string m_symbol_name;
};

/// @brief The variable reference cannot be a reference to a loop variable when
/// the context is within a loop body.
class AssignToLoopVarError : public Error {
 public:
  using Error::Error;

  std::string getMessage() const override;
};

/// @brief The type of the variable reference (lvalue) must be the same as the
/// one of the expression after appropriate type coercion.
class IncompatibleAssignmentError : public Error {
 public:
  std::string getMessage() const override;

  IncompatibleAssignmentError(Location, const PType *p_lval,
                              const PType *p_rval);

 private:
  const PType *m_lval;
  const PType *m_rval;
};

//
// If and While
//

/// @brief The type of the result of the expression (condition) must be boolean
/// type.
class NonBooleanConditionError : public Error {
 public:
  using Error::Error;

  std::string getMessage() const override;
};

//
// For
//

/// @brief The initial value of the loop variable and the constant value of the
/// condition must be in the incremental order.
class NonIncrementalLoopVariableError : public Error {
 public:
  using Error::Error;

  std::string getMessage() const override;
};

//
// Return
//

/// @brief The current context shouldn't be in the program or a procedure since
/// their return type is void.
class ReturnFromVoidError : public Error {
 public:
  using Error::Error;

  std::string getMessage() const override;
};

/// @brief The type of the result of the expression (return value) must be the
/// same type as the return type of current function after appropriate type
/// coercion.
class IncompatibleReturnTypeError : public Error {
 public:
  std::string getMessage() const override;

  IncompatibleReturnTypeError(Location, const PType *p_expected,
                              const PType *p_actual);

 private:
  const PType *m_expected;
  const PType *m_actual;
};

#endif  // SEMA_ERROR_HPP
