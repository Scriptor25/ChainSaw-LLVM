#include <csaw/ast.h>

std::ostream& csaw::operator<<(std::ostream& out, const ExprPtr& expr)
{
	if (auto e = std::dynamic_pointer_cast<BinaryExpr>(expr))
		return *e >> out;
	if (auto e = std::dynamic_pointer_cast<CallExpr>(expr))
		return *e >> out;
	if (auto e = std::dynamic_pointer_cast<SelectExpr>(expr))
		return *e >> out;
	if (auto e = std::dynamic_pointer_cast<IdExpr>(expr))
		return *e >> out;
	if (auto e = std::dynamic_pointer_cast<IndexExpr>(expr))
		return *e >> out;
	if (auto e = std::dynamic_pointer_cast<LambdaExpr>(expr))
		return *e >> out;
	if (auto e = std::dynamic_pointer_cast<MemberExpr>(expr))
		return *e >> out;
	if (auto e = std::dynamic_pointer_cast<NumExpr>(expr))
		return *e >> out;
	if (auto e = std::dynamic_pointer_cast<StrExpr>(expr))
		return *e >> out;
	if (auto e = std::dynamic_pointer_cast<UnaryExpr>(expr))
		return *e >> out;
	if (auto e = std::dynamic_pointer_cast<VarArgExpr>(expr))
		return *e >> out;

	throw;
}

std::ostream& csaw::BinaryExpr::operator>>(std::ostream& out) const
{
	return out << Left << ' ' << Operator << ' ' << Right;
}

std::ostream& csaw::CallExpr::operator>>(std::ostream& out) const
{
	out << Function << "(";
	bool first = true;
	for (auto& arg : Arguments)
	{
		if (first) first = false;
		else out << ", ";
		out << arg;
	}
	return out << ")";
}

std::ostream& csaw::ChrExpr::operator>>(std::ostream& out) const
{
	return out << '\'' << Value << '\'';
}

std::ostream& csaw::SelectExpr::operator>>(std::ostream& out) const
{
	return out << Condition << " ? " << True << " : " << False;
}

std::ostream& csaw::IdExpr::operator>>(std::ostream& out) const
{
	return out << Value;
}

std::ostream& csaw::IndexExpr::operator>>(std::ostream& out) const
{
	return out << Object << '[' << Index << ']';
}

std::ostream& csaw::LambdaExpr::operator>>(std::ostream& out) const
{
	out << '[';
	bool first = true;
	for (auto& passed : Passed)
	{
		if (first) first = false;
		else out << ", ";
		*passed >> out;
	}
	out << "](";
	first = true;
	for (auto& param : Parameters)
	{
		if (first) first = false;
		else out << ", ";
		out << param;
	}
	return out << ") " << Body;
}

std::ostream& csaw::MemberExpr::operator>>(std::ostream& out) const
{
	return out << Object << '.' << Member;
}

std::ostream& csaw::NumExpr::operator>>(std::ostream& out) const
{
	return out << Value;
}

std::ostream& csaw::StrExpr::operator>>(std::ostream& out) const
{
	return out << '"' << Value << '"';
}

std::ostream& csaw::UnaryExpr::operator>>(std::ostream& out) const
{
	return out << Operator << Value;
}

std::ostream& csaw::VarArgExpr::operator>>(std::ostream& out) const
{
	return out << "?" << Type;
}
