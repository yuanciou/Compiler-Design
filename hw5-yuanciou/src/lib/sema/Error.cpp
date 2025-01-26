#include "sema/Error.hpp"

#include <cstddef>
#include <string>
#include <utility>

#include "AST/PType.hpp"
#include "AST/operator.hpp"

Error::Error(Location p_location) : m_location{p_location} {}

Location Error::getLocation() const { return m_location; }

SymbolRedeclarationError::SymbolRedeclarationError(Location p_location,
                                                   std::string p_symbol_name)
    : Error{p_location}, m_symbol_name{std::move(p_symbol_name)} {}

std::string SymbolRedeclarationError::getMessage() const {
  return "symbol '" + m_symbol_name + "' is redeclared";
}

NonPositiveArrayDimensionError::NonPositiveArrayDimensionError(
    Location p_location, std::string p_symbol_name)
    : Error{p_location}, m_symbol_name{std::move(p_symbol_name)} {}

std::string NonPositiveArrayDimensionError::getMessage() const {
  return "'" + m_symbol_name +
         "' declared as an array with an index that is not greater than 0";
}

UndeclaredSymbolError::UndeclaredSymbolError(Location p_location,
                                             std::string p_symbol_name)
    : Error{p_location}, m_symbol_name{std::move(p_symbol_name)} {}

std::string UndeclaredSymbolError::getMessage() const {
  return "use of undeclared symbol '" + m_symbol_name + "'";
}

NonVariableSymbolError::NonVariableSymbolError(Location p_location,
                                               std::string p_symbol_name)
    : Error{p_location}, m_symbol_name{std::move(p_symbol_name)} {}

std::string NonVariableSymbolError::getMessage() const {
  return "use of non-variable symbol '" + m_symbol_name + "'";
}

NonFunctionSymbolError::NonFunctionSymbolError(Location p_location,
                                               std::string p_symbol_name)
    : Error{p_location}, m_symbol_name{std::move(p_symbol_name)} {}

std::string NonFunctionSymbolError::getMessage() const {
  return "call of non-function symbol '" + m_symbol_name + "'";
}

std::string NonIntegerArrayIndexError::getMessage() const {
  return "index of array reference must be an integer";
}

OverArraySubscriptError::OverArraySubscriptError(Location p_location,
                                                 std::string p_symbol_name)
    : Error{p_location}, m_symbol_name{std::move(p_symbol_name)} {}

std::string OverArraySubscriptError::getMessage() const {
  return "there is an over array subscript on '" + m_symbol_name + "'";
}

InvalidBinaryOperandError::InvalidBinaryOperandError(Location p_location,
                                                     Operator p_op,
                                                     const PType *p_lhs,
                                                     const PType *p_rhs)
    : Error{p_location}, m_op{p_op}, m_lhs{p_lhs}, m_rhs{p_rhs} {}

std::string InvalidBinaryOperandError::getMessage() const {
  auto op_name = std::string{kOpString[static_cast<std::size_t>(m_op)]};
  return "invalid operands to binary operator '" + op_name + "' ('" +
         m_lhs->getPTypeCString() + "' and '" + m_rhs->getPTypeCString() + "')";
}

InvalidUnaryOperandError::InvalidUnaryOperandError(Location p_location,
                                                   Operator p_op,
                                                   const PType *p_operand)
    : Error{p_location}, m_op{p_op}, m_operand{p_operand} {}

std::string InvalidUnaryOperandError::getMessage() const {
  auto op_name = std::string{kOpString[static_cast<std::size_t>(m_op)]};
  return "invalid operand to unary operator '" + op_name + "' ('" +
         m_operand->getPTypeCString() + "')";
}

ArgumentNumberMismatchError::ArgumentNumberMismatchError(
    Location p_location, std::string p_function_name)
    : Error{p_location}, m_function_name{std::move(p_function_name)} {}

std::string ArgumentNumberMismatchError::getMessage() const {
  return "too few/much arguments provided for function '" + m_function_name +
         "'";
}

IncompatibleArgumentTypeError::IncompatibleArgumentTypeError(
    Location p_location, const PType *p_expected, const PType *p_actual)
    : Error{p_location},
      m_expected{std::move(p_expected)},
      m_actual{std::move(p_actual)} {}

std::string IncompatibleArgumentTypeError::getMessage() const {
  return "incompatible type passing '" +
         std::string{m_actual->getPTypeCString()} + "' to parameter of type '" +
         std::string{m_expected->getPTypeCString()} + "'";
}

std::string ReadToNonScalarTypeError::getMessage() const {
  return "variable reference of read statement must be scalar type";
}

std::string ReadToConstantOrLoopVarError::getMessage() const {
  return "variable reference of read statement cannot be a constant or loop "
         "variable";
}

std::string PrintOutNonScalarTypeError ::getMessage() const {
  return "expression of print statement must be scalar type";
}

std::string AssignWithArrayTypeError::getMessage() const {
  return "array assignment is not allowed";
}

AssignToConstantError::AssignToConstantError(Location p_location,
                                             std::string p_symbol_name)
    : Error{p_location}, m_symbol_name{std::move(p_symbol_name)} {}

std::string AssignToConstantError::getMessage() const {
  return "cannot assign to variable '" + m_symbol_name +
         "' which is a constant";
}

std::string AssignToLoopVarError::getMessage() const {
  return "the value of loop variable cannot be modified inside the loop body";
}

IncompatibleAssignmentError::IncompatibleAssignmentError(Location p_location,
                                                         const PType *p_lval,
                                                         const PType *p_rval)
    : Error{p_location}, m_lval{p_lval}, m_rval{p_rval} {}

std::string IncompatibleAssignmentError::getMessage() const {
  return "assigning to '" + std::string{m_lval->getPTypeCString()} +
         "' from incompatible type '" + std::string{m_rval->getPTypeCString()} +
         "'";
}

std::string NonBooleanConditionError::getMessage() const {
  return "the expression of condition must be boolean type";
}

std::string NonIncrementalLoopVariableError::getMessage() const {
  return "the lower bound and upper bound of iteration count must be in the "
         "incremental order";
}

std::string ReturnFromVoidError::getMessage() const {
  return "program/procedure should not return a value";
}

IncompatibleReturnTypeError::IncompatibleReturnTypeError(
    Location p_location, const PType *p_expected, const PType *p_actual)
    : Error{p_location}, m_expected{p_expected}, m_actual{p_actual} {}

std::string IncompatibleReturnTypeError::getMessage() const {
  return "return '" + std::string{m_actual->getPTypeCString()} +
         "' from a function with return type '" +
         std::string{m_expected->getPTypeCString()} + "'";
}
