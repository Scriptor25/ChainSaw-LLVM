#include <csaw/gen.h>

#include <iostream>

csaw::value_t* csaw::GenIRAssign(const EnvPtr& env, const ExprPtr& object, value_t* value)
{
	if (auto obj = std::dynamic_pointer_cast<IdExpr>(object))
	{
		auto ptr = env->LocalVariables()[obj->Value];
		env->Builder().CreateStore(*value, *ptr);
		return value;
	}

	throw;
}

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

	if (auto callee = env->GetFunction(expr->Operator, env->GetVoidType(), { left->Type, right->Type }))
		return new value_t(env->Builder().CreateCall(**callee, { *left, *right }), callee->Result);

	if (expr->Operator == "==") return GenIRCmpEQ(env, left, right);
	if (expr->Operator == "!=") return GenIRCmpNE(env, left, right);
	if (expr->Operator == ">") return GenIRCmpGT(env, left, right);
	if (expr->Operator == "<") return GenIRCmpLT(env, left, right);
	if (expr->Operator == ">=") return GenIRCmpGE(env, left, right);
	if (expr->Operator == "<=") return GenIRCmpLE(env, left, right);
	if (expr->Operator == "&&") return GenIRLogAnd(env, left, right);
	if (expr->Operator == "||") return GenIRLogOr(env, left, right);
	if (expr->Operator == "=") return GenIRAssign(env, expr->Left, right);

	std::string op = expr->Operator;
	size_t idx = op.find('=');
	bool assign = idx != std::string::npos;
	if (assign)
		op = op.substr(0, idx);

	value_t* value = nullptr;
	if (op == "+") value = GenIRAdd(env, left, right);
	else if (op == "-") value = GenIRSub(env, left, right);
	else if (op == "*") value = GenIRMul(env, left, right);
	else if (op == "/") value = GenIRDiv(env, left, right);
	else if (op == "%") value = GenIRRem(env, left, right);
	else if (op == "&") throw;
	else if (op == "|") throw;
	else if (op == "^") throw;
	else if (op == "<<") throw;
	else if (op == ">>") throw;

	if (!value)
		throw;

	if (assign)
		return GenIRAssign(env, expr->Left, value);

	return value;
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const CallExprPtr& expr)
{
	std::string callee;
	value_t* memberof = nullptr;
	if (auto e = std::dynamic_pointer_cast<IdExpr>(expr->Function))
	{
		callee = e->Value;
	}
	else if (auto e = std::dynamic_pointer_cast<MemberExpr>(expr->Function))
	{
		callee = e->Member;
		memberof = GenIR(env, e->Object);
	}

	std::vector<value_t*> argvalues;
	std::vector<type_t*> argtypes;
	for (auto& arg : expr->Arguments)
	{
		auto value = GenIR(env, arg);
		argvalues.push_back(value);
		argtypes.push_back(value->Type);
	}

	auto function = env->GetFunction(callee, memberof ? memberof->Type : env->GetVoidType(), argtypes);
	if (!function)
		throw;

	std::vector<llvm::Value*> args;
	for (auto value : argvalues)
		args.push_back(*value);

	return new value_t(env->Builder().CreateCall(**function, args), function->Result);
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const ChrExprPtr& expr)
{
	return env->GetChr(expr->Value);
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const IdExprPtr& expr)
{
	if (auto var = env->LocalVariables()[expr->Value])
		return new value_t(env->Builder().CreateLoad(*var->Type->Element, *var, expr->Value), var->Type->Element);
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
	return env->GetNum(expr->Value);
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const SelectExprPtr& expr)
{
	auto parent = env->Builder().GetInsertBlock()->getParent();
	auto trueblock = llvm::BasicBlock::Create(env->Context(), "select.true", parent);
	auto falseblock = llvm::BasicBlock::Create(env->Context(), "select.false", parent);
	auto exitblock = llvm::BasicBlock::Create(env->Context(), "select.exit", parent);

	auto condition = GenIRBool(env, *GenIR(env, expr->Condition));
	env->Builder().CreateCondBr(condition, trueblock, falseblock);

	env->Builder().SetInsertPoint(trueblock);
	auto vtrue = GenIR(env, expr->True);
	env->Builder().CreateBr(exitblock);
	trueblock = env->Builder().GetInsertBlock();

	env->Builder().SetInsertPoint(falseblock);
	auto vfalse = GenIR(env, expr->False);
	env->Builder().CreateBr(exitblock);
	falseblock = env->Builder().GetInsertBlock();

	env->Builder().SetInsertPoint(exitblock);

	auto phi = env->Builder().CreatePHI(*vtrue->Type, 2);
	phi->addIncoming(*vtrue, trueblock);
	phi->addIncoming(*vfalse, falseblock);

	return new value_t(phi, vtrue->Type);
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const StrExprPtr& expr)
{
	return env->GetStr(expr->Value);
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const UnaryExprPtr& expr)
{
	auto val = GenIR(env, expr->Value);

	if (auto callee = env->GetFunction(expr->Operator, val->Type, { val->Type }))
		return new value_t(env->Builder().CreateCall(**callee, { *val }), callee->Result);

	if (expr->Operator == "!") return GenIRNot(env, val);
	if (expr->Operator == "-") return GenIRNeg(env, val);
	if (expr->Operator == "~") return GenIRInv(env, val);

	if (expr->Operator == "++") return GenIRAssign(env, expr->Value, GenIRAdd(env, val, env->GetNum(1)));
	if (expr->Operator == "--") return GenIRAssign(env, expr->Value, GenIRSub(env, val, env->GetNum(1)));

	throw;
}

csaw::value_t* csaw::GenIR(const EnvPtr& env, const VarArgExprPtr& expr)
{
	throw;
}
