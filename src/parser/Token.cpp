#include "parser.h"

std::ostream& csaw::operator<<(std::ostream& out, const Token& token)
{
	return out << "[ '" << token.Value << "' -> " << token.Type << " (" << token.Line << ") ]";
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<Token>& token)
{
	return out << "[ '" << token->Value << "' -> " << token->Type << " (" << token->Line << ") ]";
}

std::ostream& csaw::operator<<(std::ostream& out, const TokenType& type)
{
	switch (type)
	{
	case TOKEN_EOF: return out << "EOF";
	case TOKEN_IDENTIFIER: return out << "IDENTIFIER";
	case TOKEN_NUMBER: return out << "NUMBER";
	case TOKEN_STRING: return out << "STRING";
	case TOKEN_CHAR: return out << "CHAR";
	case TOKEN_OPERATOR: return out << "OPERATOR";
	default: return out << "WTF";
	}
}
