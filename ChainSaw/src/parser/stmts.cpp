#include <csaw/parser.h>

csaw::StmtPtr csaw::Parser::NextStmt(bool end)
{
	if (At(";")) {
		Next(); // skip ;
		return StmtPtr();
	}

	if (At("{"))
		return NextEnclosedStmt();

	if (At("alias"))
		return NextAliasStmt(end);

	if (At("for"))
		return NextForStmt(end);

	if (At("@") || At("$"))
		return NextFunStmt();

	if (At("if"))
		return NextIfStmt();

	if (At("inc"))
		return NextIncStmt(end);

	if (At("ret"))
		return NextRetStmt(end);

	if (At("thing"))
		return NextThingStmt(end);

	if (At("while"))
		return NextWhileStmt(end);

	auto expr = NextExpr();
	auto stmt = NextVarStmt(expr, end);
	if (stmt)
		return stmt;

	if (!AtEof() && end)
		ExpectAndNext(";"); // skip ;

	return expr;
}

csaw::EnclosedStmtPtr csaw::Parser::NextEnclosedStmt()
{
	std::vector<StmtPtr> enclosed;

	ExpectAndNext("{"); // skip {
	while (!AtEof() && !At("}"))
		enclosed.push_back(NextStmt(true));
	ExpectAndNext("}"); // skip }

	return std::make_shared<EnclosedStmt>(enclosed);
}

csaw::AliasStmtPtr csaw::Parser::NextAliasStmt(bool end)
{
	std::string alias;
	TypePtr origin;

	ExpectAndNext("alias"); // skip "alias"
	alias = m_Current->Value;
	ExpectAndNext(TOKEN_IDENTIFIER); // skip alias
	ExpectAndNext(":"); // skip :
	origin = NextType();
	if (!AtEof() && end)
		ExpectAndNext(";"); // skip ;

	return std::make_shared<AliasStmt>(alias, origin);
}

csaw::ForStmtPtr csaw::Parser::NextForStmt(bool end)
{
	StmtPtr begin, loop, body;
	ExprPtr condition;

	ExpectAndNext("for"); // skip "for"
	ExpectAndNext("("); // skip (
	if (!At(";"))
		begin = NextStmt(true);
	else
		Next(); // skip ;
	if (!At(";"))
	{
		condition = NextExpr();
		ExpectAndNext(";"); // skip ;
	}
	else
		Next(); // skip ;
	if (!At(")"))
		loop = NextStmt(false);
	ExpectAndNext(")"); // skip )

	body = NextStmt(end);

	return std::make_shared<ForStmt>(begin, condition, loop, body);
}

csaw::FunStmtPtr csaw::Parser::NextFunStmt()
{
	bool constructor, vararg;
	std::string name;
	TypePtr type, member;
	std::vector<Parameter> parameters;
	EnclosedStmtPtr body;

	constructor = At("$");
	if (constructor)
		Next(); // skip $
	else
		ExpectAndNext("@"); // skip @

	if (At("(")) // override operator
	{
		Next(); // skip (
		name = "";
		while (!AtEof() && !At(")")) // at least one character
		{
			name += m_Current->Value;
			ExpectAndNext(TOKEN_OPERATOR); // skip operator
		}
		ExpectAndNext(")"); // skip )
	}
	else
	{
		name = m_Current->Value;
		ExpectAndNext(TOKEN_IDENTIFIER); // skip name
	}

	if (constructor)
	{
		type = Type::Get(name);
	}
	else if (At(":"))
	{
		Next(); // skip :
		type = NextType();
	}

	if (At("("))
	{
		Next(); // skip (
		while (!AtEof() && !At(")"))
		{
			parameters.push_back(NextParameter());
			if (!At(")"))
				ExpectAndNext(","); // skip ,
		}
		ExpectAndNext(")"); // skip )
	}

	vararg = At("?");
	if (vararg)
		Next(); // skip ?

	if (At("-"))
	{
		Next(); // skip -
		ExpectAndNext(">"); // skip >
		member = NextType();
	}

	if (At(";"))
	{
		Next(); // skip ;
		return std::make_shared<FunStmt>(constructor, name, type, parameters, vararg, member, body);
	}

	body = NextEnclosedStmt();

	return std::make_shared<FunStmt>(constructor, name, type, parameters, vararg, member, body);
}

csaw::IfStmtPtr csaw::Parser::NextIfStmt()
{
	ExprPtr condition;
	StmtPtr then, else_;

	ExpectAndNext("if"); // skip "if"
	ExpectAndNext("("); // skip (
	condition = NextExpr();
	ExpectAndNext(")"); // skip )

	then = NextStmt(true);

	if (At("else"))
	{
		Next(); // skip "else"
		else_ = NextStmt(true);
	}

	return std::make_shared<IfStmt>(condition, then, else_);
}

csaw::IncStmtPtr csaw::Parser::NextIncStmt(bool end)
{
	std::string path;

	ExpectAndNext("inc"); // skip "inc"
	path = m_Current->Value;
	ExpectAndNext(TOKEN_STRING); // skip path
	if (!AtEof() && end)
		ExpectAndNext(";"); // skip ;

	return std::make_shared<IncStmt>(path);
}

csaw::RetStmtPtr csaw::Parser::NextRetStmt(bool end)
{
	ExprPtr value;

	ExpectAndNext("ret"); // skip "ret"
	if (!At(";"))
		value = NextExpr();
	if (!AtEof() && end)
		ExpectAndNext(";"); // skip ;

	return std::make_shared<RetStmt>(value);
}

csaw::ThingStmtPtr csaw::Parser::NextThingStmt(bool end)
{
	std::string name, group = "";
	std::vector<Parameter> fields;

	ExpectAndNext("thing"); // skip "thing"
	ExpectAndNext(":"); // skip :
	name = m_Current->Value;
	ExpectAndNext(TOKEN_IDENTIFIER); // skip name

	if (At(":"))
	{
		Next(); // skip :
		group = m_Current->Value;
		ExpectAndNext(TOKEN_IDENTIFIER);
	}

	if (At(";"))
	{
		Next(); // skip ;
		return std::make_shared<ThingStmt>(name, group, fields);
	}

	ExpectAndNext("{"); // skip {
	while (!AtEof() && !At("}"))
	{
		fields.push_back(NextParameter());
		if (!At("}"))
			ExpectAndNext(","); // skip ,
	}
	ExpectAndNext("}"); // skip }

	return std::make_shared<ThingStmt>(name, group, fields);
}

csaw::WhileStmtPtr csaw::Parser::NextWhileStmt(bool end)
{
	ExprPtr condition;
	StmtPtr body;

	ExpectAndNext("while"); // skip "while"
	ExpectAndNext("("); // skip (
	condition = NextExpr();
	ExpectAndNext(")"); // skip )

	body = NextStmt(end);

	return std::make_shared<WhileStmt>(condition, body);
}

csaw::VarStmtPtr csaw::Parser::NextVarStmt(const ExprPtr& expr, bool end)
{
	if ((std::dynamic_pointer_cast<IdExpr>(expr) || std::dynamic_pointer_cast<IndexExpr>(expr)) && At(TOKEN_IDENTIFIER))
	{
		TypePtr type;
		std::string name;
		ExprPtr value;

		type = NextType(expr);
		name = m_Current->Value;
		ExpectAndNext(TOKEN_IDENTIFIER); // skip name

		if (At(";"))
		{
			Next();
			return std::make_shared<VarStmt>(type, name, value);
		}

		ExpectAndNext("="); // skip =

		value = NextExpr();
		if (!AtEof() && end)
			ExpectAndNext(";"); // skip ;

		return std::make_shared<VarStmt>(type, name, value);
	}

	return VarStmtPtr();
}
