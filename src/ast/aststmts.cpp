#include "ast.h"

static size_t depth = 0;

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<Stmt>& stmt)
{
	if (!stmt)
		return out;

	if (auto s = std::dynamic_pointer_cast<AliasStmt>(stmt))
		return *s.get() >> out;
	if (auto s = std::dynamic_pointer_cast<EnclosedStmt>(stmt))
		return *s.get() >> out;
	if (auto s = std::dynamic_pointer_cast<ForStmt>(stmt))
		return *s.get() >> out;
	if (auto s = std::dynamic_pointer_cast<FunStmt>(stmt))
		return *s.get() >> out;
	if (auto s = std::dynamic_pointer_cast<IfStmt>(stmt))
		return *s.get() >> out;
	if (auto s = std::dynamic_pointer_cast<IncStmt>(stmt))
		return *s.get() >> out;
	if (auto s = std::dynamic_pointer_cast<RetStmt>(stmt))
		return *s.get() >> out;
	if (auto s = std::dynamic_pointer_cast<ThingStmt>(stmt))
		return *s.get() >> out;
	if (auto s = std::dynamic_pointer_cast<VarStmt>(stmt))
		return *s.get() >> out;
	if (auto s = std::dynamic_pointer_cast<WhileStmt>(stmt))
		return *s.get() >> out;

	if (auto e = std::dynamic_pointer_cast<Expr>(stmt))
		return out << e << ';';

	throw;
}

std::ostream& csaw::AliasStmt::operator>>(std::ostream& out) const
{
	return out << "alias " << Alias << " : " << Origin << ';';
}

Agnode_t* csaw::AliasStmt::operator>>(Agraph_t* g) const
{
	static size_t index = 0;

	auto node = agnode(g, (char*)("AliasStmt" + std::to_string(index++)).c_str(), 1);
	agset(node, (char*)"label", "AliasStmt");

	auto alias = agnode(g, (char*)Alias.c_str(), 1);
	auto origin = *Origin.get() >> g;

	agedge(g, node, alias, 0, 1);
	agedge(g, node, origin, 0, 1);

	return node;
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

Agnode_t* csaw::EnclosedStmt::operator>>(Agraph_t* g) const
{
	static size_t index = 0;

	auto s = agsubg(g, (char*)("cluster_" + std::to_string(index)).c_str(), 1);
	auto node = agnode(s, (char*)("EnclosedStmt" + std::to_string(index++)).c_str(), 1);
	agset(node, (char*)"label", "EnclosedStmt");

	for (auto& stmt : Body)
		*stmt.get() >> s;

	return node;
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

Agnode_t* csaw::ForStmt::operator>>(Agraph_t* g) const
{
	static size_t index = 0;

	auto node = agnode(g, (char*)("ForStmt" + std::to_string(index++)).c_str(), 1);
	agset(node, (char*)"label", "ForStmt");

	auto begin = *Begin.get() >> g;
	auto condition = *Condition.get() >> g;
	auto loop = *Loop.get() >> g;
	auto body = *Body.get() >> g;

	agedge(g, node, begin, 0, 1);
	agedge(g, node, condition, 0, 1);
	agedge(g, node, loop, 0, 1);
	agedge(g, node, body, 0, 1);

	return node;
}

std::ostream& csaw::FunStmt::operator>>(std::ostream& out) const
{
	out << (IsConstructor ? '$' : '@') << Name;
	if (!IsConstructor && !RetType->Name.empty())
		out << ": " << RetType;
	out << " (";

	bool first = true;
	for (auto& param : Parameters)
	{
		if (first) first = false;
		else out << ", ";
		out << param;
	}

	out << ") ";
	if (!VarArg.empty())
		out << VarArg << ' ';
	if (!MemberOf->Name.empty())
		out << "-> " << MemberOf << ' ';
	return *Body.get() >> out;
}

Agnode_t* csaw::FunStmt::operator>>(Agraph_t* g) const
{
	static size_t index = 0;

	auto node = agnode(g, (char*)("FunStmt" + std::to_string(index++)).c_str(), 1);
	agset(node, (char*)"label", Name.c_str());

	if (IsConstructor)
	{
		auto isconstr = agnode(g, (char*)("IsConstructor" + std::to_string(index)).c_str(), 1);
		agset(isconstr, (char*)"label", "Is Constructor");
		agedge(g, node, isconstr, 0, 1);
	}

	auto rettype = *RetType.get() >> g;
	auto vararg = agnode(g, (char*)("VarArg" + std::to_string(index)).c_str(), 1);
	agset(vararg, (char*)"label", VarArg.c_str());
	auto memberof = *MemberOf.get() >> g;
	auto body = *Body.get() >> g;

	agedge(g, node, rettype, 0, 1);

	for (auto& param : Parameters)
	{
		auto p = param >> g;
		agedge(g, node, p, 0, 1);
	}

	agedge(g, node, vararg, 0, 1);
	agedge(g, node, memberof, 0, 1);
	agedge(g, node, body, 0, 1);

	return node;
}

std::ostream& csaw::IfStmt::operator>>(std::ostream& out) const
{
	out << "if (" << Condition << ") " << Then;
	if (!Else)
		return out;
	return out << " else " << Else;
}

Agnode_t* csaw::IfStmt::operator>>(Agraph_t* g) const
{
	return nullptr;
}

std::ostream& csaw::IncStmt::operator>>(std::ostream& out) const
{
	return out << "inc \"" << Path << "\";";
}

Agnode_t* csaw::IncStmt::operator>>(Agraph_t* g) const
{
	return nullptr;
}

std::ostream& csaw::RetStmt::operator>>(std::ostream& out) const
{
	return out << "ret " << Value << ";";
}

Agnode_t* csaw::RetStmt::operator>>(Agraph_t* g) const
{
	return nullptr;
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

Agnode_t* csaw::ThingStmt::operator>>(Agraph_t* g) const
{
	return nullptr;
}

std::ostream& csaw::VarStmt::operator>>(std::ostream& out) const
{
	out << Type << ' ' << Name;
	if (!Value)
		return out << ';';
	return out << " = " << Value << ';';
}

Agnode_t* csaw::VarStmt::operator>>(Agraph_t* g) const
{
	return nullptr;
}

std::ostream& csaw::WhileStmt::operator>>(std::ostream& out) const
{
	return out << "while (" << Condition << ") " << Body;
}

Agnode_t* csaw::WhileStmt::operator>>(Agraph_t* g) const
{
	return nullptr;
}
