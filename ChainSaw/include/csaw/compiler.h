#pragma once

#include "ast.h"

#include <csawir/builder.h>
#include <csawir/csawir.h>

namespace csaw
{
	void Build(csawir::Builder* builder, const std::shared_ptr<Stmt>& stmt);
	void Build(csawir::Builder* builder, const std::shared_ptr<AliasStmt>& stmt);
	void Build(csawir::Builder* builder, const std::shared_ptr<EnclosedStmt>& stmt);
	void Build(csawir::Builder* builder, const std::shared_ptr<ForStmt>& stmt);
	void Build(csawir::Builder* builder, const std::shared_ptr<FunStmt>& stmt);
	void Build(csawir::Builder* builder, const std::shared_ptr<IfStmt>& stmt);
	void Build(csawir::Builder* builder, const std::shared_ptr<IncStmt>& stmt);
	void Build(csawir::Builder* builder, const std::shared_ptr<RetStmt>& stmt);
	void Build(csawir::Builder* builder, const std::shared_ptr<ThingStmt>& stmt);
	void Build(csawir::Builder* builder, const std::shared_ptr<VarStmt>& stmt);
	void Build(csawir::Builder* builder, const std::shared_ptr<WhileStmt>& stmt);

	csawir::Value* Build(csawir::Builder* builder, const std::shared_ptr<Expr>& expr);
	csawir::Value* Build(csawir::Builder* builder, const std::shared_ptr<BinExpr>& expr);
	csawir::Value* Build(csawir::Builder* builder, const std::shared_ptr<CallExpr>& expr);
	csawir::Value* Build(csawir::Builder* builder, const std::shared_ptr<ChrExpr>& expr);
	csawir::Value* Build(csawir::Builder* builder, const std::shared_ptr<ConExpr>& expr);
	csawir::Value* Build(csawir::Builder* builder, const std::shared_ptr<IdExpr>& expr);
	csawir::Value* Build(csawir::Builder* builder, const std::shared_ptr<IndexExpr>& expr);
	csawir::Value* Build(csawir::Builder* builder, const std::shared_ptr<LambdaExpr>& expr);
	csawir::Value* Build(csawir::Builder* builder, const std::shared_ptr<MemExpr>& expr);
	csawir::Value* Build(csawir::Builder* builder, const std::shared_ptr<NumExpr>& expr);
	csawir::Value* Build(csawir::Builder* builder, const std::shared_ptr<StrExpr>& expr);
	csawir::Value* Build(csawir::Builder* builder, const std::shared_ptr<UnExpr>& expr);
	csawir::Value* Build(csawir::Builder* builder, const std::shared_ptr<VarArgExpr>& expr);

	csawir::Type* Build(csawir::Builder* builder, const std::shared_ptr<ASTType>& type);
	csawir::Type* Build(csawir::Builder* builder, const std::shared_ptr<ASTArrayType>& type);
}
