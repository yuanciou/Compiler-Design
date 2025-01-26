#include "AST/PType.hpp"
#include "sema/Error.hpp"
#include "sema/ErrorPrinter.hpp"
#include "sema/SemanticAnalyzer.hpp"
#include "visitor/AstNodeInclude.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <set>
#include <stack>
#include <vector>

//
// There is only one semantic analysis that should be performed in pre-order,
// that is symbol redeclaration. Except for symbol redeclaration, all other
// semantic analyses are performed in post-order.
//
// For each node:
// 1. Push a new symbol table if this node forms a scope.
// 2. Insert the symbol into current symbol table if this node is related to
//    declaration (ProgramNode, VariableNode, FunctionNode).
// 3. Traverse child nodes of this node.
// 4. Perform semantic analyses of this node.
// 5. Pop the symbol table pushed at the 1st step and record it for later phases
//    to retain the information.
//
// If encountering an error in the child nodes of an expression, the type of the
// parent node is set to error type to propagate the error.
//

void SemanticAnalyzer::visit(ProgramNode &p_program) {
    m_symbol_manager.pushScope();
    m_context_stack.push(SemanticContext::kGlobal);
    m_returned_type_stack.push(p_program.getTypePtr());

    auto *entry = m_symbol_manager.addSymbol(
        p_program.getName(), SymbolEntry::KindEnum::kProgramKind,
        p_program.getTypePtr(), static_cast<Constant *>(nullptr));
    if (!entry) {
        printError(SymbolRedeclarationError(p_program.getLocation(),
                                            p_program.getNameCString()));
    }

    p_program.visitChildNodes(*this);

    m_returned_type_stack.pop();
    m_context_stack.pop();
    m_symbol_table_of_scoping_nodes[&p_program] = m_symbol_manager.popScope();
}

void SemanticAnalyzer::visit(DeclNode &p_decl) {
    p_decl.visitChildNodes(*this);
}

SymbolEntry::KindEnum
SemanticAnalyzer::determineVarKind(const VariableNode &p_variable) const {
    if (m_context_stack.top() == SemanticContext::kForLoop) {
        return SymbolEntry::KindEnum::kLoopVarKind;
    }

    if (m_context_stack.top() == SemanticContext::kFunction) {
        return SymbolEntry::KindEnum::kParameterKind;
    }

    // global or local
    return p_variable.getConstantPtr() ? SymbolEntry::KindEnum::kConstantKind
                                       : SymbolEntry::KindEnum::kVariableKind;
}

// NOTE: An anonymous namespace is used to define a function that is only
// visible in the current translation unit, which is similar to the static
// keyword in C.
namespace {
bool hasNonPositiveDimension(const PType *p_type) {
    return std::any_of(p_type->getDimensions().begin(),
                       p_type->getDimensions().end(),
                       [](const int dim) { return dim <= 0; });
}
}  // namespace

bool SemanticAnalyzer::isShadowingLoopVar(const std::string &p_name) const {
    auto to_be_shadowed = m_symbol_manager.lookup(p_name);
    return to_be_shadowed &&
           to_be_shadowed->getKind() == SymbolEntry::KindEnum::kLoopVarKind;
}

bool SemanticAnalyzer::isRedeclaringSymbol(const std::string &p_name) const {
    return m_symbol_manager.getCurrentTable()->lookup(p_name);
}

void SemanticAnalyzer::visit(VariableNode &p_variable) {
    SymbolEntry *entry = nullptr;
    if (isShadowingLoopVar(p_variable.getName()) ||
        isRedeclaringSymbol(p_variable.getName())) {
        printError(SymbolRedeclarationError(p_variable.getLocation(),
                                            p_variable.getNameCString()));
    } else {
        entry = m_symbol_manager.addSymbol(
            p_variable.getName(), determineVarKind(p_variable),
            p_variable.getTypePtr(), p_variable.getConstantPtr());
        assert(entry);
    }

    p_variable.visitChildNodes(*this);

    // The size of an array should be positive. Notice that size error doesn't
    // stop the array from being added to the symbol table; however, the symbol is
    // ill-formed.
    if (entry && hasNonPositiveDimension(p_variable.getTypePtr())) {
        m_error_entry_set.insert(entry);
        printError(NonPositiveArrayDimensionError(
            p_variable.getLocation(), p_variable.getNameCString()));
    }
}

void SemanticAnalyzer::visit(ConstantValueNode &p_constant_value) {
    p_constant_value.setInferredType(
        p_constant_value.getTypePtr()->getStructElementType(0));
}

void SemanticAnalyzer::visit(FunctionNode &p_function) {
    if (isShadowingLoopVar(p_function.getName()) ||
        isRedeclaringSymbol(p_function.getName())) {
        printError(SymbolRedeclarationError(p_function.getLocation(),
                                            p_function.getNameCString()));
    } else {
        auto *entry = m_symbol_manager.addSymbol(
            p_function.getName(), SymbolEntry::KindEnum::kFunctionKind,
            p_function.getTypePtr(), &p_function.getParameters());
        assert(entry);
    }

    m_symbol_manager.pushScope();
    m_context_stack.push(SemanticContext::kFunction);
    m_returned_type_stack.push(p_function.getTypePtr());

    for (const auto &parameter : p_function.getParameters()) {
        parameter->accept(*this);
    }

    // directly visit the body to prevent pushing duplicate scope
    m_context_stack.push(SemanticContext::kLocal);
    p_function.visitBodyChildNodes(*this);
    m_context_stack.pop();

    m_returned_type_stack.pop();
    m_context_stack.pop();
    m_symbol_table_of_scoping_nodes[&p_function] = m_symbol_manager.popScope();
}

void SemanticAnalyzer::visit(CompoundStatementNode &p_compound_statement) {
    m_symbol_manager.pushScope();
    m_context_stack.push(SemanticContext::kLocal);

    p_compound_statement.visitChildNodes(*this);

    m_context_stack.pop();
    m_symbol_table_of_scoping_nodes[&p_compound_statement] =
        m_symbol_manager.popScope();
}


void SemanticAnalyzer::visit(PrintNode &p_print) {
    p_print.visitChildNodes(*this);

    if (p_print.getTarget().getInferredType()->isError()) {
        return;
    }
    if (!p_print.getTarget().getInferredType()->isScalar()) {
        printError(PrintOutNonScalarTypeError(
            p_print.getTarget().getLocation()));
    }
}

namespace {
bool validateBinaryOperands(const BinaryOperatorNode &p_bin_op) {
    const auto *left_type = p_bin_op.getLeftOperand().getInferredType();
    const auto *right_type = p_bin_op.getRightOperand().getInferredType();

    switch (p_bin_op.getOp()) {
    case Operator::kPlusOp:
        if (left_type->isString() && right_type->isString()) {
            return true;
        }
        [[fallthrough]];
    case Operator::kMinusOp:
    case Operator::kMultiplyOp:
    case Operator::kDivideOp:
        if ((left_type->isInteger() || left_type->isReal()) &&
            (right_type->isInteger() || right_type->isReal())) {
            return true;
        }
        break;
    case Operator::kModOp:
        if (left_type->isInteger() && right_type->isInteger()) {
            return true;
        }
        break;
    case Operator::kAndOp:
    case Operator::kOrOp:
        if (left_type->isBool() && right_type->isBool()) {
            return true;
        }
        break;
    case Operator::kLessOp:
    case Operator::kLessOrEqualOp:
    case Operator::kEqualOp:
    case Operator::kGreaterOp:
    case Operator::kGreaterOrEqualOp:
    case Operator::kNotEqualOp:
        if ((left_type->isInteger() || left_type->isReal()) &&
            (right_type->isInteger() || right_type->isReal())) {
            return true;
        }
        break;
    default:
        assert(false && "unknown binary op or unary op");
    }

    return false;
}

void setBinaryOpInferredType(BinaryOperatorNode &p_bin_op) {
    const auto *left_type = p_bin_op.getLeftOperand().getInferredType();
    const auto *right_type = p_bin_op.getRightOperand().getInferredType();

    switch (p_bin_op.getOp()) {
    case Operator::kPlusOp:
        if (left_type->isString() && right_type->isString()) {
            p_bin_op.setInferredType(
                new PType(PType::PrimitiveTypeEnum::kStringType));
            return;
        }
        [[fallthrough]];
    case Operator::kMinusOp:
    case Operator::kMultiplyOp:
    case Operator::kDivideOp:
        if (left_type->isReal() || right_type->isReal()) {
            p_bin_op.setInferredType(
                new PType(PType::PrimitiveTypeEnum::kRealType));
            return;
        }
    case Operator::kModOp:
        p_bin_op.setInferredType(
            new PType(PType::PrimitiveTypeEnum::kIntegerType));
        return;
    case Operator::kAndOp:
    case Operator::kOrOp:
        p_bin_op.setInferredType(
            new PType(PType::PrimitiveTypeEnum::kBoolType));
        return;
    case Operator::kLessOp:
    case Operator::kLessOrEqualOp:
    case Operator::kEqualOp:
    case Operator::kGreaterOp:
    case Operator::kGreaterOrEqualOp:
    case Operator::kNotEqualOp:
        p_bin_op.setInferredType(
            new PType(PType::PrimitiveTypeEnum::kBoolType));
        return;
    default:
        assert(false && "unknown binary op or unary op");
    }
}
} // namespace

void SemanticAnalyzer::visit(BinaryOperatorNode &p_bin_op) {
    p_bin_op.visitChildNodes(*this);

    if (p_bin_op.getLeftOperand().getInferredType()->isError() ||
        p_bin_op.getRightOperand().getInferredType()->isError()) {
        // Propagate the error type.
        // NOTE: Although for operations other than arithmetic operations that has
        // fixed result type, we can set the type to the expected one, this compiler
        // handles errors with propagation.
        p_bin_op.setInferredType(new PType(PType::PrimitiveTypeEnum::kErrorType));
        return;
    }

    if (!validateBinaryOperands(p_bin_op)) {
        printErrorAndSetType(InvalidBinaryOperandError(
                               p_bin_op.getLocation(), p_bin_op.getOp(),
                               p_bin_op.getLeftOperand().getInferredType(),
                               p_bin_op.getRightOperand().getInferredType()),
                             p_bin_op);
        return;
    }

    setBinaryOpInferredType(p_bin_op);
}

namespace {
bool validateUnaryOperand(const UnaryOperatorNode &p_un_op) {
    const auto *const operand_type = p_un_op.getOperand().getInferredType();
    switch (p_un_op.getOp()) {
    case Operator::kNegOp:
        if (operand_type->isInteger() || operand_type->isReal()) {
            return true;
        }
        break;
    case Operator::kNotOp:
        if (operand_type->isBool()) {
            return true;
        }
        break;
    default:
        assert(false && "unknown binary op or unary op");
    }

    return false;
}

void setUnaryOpInferredType(UnaryOperatorNode &p_un_op) {
    switch (p_un_op.getOp()) {
    case Operator::kNegOp:
        p_un_op.setInferredType(new PType(
            p_un_op.getOperand().getInferredType()->getPrimitiveType()));
        return;
    case Operator::kNotOp:
        p_un_op.setInferredType(new PType(PType::PrimitiveTypeEnum::kBoolType));
        return;
    default:
        assert(false && "unknown binary op or unary op");
    }
}
} // namespace

void SemanticAnalyzer::visit(UnaryOperatorNode &p_un_op) {
    p_un_op.visitChildNodes(*this);

    if (p_un_op.getOperand().getInferredType()->isError()) {
        // Propagate the error type.
        // NOTE: Although for the not operator, we can set the type to boolean, this
        // compiler handles errors with propagation.
        p_un_op.setInferredType(new PType(PType::PrimitiveTypeEnum::kErrorType));
        return;
    }

    if (!validateUnaryOperand(p_un_op)) {
        printErrorAndSetType(
            InvalidUnaryOperandError(p_un_op.getLocation(), p_un_op.getOp(),
                                    p_un_op.getOperand().getInferredType()),
            p_un_op);
        return;
    }

    setUnaryOpInferredType(p_un_op);
}

bool SemanticAnalyzer::analyzeArgumentTypes(
        const FunctionNode::DeclNodes &p_parameters,
        const FunctionInvocationNode::ExprNodes &p_arguments) {
    FunctionInvocationNode::ExprNodes::const_iterator argument_iter =
        p_arguments.begin();

    for (const auto &parameter : p_parameters) {
        const auto &variables = parameter->getVariables();
        for (const auto &variable : variables) {
            auto *expr_type = (*argument_iter)->getInferredType();
            if (expr_type->isError()) {
                return false;
            }

            if (!expr_type->canCoerceTo(variable->getTypePtr())) {
                printError(IncompatibleArgumentTypeError(
                    (*argument_iter)->getLocation(), variable->getTypePtr(),
                    expr_type));
                return false;
            }

            argument_iter++;
        }
    }

    return true;
}

void SemanticAnalyzer::visit(FunctionInvocationNode &p_func_invocation) {
    p_func_invocation.visitChildNodes(*this);

    const SymbolEntry *entry = m_symbol_manager.lookup(p_func_invocation.getName());
    if (entry && m_error_entry_set.find(const_cast<SymbolEntry *>(entry)) !=
        m_error_entry_set.end()) {
        p_func_invocation.setInferredType(
            new PType(PType::PrimitiveTypeEnum::kErrorType));
        return;
    }
    // 1. The identifier has to be in symbol tables.
    if (!entry) {
        printErrorAndSetType(
            UndeclaredSymbolError(p_func_invocation.getLocation(),
                                    p_func_invocation.getNameCString()),
            p_func_invocation);
        return;
    }
    // 2. The kind of symbol has to be function.
    if (entry->getKind() != SymbolEntry::KindEnum::kFunctionKind) {
        printErrorAndSetType(
            NonFunctionSymbolError(p_func_invocation.getLocation(),
                                    p_func_invocation.getNameCString()),
            p_func_invocation);
        return;
    }

    // NOTE: Although we don't have to propagate the error type when the following
    // checks fail since we know the expected type, this compiler handles errors
    // with propagation.

    // 3. The number of arguments must be the same as one of the parameters.
    const auto& parameters = *entry->getAttribute().parameters();
    const auto& arguments = p_func_invocation.getArguments();
    if (arguments.size() != FunctionNode::getParametersNum(parameters)) {
        printErrorAndSetType(
            ArgumentNumberMismatchError(p_func_invocation.getLocation(),
                                        p_func_invocation.getNameCString()),
            p_func_invocation);
        return;
    }
    // 4. (if no above violations) The type of the result of the expression
    // (argument) must be the same type of the corresponding parameter after
    // appropriate type coercion.
    if (!analyzeArgumentTypes(parameters, arguments)) {
        p_func_invocation.setInferredType(
            new PType(PType::PrimitiveTypeEnum::kErrorType));
        return;
    }

    p_func_invocation.setInferredType(
        new PType(entry->getTypePtr()->getPrimitiveType()));
}

void SemanticAnalyzer::visit(VariableReferenceNode &p_variable_ref) {
    p_variable_ref.visitChildNodes(*this);

    const SymbolEntry *entry = m_symbol_manager.lookup(p_variable_ref.getName());
    if (entry && m_error_entry_set.find(const_cast<SymbolEntry *>(entry)) !=
        m_error_entry_set.end()) {
        p_variable_ref.setInferredType(
            new PType(PType::PrimitiveTypeEnum::kErrorType));
        return;
    }

    // 1. The identifier has to be in symbol tables.
    if (!entry) {
        printErrorAndSetType(
            UndeclaredSymbolError(p_variable_ref.getLocation(),
                                    p_variable_ref.getNameCString()),
            p_variable_ref);
        return;
    }
    // 2. The kind of symbol has to be a parameter, variable, loop_var, or
    // constant.
    if (entry->getKind() != SymbolEntry::KindEnum::kParameterKind &&
        entry->getKind() != SymbolEntry::KindEnum::kVariableKind &&
        entry->getKind() != SymbolEntry::KindEnum::kLoopVarKind &&
        entry->getKind() != SymbolEntry::KindEnum::kConstantKind) {
        printErrorAndSetType(
            NonVariableSymbolError(p_variable_ref.getLocation(),
                                    p_variable_ref.getNameCString()),
            p_variable_ref);
        return;
    }
    // 3. Each index of an array reference must be of the integer type.
    // NOTE: It's sound even they are scalar variables since the indices are
    // simply empty and the loop is not executed.
    for (const auto &index : p_variable_ref.getIndices()) {
        if (index->getInferredType()->isError()) {
            p_variable_ref.setInferredType(
                new PType(PType::PrimitiveTypeEnum::kErrorType));
            return;
        }
        if (!index->getInferredType()->isInteger()) {
            printErrorAndSetType(NonIntegerArrayIndexError(index->getLocation()), p_variable_ref);
            return;
        }
    }
    // 4. An over array subscript is forbidden.
    // NOTE: Subscripting an scalar variable also raises the error.
    if (p_variable_ref.getIndices().size() >
        entry->getTypePtr()->getDimensions().size()) {
        printErrorAndSetType(
            OverArraySubscriptError(p_variable_ref.getLocation(),
                                    p_variable_ref.getNameCString()),
            p_variable_ref);
        return;
    }

    p_variable_ref.setInferredType(entry->getTypePtr()->getStructElementType(
        p_variable_ref.getIndices().size()));
}

bool SemanticAnalyzer::analyzeAssignmentLvalue(
        const AssignmentNode &p_assignment,
        const SymbolManager &p_symbol_manager) {
    const auto &lvalue = p_assignment.getLvalue();
    const auto *const lvalue_type = lvalue.getInferredType();

    // 1. The type of the result of the variable reference cannot be an array
    // type.
    if (!lvalue_type->isScalar()) {
        printError(AssignToArrayTypeError(lvalue.getLocation()));
        return false;
    }

    const auto *const entry = p_symbol_manager.lookup(lvalue.getName());
    // 2. The variable reference cannot be a reference to a constant variable.
    if (entry->getKind() == SymbolEntry::KindEnum::kConstantKind) {
        printError(AssignToConstantError(lvalue.getLocation(), lvalue.getNameCString()));
        return false;
    }
    // 3. The variable reference cannot be a reference to a loop
    // variable when the context is within a loop body. Only the loop
    // header is allowed to increase the loop variable.
    if (m_context_stack.top() != SemanticContext::kForLoop &&
        entry->getKind() == SymbolEntry::KindEnum::kLoopVarKind) {
        printError(AssignToLoopVarError(lvalue.getLocation()));
        return false;
    }

    return true;
}

bool SemanticAnalyzer::analyzeAssignmentExpr(
        const AssignmentNode &p_assignment) {
    const auto &expr = p_assignment.getExpr();
    const auto *const expr_type = expr.getInferredType();

    // 4. The type of the result of the expression cannot be an array type.
    if (!expr_type->isScalar()) {
        printError(AssignByArrayTypeError(expr.getLocation()));
        return false;
    }
    // 5. The type of the variable reference (lvalue) must be the same as the
    // one of the expression after appropriate type coercion.
    const auto *const lvalue_type =
        p_assignment.getLvalue().getInferredType();
    if (!expr_type->canCoerceTo(lvalue_type)) {
        printError(IncompatibleAssignmentError(
            p_assignment.getLocation(), lvalue_type, expr_type));
        return false;
    }

    return true;
}

void SemanticAnalyzer::visit(AssignmentNode &p_assignment) {
    p_assignment.visitChildNodes(*this);

    // Skip the rest of semantic checks if there are any errors in the node of the
    // variable reference.
    if (p_assignment.getLvalue().getInferredType()->isError()) {
        return;
    }
    // Skip the rest of semantic checks if there are any errors in the node of the
    // variable reference.
    if (!analyzeAssignmentLvalue(p_assignment, m_symbol_manager)) {
        return;
    }
    // Skip the rest of semantic checks if there are any errors in the node of the
    // expression.
    if (p_assignment.getExpr().getInferredType()->isError()) {
        return;
    }
    if (!analyzeAssignmentExpr(p_assignment)) {
        return;
    }
}

void SemanticAnalyzer::visit(ReadNode &p_read) {
    p_read.visitChildNodes(*this);

    if (p_read.getTarget().getInferredType()->isError()) {
        return;
    }
    // 1. The type of the variable reference must be scalar type.
    if (!p_read.getTarget().getInferredType()->isScalar()) {
        printError(ReadToNonScalarTypeError(p_read.getTarget().getLocation()));
        return;
    }

    const auto *const entry =
        m_symbol_manager.lookup(p_read.getTarget().getName());
    assert(entry && "Shouldn't reach here. This should be caught during the"
                    "visits of child nodes");
    if (m_error_entry_set.find(const_cast<SymbolEntry *>(entry)) !=
        m_error_entry_set.end()) {
        return;
    }
    // 2. The kind of symbol of the variable reference cannot be constant or
    // loop_var.
    if (entry->getKind() == SymbolEntry::KindEnum::kConstantKind ||
        entry->getKind() == SymbolEntry::KindEnum::kLoopVarKind) {
        printError(ReadToConstantOrLoopVarError(p_read.getTarget().getLocation()));
        return;
    }
}

void SemanticAnalyzer::visit(IfNode &p_if) {
    p_if.visitChildNodes(*this);

    if (p_if.getCondition().getInferredType()->isError()) {
        return;
    }
    // 1. The type of the result of the expression (condition) must be boolean
    // type.
    if (!p_if.getCondition().getInferredType()->isBool()) {
        printError(NonBooleanConditionError(p_if.getCondition().getLocation()));
        return;
    }
}

void SemanticAnalyzer::visit(WhileNode &p_while) {
    p_while.visitChildNodes(*this);

    if (p_while.getCondition().getInferredType()->isError()) {
        return;
    }
    // 1. The type of the result of the expression (condition) must be boolean
    // type.
    if (!p_while.getCondition().getInferredType()->isBool()) {
        printError(NonBooleanConditionError(p_while.getCondition().getLocation()));
        return;
    }
}

void SemanticAnalyzer::visit(ForNode &p_for) {
    m_symbol_manager.pushScope();
    m_context_stack.push(SemanticContext::kForLoop);

    p_for.visitChildNodes(*this);
    // The initial value of the loop variable and the constant value of the
    // condition must be in the incremental order.
    if (p_for.getLowerBound().getConstantPtr()->integer() >=
        p_for.getUpperBound().getConstantPtr()->integer()) {
        printError(NonIncrementalLoopVariableError(p_for.getLocation()));
    }

    m_context_stack.pop();
    m_symbol_table_of_scoping_nodes[&p_for] = m_symbol_manager.popScope();
}

void SemanticAnalyzer::visit(ReturnNode &p_return) {
    p_return.visitChildNodes(*this);

    const auto *const expected_return_type = m_returned_type_stack.top();
    // 1. The current context shouldn't be in the program or a procedure since
    // their return type is void.
    if (expected_return_type->isVoid()) {
        printError(ReturnFromVoidError(p_return.getLocation()));
        return;
    }

    // Skip the rest of semantic checks if there are any errors in the node of the
    // expression (return value).
    if (p_return.getReturnValue().getInferredType()->isError()) {
        return;
    }
    const auto *const retval_type =
        p_return.getReturnValue().getInferredType();
    // 2. The type of the result of the expression (return value) must be the same
    // type as the return type of current function after appropriate type coercion.
    if (!retval_type->canCoerceTo(expected_return_type)) {
        m_error_printer.print(IncompatibleReturnTypeError(
            p_return.getReturnValue().getLocation(), expected_return_type,
            retval_type));
        return;
    }
}

void SemanticAnalyzer::printError(const Error& p_error) {
    m_error_printer.print(p_error);
    m_has_error = true;
}

void SemanticAnalyzer::printErrorAndSetType(const Error& p_error,
                                            ExpressionNode& p_expr) {
    printError(p_error);
    p_expr.setInferredType(new PType(PType::PrimitiveTypeEnum::kErrorType));
}
