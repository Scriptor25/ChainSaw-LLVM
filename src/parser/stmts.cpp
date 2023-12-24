#include "parser.h"

std::shared_ptr<csaw::Stmt> csaw::Parser::NextStmt(bool end)
{
	if (At(";")) {
		Next(); // skip ;
		return std::shared_ptr<Stmt>();
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

std::shared_ptr<csaw::EnclosedStmt> csaw::Parser::NextEnclosedStmt()
{
	std::vector<std::shared_ptr<Stmt>> enclosed;

	ExpectAndNext("{"); // skip {
	while (!AtEof() && !At("}"))
		enclosed.push_back(NextStmt(true));
	ExpectAndNext("}"); // skip }

	return std::make_shared<EnclosedStmt>(enclosed);
}

std::shared_ptr<csaw::AliasStmt> csaw::Parser::NextAliasStmt(bool end)
{
	std::string alias;
	std::shared_ptr<ASTType> origin;

	ExpectAndNext("alias"); // skip "alias"
	alias = m_Current->Value;
	ExpectAndNext(TOKEN_IDENTIFIER); // skip alias
	ExpectAndNext(":"); // skip :
	origin = NextType();
	if (!AtEof() && end)
		ExpectAndNext(";"); // skip ;

	return std::make_shared<AliasStmt>(alias, origin);
}

std::shared_ptr<csaw::ForStmt> csaw::Parser::NextForStmt(bool end)
{
	std::shared_ptr<Stmt> begin, loop, body;
	std::shared_ptr<Expr> condition;

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

std::shared_ptr<csaw::FunStmt> csaw::Parser::NextFunStmt()
{
	bool constructor;
	std::string name, vararg;
	std::shared_ptr<ASTType> type = ASTType::GetNull(), member = ASTType::GetNull();
	std::vector<ASTParameter> parameters;
	std::shared_ptr<EnclosedStmt> body;

	constructor = At("$");
	if (constructor)
		Next(); // skip $
	else
		ExpectAndNext("@"); // skip @

	if (At("(")) { // override operator
		Next(); // skip (
		name = "";
		while (!AtEof() && !At(")")) { // at least one character
			name += m_Current->Value;
			ExpectAndNext(TOKEN_OPERATOR); // skip operator
		}
		ExpectAndNext(")"); // skip )
	}
	else {
		name = m_Current->Value;
		ExpectAndNext(TOKEN_IDENTIFIER); // skip name
	}

	if (constructor) {
		type = ASTType::Get(name);
	}
	else if (At(":")) {
		Next(); // skip :
		type = NextType();
	}

	if (At("(")) {
		Next(); // skip (
		while (!AtEof() && !At(")")) {
			parameters.push_back(NextParameter());
			if (!At(")"))
				ExpectAndNext(","); // skip ,
		}
		ExpectAndNext(")"); // skip )
	}

	if (At("$")) {
		Next(); // skip $
		vararg = m_Current->Value;
		ExpectAndNext(TOKEN_IDENTIFIER); // skip vararg name
	}

	if (At("-")) {
		Next(); // skip -
		ExpectAndNext(">"); // skip >
		member = NextType();
	}

	if (At(";")) {
		Next(); // skip ;
		return std::make_shared<FunStmt>(constructor, name, type, parameters, vararg, member, body);
	}

	body = NextEnclosedStmt();

	return std::make_shared<FunStmt>(constructor, name, type, parameters, vararg, member, body);
}

std::shared_ptr<csaw::IfStmt> csaw::Parser::NextIfStmt()
{
	std::shared_ptr<Expr> condition;
	std::shared_ptr<Stmt> then, else_;

	ExpectAndNext("if"); // skip "if"
	ExpectAndNext("("); // skip (
	condition = NextExpr();
	ExpectAndNext(")"); // skip )

	then = NextStmt(true);

	if (At("else")) {
		Next(); // skip "else"
		else_ = NextStmt(true);
	}

	return std::make_shared<IfStmt>(condition, then, else_);
}

std::shared_ptr<csaw::IncStmt> csaw::Parser::NextIncStmt(bool end)
{
	std::string path;

	ExpectAndNext("inc"); // skip "inc"
	path = m_Current->Value;
	ExpectAndNext(TOKEN_STRING); // skip path
	if (!AtEof() && end)
		ExpectAndNext(";"); // skip ;

	return std::make_shared<IncStmt>(path);
}

std::shared_ptr<csaw::RetStmt> csaw::Parser::NextRetStmt(bool end)
{
	std::shared_ptr<Expr> value;

	ExpectAndNext("ret"); // skip "ret"
	if (!At(";"))
		value = NextExpr();
	if (!AtEof() && end)
		ExpectAndNext(";"); // skip ;

	return std::make_shared<RetStmt>(value);
}

std::shared_ptr<csaw::ThingStmt> csaw::Parser::NextThingStmt(bool end)
{
	std::string name, group = "";
	std::vector<ASTParameter> fields;

	ExpectAndNext("thing"); // skip "thing"
	ExpectAndNext(":"); // skip :
	name = m_Current->Value;
	ExpectAndNext(TOKEN_IDENTIFIER); // skip name

	if (At(":")) {
		Next(); // skip :
		group = m_Current->Value;
		ExpectAndNext(TOKEN_IDENTIFIER);
	}

	if (At(";")) {
		Next(); // skip ;
		return std::make_shared<ThingStmt>(name, group, fields);
	}

	ExpectAndNext("{"); // skip {
	while (!AtEof() && !At("}")) {
		fields.push_back(NextParameter());
		if (!At("}"))
			ExpectAndNext(","); // skip ,
	}
	ExpectAndNext("}"); // skip }

	return std::make_shared<ThingStmt>(name, group, fields);
}

std::shared_ptr<csaw::WhileStmt> csaw::Parser::NextWhileStmt(bool end)
{
	std::shared_ptr<Expr> condition;
	std::shared_ptr<Stmt> body;

	ExpectAndNext("while"); // skip "while"
	ExpectAndNext("("); // skip (
	condition = NextExpr();
	ExpectAndNext(")"); // skip )

	body = NextStmt(end);

	return std::make_shared<WhileStmt>(condition, body);
}

std::shared_ptr<csaw::VarStmt> csaw::Parser::NextVarStmt(const std::shared_ptr<Expr>& expr, bool end)
{
	if ((std::dynamic_pointer_cast<IdExpr>(expr) || std::dynamic_pointer_cast<IndexExpr>(expr)) && At(TOKEN_IDENTIFIER)) {

		std::shared_ptr<ASTType> type;
		std::string name;
		std::shared_ptr<Expr> value;

		type = NextType(expr);
		name = m_Current->Value;
		ExpectAndNext(TOKEN_IDENTIFIER); // skip name

		if (At(";")) {
			Next();
			return std::make_shared<VarStmt>(type, name, value);
		}

		ExpectAndNext("="); // skip =

		value = NextExpr();
		if (!AtEof() && end)
			ExpectAndNext(";"); // skip ;

		return std::make_shared<VarStmt>(type, name, value);
	}

	return std::shared_ptr<VarStmt>();
}
