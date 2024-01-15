#include <csaw/ast.h>

static size_t depth = 0;

std::ostream& csaw::operator<<(std::ostream& out, const StmtPtr& stmt)
{
	if (!stmt)
		return out;

	if (auto s = std::dynamic_pointer_cast<AliasStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<EnclosedStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<ForStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<FunctionStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<IfStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<IncStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<RetStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<ThingStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<VariableStmt>(stmt))
		return out << s;
	if (auto s = std::dynamic_pointer_cast<WhileStmt>(stmt))
		return out << s;

	if (auto e = std::dynamic_pointer_cast<Expr>(stmt))
		return out << e << ';';

	throw;
}

std::ostream& csaw::operator<<(std::ostream& out, const AliasStmtPtr& stmt)
{
	return *stmt >> out;
}

std::ostream& csaw::operator<<(std::ostream& out, const EnclosedStmtPtr& stmt)
{
	return *stmt >> out;
}

std::ostream& csaw::operator<<(std::ostream& out, const ForStmtPtr& stmt)
{
	return *stmt >> out;
}

std::ostream& csaw::operator<<(std::ostream& out, const FunctionStmtPtr& stmt)
{
	return *stmt >> out;
}

std::ostream& csaw::operator<<(std::ostream& out, const IfStmtPtr& stmt)
{
	return *stmt >> out;
}

std::ostream& csaw::operator<<(std::ostream& out, const IncStmtPtr& stmt)
{
	return *stmt >> out;
}

std::ostream& csaw::operator<<(std::ostream& out, const RetStmtPtr& stmt)
{
	return *stmt >> out;
}

std::ostream& csaw::operator<<(std::ostream& out, const ThingStmtPtr& stmt)
{
	return *stmt >> out;
}

std::ostream& csaw::operator<<(std::ostream& out, const VariableStmtPtr& stmt)
{
	return *stmt >> out;
}

std::ostream& csaw::operator<<(std::ostream& out, const WhileStmtPtr& stmt)
{
	return *stmt >> out;
}

std::ostream& csaw::AliasStmt::operator>>(std::ostream& out) const
{
	return out << "alias " << Alias << " : " << Origin << ';';
}

std::ostream& csaw::EnclosedStmt::operator>>(std::ostream& out) const
{
	out << "{" << std::endl;
	depth++;
	for (auto& s : Body)
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

std::ostream& csaw::ForStmt::operator>>(std::ostream& out) const
{
	out << "for (";
	if (Begin)
		out << Begin;
	out << ' ';
	if (Condition)
		out << Condition;
	out << "; ";
	if (Loop)
		out << Loop;
	return out << ") " << Body;
}

std::ostream& csaw::FunctionStmt::operator>>(std::ostream& out) const
{
	out << (IsConstructor ? '$' : '@') << Name;
	if (!IsConstructor && RetType)
		out << ": " << RetType;

	if (!Parameters.empty())
	{
		out << " (";
		bool first = true;
		for (auto& param : Parameters)
		{
			if (first) first = false;
			else out << ", ";
			out << param;
		}
		out << ")";
	}

	if (IsVarArg)
		out << " ?";
	if (MemberOf)
		out << " -> " << MemberOf;
	if (Body)
		out << ' ' << Body;

	return out;
}

std::ostream& csaw::IfStmt::operator>>(std::ostream& out) const
{
	out << "if (" << Condition << ") " << True;
	if (!False)
		return out;
	return out << " else " << False;
}

std::ostream& csaw::IncStmt::operator>>(std::ostream& out) const
{
	return out << "inc \"" << Path << "\";";
}

std::ostream& csaw::RetStmt::operator>>(std::ostream& out) const
{
	return out << "ret " << Value << ";";
}

std::ostream& csaw::ThingStmt::operator>>(std::ostream& out) const
{
	out << "thing: " << Name;
	if (!Group.empty())
		out << " : " << Group;
	if (Fields.empty())
		return out << ';';
	out << " {" << std::endl;
	depth++;
	bool first = true;
	for (auto& field : Fields)
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

std::ostream& csaw::VariableStmt::operator>>(std::ostream& out) const
{
	out << Type << ' ' << Name;
	if (!Value)
		return out << ';';
	return out << " = " << Value << ';';
}

std::ostream& csaw::WhileStmt::operator>>(std::ostream& out) const
{
	return out << "while (" << Condition << ") " << Body;
}
