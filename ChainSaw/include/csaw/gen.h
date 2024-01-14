#pragma once

#include "ast.h"
#include "env.h"

#include <llvm/IR/Value.h>

namespace csaw
{
	class Environment;
	typedef std::shared_ptr<Environment> EnvPtr;

	struct type_t
	{
		type_t(llvm::Type* type, type_t* element)
			: Type(type), Element(element)
		{}

		type_t(llvm::Type* type)
			: type_t(type, nullptr)
		{}

		operator llvm::Type* () { return Type; }
		operator llvm::Type* () const { return Type; }

		llvm::Type* Type;
		type_t* Element;
	};

	struct value_t
	{
		value_t(llvm::Value* value, type_t* type)
			: Value(value), Type(type)
		{}

		value_t(llvm::Value* value)
			: value_t(value, new type_t(value->getType()))
		{}

		operator llvm::Value* () { return Value; }
		operator llvm::Value* () const { return Value; }

		llvm::Value* Value;
		type_t* Type;
	};

	struct function_t
	{

		type_t* MemberOf;
		std::vector<type_t*> Args;
	};

	type_t* GenIR(const EnvPtr& env, const TypePtr& type);
	type_t* GenIR(const EnvPtr& env, const ArrayTypePtr& type);

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

	value_t* GenIR(const EnvPtr& env, const ExprPtr& expr);
	value_t* GenIR(const EnvPtr& env, const BinaryExprPtr& expr);
	value_t* GenIR(const EnvPtr& env, const CallExprPtr& expr);
	value_t* GenIR(const EnvPtr& env, const ChrExprPtr& expr);
	value_t* GenIR(const EnvPtr& env, const IdExprPtr& expr);
	value_t* GenIR(const EnvPtr& env, const IndexExprPtr& expr);
	value_t* GenIR(const EnvPtr& env, const LambdaExprPtr& expr);
	value_t* GenIR(const EnvPtr& env, const MemberExprPtr& expr);
	value_t* GenIR(const EnvPtr& env, const NumExprPtr& expr);
	value_t* GenIR(const EnvPtr& env, const SelectExprPtr& expr);
	value_t* GenIR(const EnvPtr& env, const StrExprPtr& expr);
	value_t* GenIR(const EnvPtr& env, const UnaryExprPtr& expr);
	value_t* GenIR(const EnvPtr& env, const VarArgExprPtr& expr);

	value_t* GenIRAdd(const EnvPtr& env, value_t* left, value_t* right);
	value_t* GenIRSub(const EnvPtr& env, value_t* left, value_t* right);
	value_t* GenIRMul(const EnvPtr& env, value_t* left, value_t* right);
	value_t* GenIRDiv(const EnvPtr& env, value_t* left, value_t* right);
	value_t* GenIRRem(const EnvPtr& env, value_t* left, value_t* right);

	value_t* GenIRCmpEQ(const EnvPtr& env, value_t* left, value_t* right);
	value_t* GenIRCmpNE(const EnvPtr& env, value_t* left, value_t* right);
	value_t* GenIRCmpGT(const EnvPtr& env, value_t* left, value_t* right);
	value_t* GenIRCmpLT(const EnvPtr& env, value_t* left, value_t* right);
	value_t* GenIRCmpGE(const EnvPtr& env, value_t* left, value_t* right);
	value_t* GenIRCmpLE(const EnvPtr& env, value_t* left, value_t* right);

	llvm::Value* GenIRBool(const EnvPtr& env, llvm::Value* value);
}
