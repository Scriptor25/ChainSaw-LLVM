#include <csaw/compiler.h>

void csaw::Build(cir::Builder* builder, const std::shared_ptr<Stmt>& stmt)
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

void csaw::Build(cir::Builder* builder, const std::shared_ptr<AliasStmt>& stmt)
{
	throw;
}

void csaw::Build(cir::Builder* builder, const std::shared_ptr<EnclosedStmt>& stmt)
{
	throw;
}

void csaw::Build(cir::Builder* builder, const std::shared_ptr<ForStmt>& stmt)
{
	throw;
}

void csaw::Build(cir::Builder* builder, const std::shared_ptr<FunStmt>& stmt)
{
	throw;
}

void csaw::Build(cir::Builder* builder, const std::shared_ptr<IfStmt>& stmt)
{
	throw;
}

void csaw::Build(cir::Builder* builder, const std::shared_ptr<IncStmt>& stmt)
{
	throw;
}

void csaw::Build(cir::Builder* builder, const std::shared_ptr<RetStmt>& stmt)
{
	throw;
}

void csaw::Build(cir::Builder* builder, const std::shared_ptr<ThingStmt>& stmt)
{
	throw;
}

void csaw::Build(cir::Builder* builder, const std::shared_ptr<VarStmt>& stmt)
{
	throw;
}

void csaw::Build(cir::Builder* builder, const std::shared_ptr<WhileStmt>& stmt)
{
	throw;
}

cir::Value* csaw::Build(cir::Builder* builder, const std::shared_ptr<Expr>& expr)
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

cir::Value* csaw::Build(cir::Builder* builder, const std::shared_ptr<BinExpr>& expr)
{
	throw;
}

cir::Value* csaw::Build(cir::Builder* builder, const std::shared_ptr<CallExpr>& expr)
{
	throw;
}

cir::Value* csaw::Build(cir::Builder* builder, const std::shared_ptr<ChrExpr>& expr)
{
	throw;
}

cir::Value* csaw::Build(cir::Builder* builder, const std::shared_ptr<ConExpr>& expr)
{
	throw;
}

cir::Value* csaw::Build(cir::Builder* builder, const std::shared_ptr<IdExpr>& expr)
{
	throw;
}

cir::Value* csaw::Build(cir::Builder* builder, const std::shared_ptr<IndexExpr>& expr)
{
	throw;
}

cir::Value* csaw::Build(cir::Builder* builder, const std::shared_ptr<LambdaExpr>& expr)
{
	throw;
}

cir::Value* csaw::Build(cir::Builder* builder, const std::shared_ptr<MemExpr>& expr)
{
	throw;
}

cir::Value* csaw::Build(cir::Builder* builder, const std::shared_ptr<NumExpr>& expr)
{
	throw;
}

cir::Value* csaw::Build(cir::Builder* builder, const std::shared_ptr<StrExpr>& expr)
{
	throw;
}

cir::Value* csaw::Build(cir::Builder* builder, const std::shared_ptr<UnExpr>& expr)
{
	throw;
}

cir::Value* csaw::Build(cir::Builder* builder, const std::shared_ptr<VarArgExpr>& expr)
{
	throw;
}

cir::Type* csaw::Build(cir::Builder* builder, const std::shared_ptr<ASTType>& type)
{
	throw;
}

cir::Type* csaw::Build(cir::Builder* builder, const std::shared_ptr<ASTArrayType>& type)
{
	throw;
}
