#include "compiler.h"

csaw::value_t csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<Expr>& expr)
{
	if (auto e = std::dynamic_pointer_cast<BinExpr>(expr))
		return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<CallExpr>(expr))
		return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<ChrExpr>(expr))
		return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<ConExpr>(expr))
		return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<IdExpr>(expr))
		return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<IndexExpr>(expr))
		return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<LambdaExpr>(expr))
		return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<MemExpr>(expr))
		return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<NumExpr>(expr))
		return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<StrExpr>(expr))
		return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<UnExpr>(expr))
		return GenIR(env, e);
	if (auto e = std::dynamic_pointer_cast<VarArgExpr>(expr))
		return GenIR(env, e);

	throw "TODO";
}

static csaw::value_t Assign(const std::shared_ptr<csaw::Environment>& env, const std::shared_ptr<csaw::Expr>& obj, csaw::value_t value)
{
	if (auto e = std::dynamic_pointer_cast<csaw::IdExpr>(obj))
		return env->SetVariable(e->Value, value);

	if (auto e = std::dynamic_pointer_cast<csaw::MemExpr>(obj))
	{
		auto object = GenIR(env, e->Object);
		auto strtype = llvm::dyn_cast<llvm::StructType>(object.ptrType.element);
		auto type = csaw::Environment::GetType(strtype);

		unsigned int i = 0;
		for (; i < type->fields.size(); i++)
			if (type->fields[i].first == e->Member)
				break;

		if (i == type->fields.size())
		{
			llvm::errs() << "Undefined field '" << e->Member << "'\r\n";
			throw;
		}

		auto ptr = csaw::Environment::Builder().CreateStructGEP(strtype, object(), i);
		csaw::Environment::Builder().CreateStore(value(), ptr);
		return value;
	}

	throw "TODO";
}

csaw::value_t csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<BinExpr>& expr)
{
	std::string op = expr->Operator;
	bool assign = op.find_last_of('=') == 1 && !(op == "==" || op == "!=" || op == "<=" || op == ">=");

	auto left = GenIR(env, expr->Left);
	auto right = GenIR(env, expr->Right);

	auto value = Environment::CreateCall(assign ? left.ptrType : type_t(), expr->Operator, { left, right }, true);
	if (value)
		return value;

	if (op == "=")
		return Assign(env, expr->Left, right);

	if (assign)
		op = op.substr(0, 1);

	if (op == "+")
		value = OpAdd(left, right);
	else if (op == "-")
		value = OpSub(left, right);
	else if (op == "*")
		value = OpMul(left, right);
	else if (op == "/")
		value = OpDiv(left, right);
	else if (op == "%")
		value = OpMod(left, right);
	else if (op == "==")
		value = OpEQ(left, right);
	else if (op == "!=")
		value = OpNE(left, right);
	else if (op == "<")
		value = OpLT(left, right);
	else if (op == ">")
		value = OpGT(left, right);
	else if (op == "<=")
		value = OpLTE(left, right);
	else if (op == ">=")
		value = OpGTE(left, right);
	else if (op == "&&")
		value = OpLAnd(left, right);
	else if (op == "||")
		value = OpLOr(left, right);
	else if (op == "&")
		value = OpAnd(left, right);
	else if (op == "|")
		value = OpOr(left, right);
	else if (op == "^")
		value = OpXOr(left, right);
	else if (op == "<<")
		value = OpShL(left, right);
	else if (op == ">>")
		value = OpShR(left, right);

	if (!value)
		throw "TODO";

	if (assign)
		return Assign(env, expr->Left, value);

	return value;
}

csaw::value_t csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<CallExpr>& expr)
{
	std::vector<value_t> args;
	for (auto& arg : expr->Arguments)
		args.push_back(GenIR(env, arg));

	if (auto e = std::dynamic_pointer_cast<IdExpr>(expr->Function))
		return Environment::CreateCall(type_t(), e->Value, args);

	if (auto e = std::dynamic_pointer_cast<MemExpr>(expr->Function))
	{
		auto object = GenIR(env, e->Object);
		args.insert(args.begin(), object);
		return Environment::CreateCall(object.ptrType, e->Member, args);
	}

	throw "TODO";
}

csaw::value_t csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<ChrExpr>& expr)
{
	return value_t(Environment::Builder().getInt8(expr->Value), type_t("chr", Environment::Builder().getInt8Ty()));
}

csaw::value_t csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<ConExpr>& expr)
{
	auto fun = Environment::Builder().GetInsertBlock()->getParent();
	auto bthen = llvm::BasicBlock::Create(Environment::Context(), "then");
	auto belse = llvm::BasicBlock::Create(Environment::Context(), "else");
	auto bmerge = llvm::BasicBlock::Create(Environment::Context(), "merge");

	auto condition = GenIR(env, expr->Condition);
	condition.value = NumToBool(condition());
	Environment::Builder().CreateCondBr(condition(), bthen, belse);

	fun->insert(fun->end(), bthen);
	Environment::Builder().SetInsertPoint(bthen);
	auto vthen = GenIR(env, expr->Then);
	Environment::Builder().CreateBr(bmerge);
	bthen = Environment::Builder().GetInsertBlock();

	fun->insert(fun->end(), belse);
	Environment::Builder().SetInsertPoint(belse);
	auto velse = GenIR(env, expr->Else);
	Environment::Builder().CreateBr(bmerge);
	belse = Environment::Builder().GetInsertBlock();

	fun->insert(fun->end(), bmerge);
	Environment::Builder().SetInsertPoint(bmerge);
	auto phi = Environment::Builder().CreatePHI(vthen()->getType(), 2);
	phi->addIncoming(vthen(), bthen);
	phi->addIncoming(velse(), belse);

	return value_t(phi, vthen.ptrType);
}

csaw::value_t csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<IdExpr>& expr)
{
	auto value = env->GetVariable(expr->Value);
	if (auto global = llvm::dyn_cast<llvm::GlobalValue>(value()))
		return value_t(Environment::Builder().CreateLoad(global->getValueType(), global), value.ptrType);
	return value;
}

csaw::value_t csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<IndexExpr>& expr)
{
	auto object = GenIR(env, expr->Object);
	auto index = GenIR(env, expr->Index);

	if (auto value = Environment::CreateCall(object.ptrType, "[]", { object, index }, true))
		return value;

	throw "TODO";
}

csaw::value_t csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<LambdaExpr>& expr)
{
	throw "TODO";
}

csaw::value_t csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<MemExpr>& expr)
{
	auto object = GenIR(env, expr->Object);
	auto strtype = llvm::dyn_cast<llvm::StructType>(object.ptrType.element);
	auto type = Environment::GetType(strtype);

	unsigned int i = 0;
	for (; i < type->fields.size(); i++)
		if (type->fields[i].first == expr->Member)
			break;

	if (i == type->fields.size())
	{
		llvm::errs() << "Undefined field '" << expr->Member << "'\r\n";
		throw;
	}

	auto ptr = Environment::Builder().CreateStructGEP(strtype, object(), i);
	return value_t(Environment::Builder().CreateLoad(strtype->getElementType(i), ptr), type->fields[i].second);
}

csaw::value_t csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<NumExpr>& expr)
{
	auto dty = Environment::Builder().getDoubleTy();
	return value_t(llvm::ConstantFP::get(dty, expr->Value), type_t("num", dty));
}

csaw::value_t csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<StrExpr>& expr)
{
	auto i8 = Environment::Builder().getInt8Ty();
	auto str = Environment::Builder().CreateGlobalStringPtr(expr->Value, "str");
	return value_t(str, type_t("str", str->getType(), i8));
}

csaw::value_t csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<UnExpr>& expr)
{
	auto val = GenIR(env, expr->Value);

	auto value = Environment::CreateCall(val.ptrType, expr->Operator, { val }, true);
	if (value)
		return value;

	auto dty = Environment::Builder().getDoubleTy();

	if (expr->Operator == "!")
		value = OpNot(val);
	else if (expr->Operator == "-")
		value = OpNeg(val);
	else if (expr->Operator == "~")
		value = OpInv(val);
	else if (expr->Operator == "++")
		return Assign(env, expr->Value, OpAdd(val, value_t(llvm::ConstantFP::get(dty, 1.0), type_t("num", dty))));
	else if (expr->Operator == "--")
		return Assign(env, expr->Value, OpSub(val, value_t(llvm::ConstantFP::get(dty, 1.0), type_t("num", dty))));

	if (!value)
		throw "TODO";

	return value;
}

csaw::value_t csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<VarArgExpr>& expr)
{
	if (!expr->Type)
		return value_t(env->GetVarArgs(), type_t("any", Environment::Builder().getPtrTy()));

	auto type = GenIR(expr->Type);
	return Environment::NextVarArg(type, env->GetVarArgs());
}
