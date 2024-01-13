#pragma once

#include "ast.h"

#include <csawir/builder.h>
#include <csawir/cir.h>

namespace csaw
{
	void Build(cir::Builder* builder, const std::shared_ptr<Stmt>& stmt);
	void Build(cir::Builder* builder, const std::shared_ptr<AliasStmt>& stmt);
	void Build(cir::Builder* builder, const std::shared_ptr<EnclosedStmt>& stmt);
	void Build(cir::Builder* builder, const std::shared_ptr<ForStmt>& stmt);
	void Build(cir::Builder* builder, const std::shared_ptr<FunStmt>& stmt);
	void Build(cir::Builder* builder, const std::shared_ptr<IfStmt>& stmt);
	void Build(cir::Builder* builder, const std::shared_ptr<IncStmt>& stmt);
	void Build(cir::Builder* builder, const std::shared_ptr<RetStmt>& stmt);
	void Build(cir::Builder* builder, const std::shared_ptr<ThingStmt>& stmt);
	void Build(cir::Builder* builder, const std::shared_ptr<VarStmt>& stmt);
	void Build(cir::Builder* builder, const std::shared_ptr<WhileStmt>& stmt);

	cir::Value* Build(cir::Builder* builder, const std::shared_ptr<Expr>& expr);
	cir::Value* Build(cir::Builder* builder, const std::shared_ptr<BinExpr>& expr);
	cir::Value* Build(cir::Builder* builder, const std::shared_ptr<CallExpr>& expr);
	cir::Value* Build(cir::Builder* builder, const std::shared_ptr<ChrExpr>& expr);
	cir::Value* Build(cir::Builder* builder, const std::shared_ptr<ConExpr>& expr);
	cir::Value* Build(cir::Builder* builder, const std::shared_ptr<IdExpr>& expr);
	cir::Value* Build(cir::Builder* builder, const std::shared_ptr<IndexExpr>& expr);
	cir::Value* Build(cir::Builder* builder, const std::shared_ptr<LambdaExpr>& expr);
	cir::Value* Build(cir::Builder* builder, const std::shared_ptr<MemExpr>& expr);
	cir::Value* Build(cir::Builder* builder, const std::shared_ptr<NumExpr>& expr);
	cir::Value* Build(cir::Builder* builder, const std::shared_ptr<StrExpr>& expr);
	cir::Value* Build(cir::Builder* builder, const std::shared_ptr<UnExpr>& expr);
	cir::Value* Build(cir::Builder* builder, const std::shared_ptr<VarArgExpr>& expr);

	cir::Type* Build(cir::Builder* builder, const std::shared_ptr<ASTType>& type);
	cir::Type* Build(cir::Builder* builder, const std::shared_ptr<ASTArrayType>& type);
}
