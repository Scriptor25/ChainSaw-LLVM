#include <csaw/generator.h>

void csaw::GenIR(const EnvPtr& env, const StmtPtr& stmt)
{
	if (auto s = std::dynamic_pointer_cast<AliasStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<EnclosedStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<ForStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<FunctionStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<IfStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<IncStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<RetStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<ThingStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<VariableStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<WhileStmt>(stmt)) return GenIR(env, s);

	if (auto e = std::dynamic_pointer_cast<Expr>(stmt))
	{
		GenIR(env, e);
		return;
	}

	throw;
}

void csaw::GenIR(const EnvPtr& env, const AliasStmtPtr& stmt)
{
	throw;
}

void csaw::GenIR(const EnvPtr& env, const EnclosedStmtPtr& stmt)
{
	throw;
}

void csaw::GenIR(const EnvPtr& env, const ForStmtPtr& stmt)
{
	throw;
}

void csaw::GenIR(const EnvPtr& env, const FunctionStmtPtr& stmt)
{
	throw;
}

void csaw::GenIR(const EnvPtr& env, const IfStmtPtr& stmt)
{
	throw;
}

void csaw::GenIR(const EnvPtr& env, const IncStmtPtr& stmt)
{
	throw;
}

void csaw::GenIR(const EnvPtr& env, const RetStmtPtr& stmt)
{
	throw;
}

void csaw::GenIR(const EnvPtr& env, const ThingStmtPtr& stmt)
{
	throw;
}

void csaw::GenIR(const EnvPtr& env, const VariableStmtPtr& stmt)
{
	throw;
}

void csaw::GenIR(const EnvPtr& env, const WhileStmtPtr& stmt)
{
	throw;
}
