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

		operator llvm::Value* () { return Value; }
		operator llvm::Value* () const { return Value; }

		llvm::Value* Value;
		type_t* Type;
	};

	struct function_t
	{
		function_t(
			llvm::Function* function,
			type_t* result,
			type_t* member_of,
			std::vector<type_t*> args,
			bool is_var_args,
			bool is_constructor)
			: Function(function),
			Result(result),
			MemberOf(member_of),
			Args(args),
			IsVarArgs(is_var_args),
			IsConstructor(is_constructor)
		{}

		llvm::Function* operator*() { return Function; }

		llvm::Function* Function;
		type_t* Result;
		type_t* MemberOf;
		std::vector<type_t*> Args;
		bool IsVarArgs;
		bool IsConstructor;
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

	value_t* GenIRAssign(const EnvPtr& env, const ExprPtr& object, value_t* value);

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
	value_t* GenIRLogAnd(const EnvPtr& env, value_t* left, value_t* right);
	value_t* GenIRLogOr(const EnvPtr& env, value_t* left, value_t* right);

	value_t* GenIRNot(const EnvPtr& env, value_t* value);
	value_t* GenIRNeg(const EnvPtr& env, value_t* value);
	value_t* GenIRInv(const EnvPtr& env, value_t* value);

	llvm::Value* GenIRBool(const EnvPtr& env, llvm::Value* value);
	llvm::Value* GenIRNum(const EnvPtr& env, llvm::Value* value);
}
