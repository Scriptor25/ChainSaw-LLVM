#include "parser.h"

std::ostream& csaw::Token::operator<<(std::ostream& out) const
{
	return out << "[ '" << Value << "' -> " << Type << " (" << Line << ") ]";
}
