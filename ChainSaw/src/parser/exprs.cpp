#include <csaw/parser.h>

#include <iostream>

csaw::ExprPtr csaw::Parser::NextExpr()
{
	return NextConExpr();
}

csaw::ExprPtr csaw::Parser::NextConExpr()
{
	auto expr = NextBinAndExpr();

	while (At("?")) {
		Next(); // skip ?
		auto then = NextExpr();
		ExpectAndNext(":"); // skip :
		auto else_ = NextExpr();
		expr = std::make_shared<ConExpr>(expr, then, else_);
	}

	return expr;
}

csaw::ExprPtr csaw::Parser::NextBinAndExpr()
{
	auto left = NextBinOrExpr();

	while (At("&")) {
		std::string operator_ = m_Current->Value;
		Next(); // skip operator

		if (At("=")) {
			operator_ += m_Current->Value;
			Next(); // skip =
			left = std::make_shared<BinExpr>(left, NextExpr(), operator_);
			//left = std::make_shared<AssignExpr>(left, std::make_shared<BinExpr>(left, NextExpr(), operator_));
			continue;
		}
		else if (At("&")) {
			operator_ += m_Current->Value;
			Next(); // skip operator
		}

		left = std::make_shared<BinExpr>(left, NextBinOrExpr(), operator_);
	}

	return left;
}

csaw::ExprPtr csaw::Parser::NextBinOrExpr()
{
	auto left = NextBinXOrExpr();

	while (At("|")) {
		std::string operator_ = m_Current->Value;
		Next(); // skip operator

		if (At("=")) {
			operator_ += m_Current->Value;
			Next(); // skip =
			left = std::make_shared<BinExpr>(left, NextExpr(), operator_);
			//left = std::make_shared<AssignExpr>(left, std::make_shared<BinExpr>(left, NextExpr(), operator_));
			continue;
		}
		else if (At("|")) {
			operator_ += m_Current->Value;
			Next(); // skip operator
		}

		left = std::make_shared<BinExpr>(left, NextBinXOrExpr(), operator_);
	}

	return left;
}

csaw::ExprPtr csaw::Parser::NextBinXOrExpr()
{
	auto left = NextBinCmpExpr();

	while (At("^")) {
		std::string operator_ = m_Current->Value;
		Next(); // skip operator

		if (At("=")) {
			operator_ += m_Current->Value;
			Next(); // skip =
			left = std::make_shared<BinExpr>(left, NextExpr(), operator_);
			//left = std::make_shared<AssignExpr>(left, std::make_shared<BinExpr>(left, NextExpr(), operator_));
			continue;
		}

		left = std::make_shared<BinExpr>(left, NextBinCmpExpr(), operator_);
	}

	return left;
}

csaw::ExprPtr csaw::Parser::NextBinCmpExpr()
{
	auto left = NextBinSumExpr();

	while (At("=") || At("!") || At("<") || At(">")) {
		std::string operator_ = m_Current->Value;
		Next(); // skip operator

		if (At(operator_) || At("=")) {
			operator_ += m_Current->Value;
			Next(); // skip operator
		}
		else if (operator_ == "=") {
			left = std::make_shared<BinExpr>(left, NextExpr(), operator_);
			//left = std::make_shared<AssignExpr>(left, NextExpr());
			continue;
		}

		left = std::make_shared<BinExpr>(left, NextBinSumExpr(), operator_);
	}

	return left;
}

csaw::ExprPtr csaw::Parser::NextBinSumExpr()
{
	auto left = NextBinProExpr();

	while (At("+") || At("-")) {
		std::string operator_ = m_Current->Value;
		Next(); // skip operator

		if (At("=")) {
			operator_ += m_Current->Value;
			Next(); // skip =
			left = std::make_shared<BinExpr>(left, NextExpr(), operator_);
			//left = std::make_shared<AssignExpr>(left, std::make_shared<BinExpr>(left, NextExpr(), operator_));
			continue;
		}
		else if (At(operator_)) {
			operator_ += m_Current->Value;
			Next(); // skip operator
			left = std::make_shared<UnExpr>(operator_, left);
			//left = std::make_shared<AssignExpr>(left, std::make_shared<BinExpr>(left, std::make_shared<NumExpr>(1), operator_));
			continue;
		}

		left = std::make_shared<BinExpr>(left, NextBinProExpr(), operator_);
	}

	return left;
}

csaw::ExprPtr csaw::Parser::NextBinProExpr()
{
	auto left = NextCallExpr();

	while (At("*") || At("/") || At("%")) {
		std::string operator_ = m_Current->Value;
		Next(); // skip operator

		if (At("=")) {
			operator_ += m_Current->Value;
			Next(); // skip =
			left = std::make_shared<BinExpr>(left, NextExpr(), operator_);
			//left = std::make_shared<AssignExpr>(left, std::make_shared<BinExpr>(left, NextExpr(), operator_));
			continue;
		}

		left = std::make_shared<BinExpr>(left, NextCallExpr(), operator_);
	}

	return left;
}

csaw::ExprPtr csaw::Parser::NextCallExpr()
{
	auto expr = NextIndexExpr();

	while (At("(")) {
		Next(); // skip (
		std::vector<ExprPtr> arguments;
		while (!AtEof() && !At(")")) {
			arguments.push_back(NextExpr());
			if (!At(")"))
				ExpectAndNext(","); // skip ,
		}
		ExpectAndNext(")"); // skip )

		expr = std::make_shared<CallExpr>(expr, arguments);

		if (At("["))
			expr = NextIndexExpr(expr);

		if (At("."))
			expr = NextMemExpr(expr);
	}

	return expr;
}

csaw::ExprPtr csaw::Parser::NextIndexExpr()
{
	return NextIndexExpr(NextMemExpr());
}

csaw::ExprPtr csaw::Parser::NextIndexExpr(ExprPtr expr)
{
	while (At("[")) {
		Next(); // skip [
		auto index = NextExpr();
		ExpectAndNext("]"); // skip ]
		expr = std::make_shared<IndexExpr>(expr, index);

		if (At("."))
			expr = NextMemExpr(expr);
	}

	return expr;
}

csaw::ExprPtr csaw::Parser::NextMemExpr()
{
	return NextMemExpr(NextPrimExpr());
}

csaw::ExprPtr csaw::Parser::NextMemExpr(ExprPtr expr)
{
	while (At(".")) {
		Next(); // skip .
		expr = std::make_shared<MemExpr>(expr, m_Current->Value);
		ExpectAndNext(TOKEN_IDENTIFIER);
	}

	return expr;
}

csaw::ExprPtr csaw::Parser::NextPrimExpr()
{
	if (AtEof())
	{
		std::cerr << "reached end of file in incomplete state" << std::endl;
		throw;
	}

	switch (m_Current->Type)
	{
	case TOKEN_IDENTIFIER:
	{
		auto expr = std::make_shared<IdExpr>(m_Current->Value);
		Next(); // skip id
		return expr;
	}
	case TOKEN_NUMBER:
	{
		ExprPtr expr;
		if (m_Current->Value.rfind("0x", 0) != std::string::npos)
			expr = std::make_shared<NumExpr>(m_Current->Value.substr(2), 16);
		else if (m_Current->Value.rfind("0b", 0) != std::string::npos)
			expr = std::make_shared<NumExpr>(m_Current->Value.substr(2), 2);
		else
			expr = std::make_shared<NumExpr>(m_Current->Value);
		Next(); // skip num
		return expr;
	}
	case TOKEN_STRING:
	{
		auto expr = std::make_shared<StrExpr>(m_Current->Value);
		Next(); // skip str
		return expr;
	}
	case TOKEN_CHAR:
	{
		auto expr = std::make_shared<ChrExpr>(m_Current->Value[0]);
		Next(); // skip chr
		return expr;
	}
	default:
		break;
	}

	if (m_Current->Value == "(")
	{
		Next(); // skip (
		auto expr = NextExpr();
		ExpectAndNext(")"); // skip )
		return expr;
	}

	if (m_Current->Value == "-")
	{
		Next(); // skip -
		return std::make_shared<UnExpr>("-", NextCallExpr());
	}
	if (m_Current->Value == "!")
	{
		Next(); // skip !
		return std::make_shared<UnExpr>("!", NextCallExpr());
	}
	if (m_Current->Value == "~")
	{
		Next(); // skip ~
		return std::make_shared<UnExpr>("~", NextCallExpr());
	}
	if (m_Current->Value == "?")
	{
		Next(); // skip ?
		if (At("?"))
		{
			Next(); // skip ?
			return std::make_shared<VarArgExpr>();
		}
		return std::make_shared<VarArgExpr>(NextType());
	}

	if (m_Current->Value == "[")
	{ // lambda!
		Next(); // skip [
		std::vector<IdExprPtr> passed;
		while (!AtEof() && !At("]"))
		{
			passed.push_back(std::dynamic_pointer_cast<IdExpr>(NextPrimExpr()));
			if (!At("]"))
				ExpectAndNext(","); // skip ,
		}
		ExpectAndNext("]"); // skip ]

		ExpectAndNext("("); // skip (
		std::vector<Parameter> parameters;
		while (!AtEof() && !At(")"))
		{
			parameters.push_back(NextParameter());
			if (!At(")"))
				ExpectAndNext(","); // skip ,
		}
		ExpectAndNext(")"); // skip )

		auto body = NextStmt(false);
		return std::make_shared<LambdaExpr>(passed, parameters, body);
	}

	std::cerr << "unhandled token " << m_Current << std::endl;
	throw;
}
