#include <csaw/generator.h>

llvm::Value* csaw::GenIR(const EnvPtr& env, const ExprPtr& expr)
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

llvm::Value* csaw::GenIR(const EnvPtr& env, const BinaryExprPtr& expr)
{
	throw;
}

llvm::Value* csaw::GenIR(const EnvPtr& env, const CallExprPtr& expr)
{
	throw;
}

llvm::Value* csaw::GenIR(const EnvPtr& env, const ChrExprPtr& expr)
{
	throw;
}

llvm::Value* csaw::GenIR(const EnvPtr& env, const IdExprPtr& expr)
{
	throw;
}

llvm::Value* csaw::GenIR(const EnvPtr& env, const IndexExprPtr& expr)
{
	throw;
}

llvm::Value* csaw::GenIR(const EnvPtr& env, const LambdaExprPtr& expr)
{
	throw;
}

llvm::Value* csaw::GenIR(const EnvPtr& env, const MemberExprPtr& expr)
{
	throw;
}

llvm::Value* csaw::GenIR(const EnvPtr& env, const NumExprPtr& expr)
{
	throw;
}

llvm::Value* csaw::GenIR(const EnvPtr& env, const SelectExprPtr& expr)
{
	throw;
}

llvm::Value* csaw::GenIR(const EnvPtr& env, const StrExprPtr& expr)
{
	throw;
}

llvm::Value* csaw::GenIR(const EnvPtr& env, const UnaryExprPtr& expr)
{
	throw;
}

llvm::Value* csaw::GenIR(const EnvPtr& env, const VarArgExprPtr& expr)
{
	throw;
}
