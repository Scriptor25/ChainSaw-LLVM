#include <csawir/parser.h>

std::ostream& csawir::operator<<(std::ostream& out, const Token& token)
{
	return out << "[ '" << token.Value << "' -> " << token.Type << " (" << token.Line << ") ]";
}

std::ostream& csawir::operator<<(std::ostream& out, const TokenType& type)
{
	switch (type)
	{
	case TOKEN_EOF: return out << "EOF";
	case TOKEN_IDENTIFIER: return out << "IDENTIFIER";
	case TOKEN_NUMBER: return out << "NUMBER";
	case TOKEN_STRING: return out << "STRING";
	case TOKEN_CHAR: return out << "CHAR";
	case TOKEN_OPERATOR: return out << "OPERATOR";
	case TOKEN_REGISTER: return out << "REGISTER";
	case TOKEN_GLOBAL: return out << "GLOBAL";
	default: return out << "WTF";
	}
}
