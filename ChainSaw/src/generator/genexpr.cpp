#include <csaw/gen.h>

#include <iostream>

csaw::value_t* csaw::GenIR(const EnvPtr& env, const ExprPtr& expr)
{
	if (auto e = std::dynamic_pointer_cast<BinaryExpr>(expr)) return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<CallExpr>(expr)) return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<ChrExpr>(expr)) return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<IdExpr>(expr)) return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<IndexExpr>(expr)) return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<LambdaExpr>(expr)) return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<MemberExpr>(expr)) return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<NumExpr>(expr)) return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<SelectExpr>(expr)) return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<StrExpr>(expr)) return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<UnaryExpr>(expr)) return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<VarArgExpr>(expr)) return GenIR(env, e);

	throw;
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const BinaryExprPtr& expr)
{
	auto left = GenIR(env, expr->Left);
	auto right = GenIR(env, expr->Right);

	if (expr->Operator == "+") return GenIRAdd(env, left, right);
	if (expr->Operator == "-") return GenIRSub(env, left, right);
	if (expr->Operator == "*") return GenIRMul(env, left, right);
	if (expr->Operator == "/") return GenIRDiv(env, left, right);
	if (expr->Operator == "%") return GenIRRem(env, left, right);
	if (expr->Operator == "==") return GenIRCmpEQ(env, left, right);
	if (expr->Operator == "!=") return GenIRCmpNE(env, left, right);
	if (expr->Operator == ">") return GenIRCmpGT(env, left, right);
	if (expr->Operator == "<") return GenIRCmpLT(env, left, right);
	if (expr->Operator == ">=") return GenIRCmpGE(env, left, right);
	if (expr->Operator == "<=") return GenIRCmpLE(env, left, right);

	throw;
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const CallExprPtr& expr)
{
	std::string callee;
	if (auto e = std::dynamic_pointer_cast<IdExpr>(expr->Function))
	{
		callee = e->Value;
	}
	else if (auto e = std::dynamic_pointer_cast<MemberExpr>(expr->Function))
	{
		callee = e->Member;
	}

	std::vector<llvm::Value*> args;
	for (auto& arg : expr->Arguments)
		args.push_back(*GenIR(env, arg));

	return new value_t(env->Builder().CreateCall(env->Module().getFunction(callee), args));
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const ChrExprPtr& expr)
{
	throw;
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const IdExprPtr& expr)
{
	if (auto value = env->Values()[expr->Value])
		return value;
	if (auto var = env->Variables()[expr->Value])
		return new value_t(env->Builder().CreateLoad(*var->Type->Element, *var, expr->Value), var->Type->Element);
	return nullptr;
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const IndexExprPtr& expr)
{
	throw;
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const LambdaExprPtr& expr)
{
	throw;
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const MemberExprPtr& expr)
{
	throw;
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const NumExprPtr& expr)
{
	return new value_t(llvm::ConstantFP::get(env->Builder().getDoubleTy(), expr->Value));
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const SelectExprPtr& expr)
{
	auto condition = GenIR(env, expr->Condition);
	auto vtrue = GenIR(env, expr->True);
	auto vfalse = GenIR(env, expr->False);
	return new value_t(env->Builder().CreateSelect(GenIRBool(env, *condition), *vtrue, *vfalse), vtrue->Type);
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const StrExprPtr& expr)
{
	auto str = env->Builder().CreateGlobalStringPtr(expr->Value);
	return new value_t(str, new type_t(str->getType(), new type_t(llvm::Type::getInt8Ty(env->Context()))));
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const UnaryExprPtr& expr)
{
	throw;
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const VarArgExprPtr& expr)
{
	throw;
}
