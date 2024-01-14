#pragma once

#include "ast.h"
#include "env.h"

#include <llvm/IR/Value.h>

namespace csaw
{
	llvm::Type* GenIR(const EnvPtr& env, const TypePtr& type);
	llvm::Type* GenIR(const EnvPtr& env, const ArrayTypePtr& type);

	void GenIR(const EnvPtr& env, const StmtPtr& stmt);
	void GenIR(const EnvPtr& env, const AliasStmtPtr& stmt);
	void GenIR(const EnvPtr& env, const EnclosedStmtPtr& stmt);
	void GenIR(const EnvPtr& env, const ForStmtPtr& stmt);
	void GenIR(const EnvPtr& env, const FunctionStmtPtr& stmt);
	void GenIR(const EnvPtr& env, const IfStmtPtr& stmt);
	void GenIR(const EnvPtr& env, const IncStmtPtr& stmt);
	void GenIR(const EnvPtr& env, const RetStmtPtr& stmt);
	void GenIR(const EnvPtr& env, const ThingStmtPtr& stmt);
	void GenIR(const EnvPtr& env, const VariableStmtPtr& stmt);
	void GenIR(const EnvPtr& env, const WhileStmtPtr& stmt);

	llvm::Value* GenIR(const EnvPtr& env, const ExprPtr& expr);
	llvm::Value* GenIR(const EnvPtr& env, const BinaryExprPtr& expr);
	llvm::Value* GenIR(const EnvPtr& env, const CallExprPtr& expr);
	llvm::Value* GenIR(const EnvPtr& env, const ChrExprPtr& expr);
	llvm::Value* GenIR(const EnvPtr& env, const IdExprPtr& expr);
	llvm::Value* GenIR(const EnvPtr& env, const IndexExprPtr& expr);
	llvm::Value* GenIR(const EnvPtr& env, const LambdaExprPtr& expr);
	llvm::Value* GenIR(const EnvPtr& env, const MemberExprPtr& expr);
	llvm::Value* GenIR(const EnvPtr& env, const NumExprPtr& expr);
	llvm::Value* GenIR(const EnvPtr& env, const SelectExprPtr& expr);
	llvm::Value* GenIR(const EnvPtr& env, const StrExprPtr& expr);
	llvm::Value* GenIR(const EnvPtr& env, const UnaryExprPtr& expr);
	llvm::Value* GenIR(const EnvPtr& env, const VarArgExprPtr& expr);

}
