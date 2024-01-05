#include "../parser/parser.h"
#include "compiler.h"

#include <llvm/IR/Verifier.h>

llvm::Type* csaw::GenIR(const std::shared_ptr<ASTType>& type)
{
	if (auto t = std::dynamic_pointer_cast<ASTArrayType>(type))
		return GenIR(t);

	if (type->Name.empty())
		return llvm::Type::getVoidTy(Environment::Context());
	if (type->Name == "any")
		return llvm::PointerType::get(Environment::Context(), 0);
	if (type->Name == "num")
		return llvm::Type::getDoubleTy(Environment::Context());
	if (type->Name == "chr")
		return llvm::Type::getInt8Ty(Environment::Context());
	if (type->Name == "str")
		return llvm::PointerType::get(llvm::Type::getInt8Ty(Environment::Context()), 0);

	throw;
}

llvm::Type* csaw::GenIR(const std::shared_ptr<ASTArrayType>& type)
{
	return llvm::ArrayType::get(GenIR(type->Type), type->Size);
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<Stmt>& stmt)
{
	if (auto s = std::dynamic_pointer_cast<AliasStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<EnclosedStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<ForStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<FunStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<IfStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<IncStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<RetStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<ThingStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<VarStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<WhileStmt>(stmt))
		return GenIR(env, s);

	if (auto e = std::dynamic_pointer_cast<Expr>(stmt))
		return GenIR(env, e);

	throw;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<AliasStmt>& stmt)
{
	throw;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<EnclosedStmt>& stmt)
{
	auto e = std::make_shared<Environment>(env);

	llvm::Value* value = nullptr;
	for (auto& s : stmt->Body)
		value = GenIR(e, s);

	return value;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<ForStmt>& stmt)
{
	throw;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<FunStmt>& stmt)
{
	std::vector<llvm::Type*> pt;
	for (auto& parameter : stmt->Parameters)
		pt.push_back(GenIR(parameter.Type));

	auto ft = llvm::FunctionType::get(GenIR(stmt->RetType), pt, !stmt->VarArg.empty());
	auto fun = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, stmt->Name, Environment::Module());
	Environment::CreateFunction(stmt->Name, fun);

	unsigned i = 0;
	for (auto& arg : fun->args())
		arg.setName(stmt->Parameters[i++].Name);

	if (!stmt->Body)
		return fun;

	if (!fun->empty())
		throw;

	auto entry = llvm::BasicBlock::Create(Environment::Context(), "entry", fun);
	Environment::Builder().SetInsertPoint(entry);

	auto fe = std::make_shared<Environment>(env);

	for (auto& arg : fun->args())
		fe->CreateVariable(std::string(arg.getName()), &arg);

	auto value = GenIR(fe, stmt->Body);

	if (!value || !llvm::ReturnInst::classof(value))
		Environment::Builder().CreateRetVoid();

	if (llvm::verifyFunction(*fun, &llvm::errs()))
	{
		fun->print(llvm::errs());
		throw;
	}

	Environment::Optimize(fun);

	return fun;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<IfStmt>& stmt)
{
	auto condition = GenIR(env, stmt->Condition);

	auto fun = Environment::Builder().GetInsertBlock()->getParent();
	auto bthen = llvm::BasicBlock::Create(Environment::Context(), "then", fun);
	auto belse = llvm::BasicBlock::Create(Environment::Context(), "else");
	auto bmerge = llvm::BasicBlock::Create(Environment::Context(), "merge");

	Environment::Builder().CreateCondBr(condition, bthen, belse);

	Environment::Builder().SetInsertPoint(bthen);
	auto vthen = GenIR(env, stmt->Then);
	Environment::Builder().CreateBr(bmerge);
	bthen = Environment::Builder().GetInsertBlock();

	fun->insert(fun->end(), belse);
	Environment::Builder().SetInsertPoint(belse);
	auto velse = GenIR(env, stmt->Else);
	Environment::Builder().CreateBr(bmerge);
	belse = Environment::Builder().GetInsertBlock();

	fun->insert(fun->end(), bmerge);
	Environment::Builder().SetInsertPoint(bmerge);
	auto phi = Environment::Builder().CreatePHI(vthen->getType(), 2);
	phi->addIncoming(vthen, bthen);
	phi->addIncoming(velse, belse);

	return phi;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<IncStmt>& stmt)
{
	auto path = env->Path().parent_path() / stmt->Path;
	if (!ParseInc(env, path))
		throw;
	return nullptr;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<RetStmt>& stmt)
{
	if (!stmt->Value)
		return Environment::Builder().CreateRetVoid();
	return Environment::Builder().CreateRet(GenIR(env, stmt->Value));
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<ThingStmt>& stmt)
{
	throw;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<VarStmt>& stmt)
{
	auto type = GenIR(stmt->Type);

	if (!stmt->Value)
	{
		env->CreateVariable(stmt->Name, type);
		return nullptr;
	}

	auto value = GenIR(env, stmt->Value);
	if (value->getType() != type)
		throw;

	env->CreateVariable(stmt->Name, value);
	return nullptr;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<WhileStmt>& stmt)
{
	auto fun = Environment::Builder().GetInsertBlock()->getParent();
	auto bcondition = llvm::BasicBlock::Create(Environment::Context(), "condition");
	auto bwhile = llvm::BasicBlock::Create(Environment::Context(), "while");
	auto bexit = llvm::BasicBlock::Create(Environment::Context(), "exit");

	Environment::Builder().CreateBr(bcondition);

	fun->insert(fun->end(), bcondition);
	Environment::Builder().SetInsertPoint(bcondition);
	auto vcondition = GenIR(env, stmt->Condition);
	if (vcondition->getType()->isDoubleTy())
		vcondition = Environment::Builder().CreateFCmpONE(vcondition, llvm::ConstantFP::get(llvm::Type::getDoubleTy(Environment::Context()), 0.0));
	Environment::Builder().CreateCondBr(vcondition, bwhile, bexit);

	fun->insert(fun->end(), bwhile);
	Environment::Builder().SetInsertPoint(bwhile);
	auto vwhile = GenIR(env, stmt->Body);
	Environment::Builder().CreateBr(bcondition);

	fun->insert(fun->end(), bexit);
	Environment::Builder().SetInsertPoint(bexit);

	return vwhile;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<Expr>& expr)
{
	if (auto e = std::dynamic_pointer_cast<AssignExpr>(expr))
		return GenIR(env, e);
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

	throw;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<AssignExpr>& expr)
{
	auto value = GenIR(env, expr->Value);
	if (auto e = std::dynamic_pointer_cast<IdExpr>(expr->Object))
		return env->SetVariable(e->Value, value);

	throw;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<BinExpr>& expr)
{
	auto left = GenIR(env, expr->Left);
	auto right = GenIR(env, expr->Right);

	llvm::Value* value = nullptr;
	if (expr->Operator == "+")
		value = OpAdd(left, right);
	else if (expr->Operator == "-")
		value = OpSub(left, right);
	else if (expr->Operator == "*")
		value = OpMul(left, right);
	else if (expr->Operator == "/")
		value = OpDiv(left, right);
	else if (expr->Operator == "==")
		value = OpEqu(left, right);
	else if (expr->Operator == "!=")
		value = OpNeq(left, right);
	else if (expr->Operator == "&&")
		value = OpAnd(left, right);
	else if (expr->Operator == "||")
		value = OpOr(left, right);

	if (value)
		return value;

	return Environment::CreateCall(expr->Operator, { left, right });
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<CallExpr>& expr)
{
	std::vector<llvm::Value*> args;
	for (auto& arg : expr->Arguments)
		args.push_back(GenIR(env, arg));

	if (auto e = std::dynamic_pointer_cast<IdExpr>(expr->Function))
		return Environment::CreateCall(e->Value, args);

	throw;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<ChrExpr>& expr)
{
	throw;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<ConExpr>& expr)
{
	auto condition = GenIR(env, expr->Condition);

	auto fun = Environment::Builder().GetInsertBlock()->getParent();
	auto bthen = llvm::BasicBlock::Create(Environment::Context(), "then", fun);
	auto belse = llvm::BasicBlock::Create(Environment::Context(), "else");
	auto bmerge = llvm::BasicBlock::Create(Environment::Context(), "merge");

	Environment::Builder().CreateCondBr(condition, bthen, belse);

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
	auto phi = Environment::Builder().CreatePHI(vthen->getType(), 2);
	phi->addIncoming(vthen, bthen);
	phi->addIncoming(velse, belse);

	return phi;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<IdExpr>& expr)
{
	return env->GetVariable(expr->Value);
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<IndexExpr>& expr)
{
	throw;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<LambdaExpr>& expr)
{
	throw;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<MemExpr>& expr)
{
	throw;
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<NumExpr>& expr)
{
	return llvm::ConstantFP::get(Environment::Context(), llvm::APFloat(expr->Value));
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<StrExpr>& expr)
{
	return Environment::Builder().CreateGlobalString(expr->Value);
}

llvm::Value* csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<UnExpr>& expr)
{
	auto val = GenIR(env, expr->Value);

	llvm::Value* value = nullptr;
	if (expr->Operator == "!")
		value = OpNot(val);
	else if (expr->Operator == "-")
		value = OpNeg(val);
	else if (expr->Operator == "~")
		value = OpInv(val);

	if (value)
		return value;

	return Environment::CreateCall(expr->Operator, { val });
}
