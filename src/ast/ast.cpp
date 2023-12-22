#include "ast.h"

static size_t depth = 0;

std::ostream& csaw::operator<<(std::ostream& out, const ASTParameter& parameter)
{
	return out << parameter.Name << ": " << parameter.Type;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<ASTType>& type)
{
	if (auto t = std::dynamic_pointer_cast<ASTArrayType>(type))
		return out << t;

	return out << type->Name;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<ASTArrayType>& type)
{
	return out << type->Type << "[" << type->Size << "]";
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<Stmt>& stmt)
{
	if (auto s = std::dynamic_pointer_cast<AliasStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<EnclosedStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<ForStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<FunStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<IfStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<IncStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<RetStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<ThingStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<VarStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<WhileStmt>(stmt))
		return out << s;

	if (auto e = std::dynamic_pointer_cast<Expr>(stmt))
		return out << e << ';';

	throw;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<AliasStmt>& stmt)
{
	return out << "alias " << stmt->Alias << " : " << stmt->Origin << ';';
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<EnclosedStmt>& stmt)
{
	out << "{" << std::endl;
	depth++;
	for (auto& s : stmt->Body)
	{
		for (size_t d = 0; d < depth; d++)
			out << '\t';
		out << s << std::endl;
	}
	depth--;

	for (size_t d = 0; d < depth; d++)
		out << '\t';
	return out << "}";
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<ForStmt>& stmt)
{
	out << "for (";
	if (stmt->Begin)
		out << stmt->Begin;
	out << ' ';
	if (stmt->Condition)
		out << stmt->Condition;
	out << "; ";
	if (stmt->Loop)
		out << stmt->Loop;
	return out << ") " << stmt->Body;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<FunStmt>& stmt)
{
	out << (stmt->IsConstructor ? '$' : '@') << stmt->Name;
	if (!stmt->IsConstructor && !stmt->RetType->Name.empty())
		out << ": " << stmt->RetType;
	out << " (";

	bool first = true;
	for (auto& param : stmt->Parameters)
	{
		if (first) first = false;
		else out << ", ";
		out << param;
	}

	out << ") ";
	if (!stmt->VarArg.empty())
		out << stmt->VarArg << ' ';
	if (!stmt->MemberOf->Name.empty())
		out << "-> " << stmt->MemberOf << ' ';
	out << stmt->Body;

	return out;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<IfStmt>& stmt)
{
	out << "if (" << stmt->Condition << ") " << stmt->Then;
	if (!stmt->Else)
		return out;
	return out << " else " << stmt->Else;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<IncStmt>& stmt)
{
	return out << "inc \"" << stmt->Path << "\";";
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<RetStmt>& stmt)
{
	return out << "ret " << stmt->Value << ";";
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<ThingStmt>& stmt)
{
	out << "thing: " << stmt->Name;
	if (!stmt->Group.empty())
		out << " : " << stmt->Group;
	if (stmt->Fields.empty())
		return out << ';';
	out << " {" << std::endl;
	depth++;
	bool first = true;
	for (auto& field : stmt->Fields)
	{
		if (first) first = false;
		else out << "," << std::endl;
		for (size_t d = 0; d < depth; d++)
			out << '\t';
		out << field;
	}
	depth--;
	for (size_t d = 0; d < depth; d++)
		out << '\t';
	return out << std::endl << '}';
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<VarStmt>& stmt)
{
	out << stmt->Type << ' ' << stmt->Name;
	if (!stmt->Value)
		return out << ';';
	return out << " = " << stmt->Value << ';';
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<WhileStmt>& stmt)
{
	return out << "while (" << stmt->Condition << ") " << stmt->Body;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<Expr>& expr)
{
	if (auto e = std::dynamic_pointer_cast<AssignExpr>(expr))
		return out << e;
	if (auto e = std::dynamic_pointer_cast<BinExpr>(expr))
		return out << e;
	if (auto e = std::dynamic_pointer_cast<CallExpr>(expr))
		return out << e;
	if (auto e = std::dynamic_pointer_cast<ConExpr>(expr))
		return out << e;
	if (auto e = std::dynamic_pointer_cast<IdExpr>(expr))
		return out << e;
	if (auto e = std::dynamic_pointer_cast<IndexExpr>(expr))
		return out << e;
	if (auto e = std::dynamic_pointer_cast<LambdaExpr>(expr))
		return out << e;
	if (auto e = std::dynamic_pointer_cast<MemExpr>(expr))
		return out << e;
	if (auto e = std::dynamic_pointer_cast<NumExpr>(expr))
		return out << e;
	if (auto e = std::dynamic_pointer_cast<StrExpr>(expr))
		return out << e;
	if (auto e = std::dynamic_pointer_cast<UnExpr>(expr))
		return out << e;

	throw;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<AssignExpr>& expr)
{
	return out << expr->Object << " = " << expr->Value;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<BinExpr>& expr)
{
	return out << expr->Left << ' ' << expr->Operator << ' ' << expr->Right;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<CallExpr>& expr)
{
	out << expr->Function << "(";
	bool first = true;
	for (auto& arg : expr->Arguments)
	{
		if (first) first = false;
		else out << ", ";
		out << arg;
	}
	return out << ")";
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<ChrExpr>& expr)
{
	return out << '\'' << expr->Value << '\'';
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<ConExpr>& expr)
{
	return out << expr->Condition << " ? " << expr->Then << " : " << expr->Else;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<IdExpr>& expr)
{
	return out << expr->Value;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<IndexExpr>& expr)
{
	return out << expr->Object << '[' << expr->Index << ']';
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<LambdaExpr>& expr)
{
	out << '[';
	bool first = true;
	for (auto& passed : expr->Passed)
	{
		if (first) first = false;
		else out << ", ";
		out << passed;
	}
	out << "](";
	first = true;
	for (auto& param : expr->Parameters)
	{
		if (first) first = false;
		else out << ", ";
		out << param;
	}
	return out << ") " << expr->Body;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<MemExpr>& expr)
{
	return out << expr->Object << '.' << expr->Member;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<NumExpr>& expr)
{
	return out << expr->Value;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<StrExpr>& expr)
{
	return out << '"' << expr->Value << '"';
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<UnExpr>& expr)
{
	return out << expr->Operator << expr->Value;
}
