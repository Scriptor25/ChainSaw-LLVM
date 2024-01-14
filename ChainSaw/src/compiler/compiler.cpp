#include <csaw/compiler.h>

static std::map<std::string, csawir::Value*> VALUES;

void csaw::Build(csawir::Builder& builder, const std::shared_ptr<Stmt>& stmt)
{
	if (!stmt)
		throw;

	if (auto s = std::dynamic_pointer_cast<AliasStmt>(stmt))
		return Build(builder, s);
	if (auto s = std::dynamic_pointer_cast<EnclosedStmt>(stmt))
		return Build(builder, s);
	if (auto s = std::dynamic_pointer_cast<ForStmt>(stmt))
		return Build(builder, s);
	if (auto s = std::dynamic_pointer_cast<FunStmt>(stmt))
		return Build(builder, s);
	if (auto s = std::dynamic_pointer_cast<IfStmt>(stmt))
		return Build(builder, s);
	if (auto s = std::dynamic_pointer_cast<IncStmt>(stmt))
		return Build(builder, s);
	if (auto s = std::dynamic_pointer_cast<RetStmt>(stmt))
		return Build(builder, s);
	if (auto s = std::dynamic_pointer_cast<ThingStmt>(stmt))
		return Build(builder, s);
	if (auto s = std::dynamic_pointer_cast<VarStmt>(stmt))
		return Build(builder, s);
	if (auto s = std::dynamic_pointer_cast<WhileStmt>(stmt))
		return Build(builder, s);

	if (auto e = std::dynamic_pointer_cast<Expr>(stmt))
	{
		Build(builder, e);
		return;
	}

	throw;
}

void csaw::Build(csawir::Builder& builder, const std::shared_ptr<AliasStmt>& stmt)
{
	throw;
}

void csaw::Build(csawir::Builder& builder, const std::shared_ptr<EnclosedStmt>& stmt)
{
	for (auto& s : stmt->Body)
		Build(builder, s);
}

void csaw::Build(csawir::Builder& builder, const std::shared_ptr<ForStmt>& stmt)
{
	throw;
}

void csaw::Build(csawir::Builder& builder, const std::shared_ptr<FunStmt>& stmt)
{
	auto rettype = Build(builder, stmt->RetType);
	std::vector<csawir::Type*> args;
	for (auto& arg : stmt->Parameters)
		args.push_back(Build(builder, arg.Type));

	auto funtype = builder.GetContext().GetFunctionType(rettype, args, stmt->IsVarArg);
	auto function = builder.GetContext().GetFunction(stmt->Name, funtype);
	if (!function)
		function = builder.GetContext().CreateFunction(stmt->Name, funtype);

	if (!stmt->Body)
		return;

	if (function->IsDefined())
		throw;

	VALUES.clear();
	size_t i = 0;
	for (auto& arg : stmt->Parameters)
	{
		function->Args.push_back(arg.Name);
	}

	builder.StartInsert(function, "entry");
	Build(builder, stmt->Body);

	builder.StartInsert(nullptr, "");
	VALUES.clear();
}

void csaw::Build(csawir::Builder& builder, const std::shared_ptr<IfStmt>& stmt)
{
	throw;
}

void csaw::Build(csawir::Builder& builder, const std::shared_ptr<IncStmt>& stmt)
{
	throw;
}

void csaw::Build(csawir::Builder& builder, const std::shared_ptr<RetStmt>& stmt)
{
	builder.AppendRet(stmt->Value ? Build(builder, stmt->Value) : nullptr);
}

void csaw::Build(csawir::Builder& builder, const std::shared_ptr<ThingStmt>& stmt)
{
	throw;
}

void csaw::Build(csawir::Builder& builder, const std::shared_ptr<VarStmt>& stmt)
{
	throw;
}

void csaw::Build(csawir::Builder& builder, const std::shared_ptr<WhileStmt>& stmt)
{
	throw;
}

csawir::Value* csaw::Build(csawir::Builder& builder, const std::shared_ptr<Expr>& expr)
{
	if (!expr)
		throw;

	if (auto e = std::dynamic_pointer_cast<BinExpr>(expr))
		return Build(builder, e);
	if (auto e = std::dynamic_pointer_cast<CallExpr>(expr))
		return Build(builder, e);
	if (auto e = std::dynamic_pointer_cast<ChrExpr>(expr))
		return Build(builder, e);
	if (auto e = std::dynamic_pointer_cast<ConExpr>(expr))
		return Build(builder, e);
	if (auto e = std::dynamic_pointer_cast<IdExpr>(expr))
		return Build(builder, e);
	if (auto e = std::dynamic_pointer_cast<IndexExpr>(expr))
		return Build(builder, e);
	if (auto e = std::dynamic_pointer_cast<LambdaExpr>(expr))
		return Build(builder, e);
	if (auto e = std::dynamic_pointer_cast<MemExpr>(expr))
		return Build(builder, e);
	if (auto e = std::dynamic_pointer_cast<NumExpr>(expr))
		return Build(builder, e);
	if (auto e = std::dynamic_pointer_cast<StrExpr>(expr))
		return Build(builder, e);
	if (auto e = std::dynamic_pointer_cast<UnExpr>(expr))
		return Build(builder, e);
	if (auto e = std::dynamic_pointer_cast<VarArgExpr>(expr))
		return Build(builder, e);

	throw;
}

csawir::Value* csaw::Build(csawir::Builder& builder, const std::shared_ptr<BinExpr>& expr)
{
	auto left = Build(builder, expr->Left);
	auto right = Build(builder, expr->Right);

	return builder.AppendCall(builder.GetContext().GetFunction(expr->Operator, { left->GetType(),right->GetType() }), { left, right });
}

csawir::Value* csaw::Build(csawir::Builder& builder, const std::shared_ptr<CallExpr>& expr)
{
	std::vector<csawir::Type*> argtypes;
	std::vector<csawir::Value*> args;
	for (auto& arg : expr->Arguments)
	{
		auto a = Build(builder, arg);
		args.push_back(a);
		argtypes.push_back(a->GetType());
	}

	std::string callee;
	csawir::Value* object = nullptr;

	if (auto e = std::dynamic_pointer_cast<IdExpr>(expr->Function))
	{
		callee = e->Value;
	}
	else if (auto e = std::dynamic_pointer_cast<MemExpr>(expr->Function))
	{
		callee = e->Member;
		object = Build(builder, e->Object);
	}

	auto function = builder.GetContext().GetFunction(callee, argtypes);
	if (!function)
		throw;

	return builder.AppendCall(function, args);
}

csawir::Value* csaw::Build(csawir::Builder& builder, const std::shared_ptr<ChrExpr>& expr)
{
	throw;
}

csawir::Value* csaw::Build(csawir::Builder& builder, const std::shared_ptr<ConExpr>& expr)
{
	auto condition = Build(builder, expr->Condition);
	auto vthen = Build(builder, expr->Then);
	auto velse = Build(builder, expr->Else);
	return builder.AppendCall(builder.GetContext().GetFunction("?", { condition->GetType(), vthen->GetType(), velse->GetType() }), { condition, vthen,velse });
}

csawir::Value* csaw::Build(csawir::Builder& builder, const std::shared_ptr<IdExpr>& expr)
{
	return VALUES[expr->Value];
}

csawir::Value* csaw::Build(csawir::Builder& builder, const std::shared_ptr<IndexExpr>& expr)
{
	throw;
}

csawir::Value* csaw::Build(csawir::Builder& builder, const std::shared_ptr<LambdaExpr>& expr)
{
	throw;
}

csawir::Value* csaw::Build(csawir::Builder& builder, const std::shared_ptr<MemExpr>& expr)
{
	throw;
}

csawir::Value* csaw::Build(csawir::Builder& builder, const std::shared_ptr<NumExpr>& expr)
{
	return builder.CreateNum(expr->Value);
}

csawir::Value* csaw::Build(csawir::Builder& builder, const std::shared_ptr<StrExpr>& expr)
{
	throw;
}

csawir::Value* csaw::Build(csawir::Builder& builder, const std::shared_ptr<UnExpr>& expr)
{
	throw;
}

csawir::Value* csaw::Build(csawir::Builder& builder, const std::shared_ptr<VarArgExpr>& expr)
{
	throw;
}

csawir::Type* csaw::Build(csawir::Builder& builder, const std::shared_ptr<ASTType>& type)
{
	if (auto arr = std::dynamic_pointer_cast<ASTArrayType>(type))
		return Build(builder, arr);

	return builder.GetContext().GetType(type->Name);
}

csawir::Type* csaw::Build(csawir::Builder& builder, const std::shared_ptr<ASTArrayType>& type)
{
	return builder.GetContext().GetArrayType(Build(builder, type->Type), type->Size);
}
