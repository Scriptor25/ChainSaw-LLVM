#include "parser.h"

std::ostream& csaw::operator<<(std::ostream& out, const Token& token)
{
	return out << "[ '" << token.Value << "' -> " << token.Type << " (" << token.Line << ") ]";
}
