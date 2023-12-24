#include "ast.h"

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<Expr>& expr)
{
	if (auto e = std::dynamic_pointer_cast<AssignExpr>(expr))
		return *e.get() >> out;
	if (auto e = std::dynamic_pointer_cast<BinExpr>(expr))
		return *e.get() >> out;
	if (auto e = std::dynamic_pointer_cast<CallExpr>(expr))
		return *e.get() >> out;
	if (auto e = std::dynamic_pointer_cast<ConExpr>(expr))
		return *e.get() >> out;
	if (auto e = std::dynamic_pointer_cast<IdExpr>(expr))
		return *e.get() >> out;
	if (auto e = std::dynamic_pointer_cast<IndexExpr>(expr))
		return *e.get() >> out;
	if (auto e = std::dynamic_pointer_cast<LambdaExpr>(expr))
		return *e.get() >> out;
	if (auto e = std::dynamic_pointer_cast<MemExpr>(expr))
		return *e.get() >> out;
	if (auto e = std::dynamic_pointer_cast<NumExpr>(expr))
		return *e.get() >> out;
	if (auto e = std::dynamic_pointer_cast<StrExpr>(expr))
		return *e.get() >> out;
	if (auto e = std::dynamic_pointer_cast<UnExpr>(expr))
		return *e.get() >> out;

	throw;
}

std::ostream& csaw::AssignExpr::operator>>(std::ostream& out) const
{
	return out << Object << " = " << Value;
}

Agnode_t* csaw::AssignExpr::operator>>(Agraph_t* g) const
{
	static size_t index = 0;

	auto node = agnode(g, (char*)("AssignExpr" + std::to_string(index++)).c_str(), 1);
	agset(node, (char*)"label", "=");

	auto object = *Object.get() >> g;
	auto value = *Value.get() >> g;

	agedge(g, node, object, 0, 1);
	agedge(g, node, value, 0, 1);

	return node;
}

std::ostream& csaw::BinExpr::operator>>(std::ostream& out) const
{
	return out << Left << ' ' << Operator << ' ' << Right;
}

Agnode_t* csaw::BinExpr::operator>>(Agraph_t* g) const
{
	static size_t index = 0;

	auto node = agnode(g, (char*)("BinExpr" + std::to_string(index++)).c_str(), 1);
	agset(node, (char*)"label", Operator.c_str());

	auto left = *Left.get() >> g;
	auto right = *Right.get() >> g;

	agedge(g, node, left, 0, 1);
	agedge(g, node, right, 0, 1);

	return node;
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

Agnode_t* csaw::CallExpr::operator>>(Agraph_t* g) const
{
	static size_t index = 0;

	auto node = agnode(g, (char*)("CallExpr" + std::to_string(index++)).c_str(), 1);
	agset(node, (char*)"label", "CallExpr");

	auto func = *Function.get() >> g;
	agedge(g, node, func, 0, 1);

	for (auto& arg : Arguments)
	{
		auto a = *arg.get() >> g;
		agedge(g, node, a, 0, 1);
	}

	return node;
}

std::ostream& csaw::ChrExpr::operator>>(std::ostream& out) const
{
	return out << '\'' << Value << '\'';
}

Agnode_t* csaw::ChrExpr::operator>>(Agraph_t* g) const
{
	return agnode(g, (char*)&Value, 1);
}

std::ostream& csaw::ConExpr::operator>>(std::ostream& out) const
{
	return out << Condition << " ? " << Then << " : " << Else;
}

Agnode_t* csaw::ConExpr::operator>>(Agraph_t* g) const
{
	static size_t index = 0;

	auto node = agnode(g, (char*)("ConExpr" + std::to_string(index++)).c_str(), 1);
	agset(node, (char*)"label", "ConExpr");

	auto condition = *Condition.get() >> g;
	auto then = *Then.get() >> g;
	auto else_ = *Else.get() >> g;

	agedge(g, node, condition, 0, 1);
	agedge(g, node, then, 0, 1);
	agedge(g, node, else_, 0, 1);

	return node;
}

std::ostream& csaw::IdExpr::operator>>(std::ostream& out) const
{
	return out << Value;
}

Agnode_t* csaw::IdExpr::operator>>(Agraph_t* g) const
{
	return agnode(g, (char*)Value.c_str(), 1);
}

std::ostream& csaw::IndexExpr::operator>>(std::ostream& out) const
{
	return out << Object << '[' << Index << ']';
}

Agnode_t* csaw::IndexExpr::operator>>(Agraph_t* g) const
{
	static size_t index = 0;

	auto node = agnode(g, (char*)("IndexExpr" + std::to_string(index++)).c_str(), 1);
	agset(node, (char*)"label", "IndexExpr");

	auto object = *Object.get() >> g;
	auto idx = *Index.get() >> g;

	auto edge1 = agedge(g, node, object, 0, 1);
	auto edge2 = agedge(g, node, idx, 0, 1);

	return node;
}

std::ostream& csaw::LambdaExpr::operator>>(std::ostream& out) const
{
	out << '[';
	bool first = true;
	for (auto& passed : Passed)
	{
		if (first) first = false;
		else out << ", ";
		*passed.get() >> out;
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

Agnode_t* csaw::LambdaExpr::operator>>(Agraph_t* g) const
{
	static size_t index = 0;

	auto node = agnode(g, (char*)("LambdaExpr" + std::to_string(index++)).c_str(), 1);
	agset(node, (char*)"label", "LambdaExpr");

	for (auto& passed : Passed)
	{
		auto p = *passed.get() >> g;
		agedge(g, node, p, 0, 1);
	}

	for (auto& param : Parameters)
	{
		auto p = param >> g;
		agedge(g, node, p, 0, 1);
	}

	auto body = *Body.get() >> g;
	agedge(g, node, body, 0, 1);

	return node;
}

std::ostream& csaw::MemExpr::operator>>(std::ostream& out) const
{
	return out << Object << '.' << Member;
}

Agnode_t* csaw::MemExpr::operator>>(Agraph_t* g) const
{
	static size_t index = 0;

	auto node = agnode(g, (char*)("MemExpr" + std::to_string(index++)).c_str(), 1);
	agset(node, (char*)"label", "MemExpr");

	auto object = *Object.get() >> g;
	auto member = agnode(g, (char*)Member.c_str(), 1);

	agedge(g, node, object, 0, 1);
	agedge(g, node, member, 0, 1);

	return node;
}

std::ostream& csaw::NumExpr::operator>>(std::ostream& out) const
{
	return out << Value;
}

Agnode_t* csaw::NumExpr::operator>>(Agraph_t* g) const
{
	return agnode(g, (char*)std::to_string(Value).c_str(), 1);
}

std::ostream& csaw::StrExpr::operator>>(std::ostream& out) const
{
	return out << '"' << Value << '"';
}

Agnode_t* csaw::StrExpr::operator>>(Agraph_t* g) const
{
	return agnode(g, (char*)Value.c_str(), 1);
}

std::ostream& csaw::UnExpr::operator>>(std::ostream& out) const
{
	return out << Operator << Value;
}

Agnode_t* csaw::UnExpr::operator>>(Agraph_t* g) const
{
	static size_t index = 0;

	auto node = agnode(g, (char*)("UnExpr" + std::to_string(index++)).c_str(), 1);
	agset(node, (char*)"label", "UnExpr");

	auto op = agnode(g, (char*)Operator.c_str(), 1);
	auto value = *Value.get() >> g;

	agedge(g, node, op, 0, 1);
	agedge(g, node, value, 0, 1);

	return node;
}
