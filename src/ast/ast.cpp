#include "ast.h"

std::ostream& csaw::operator<<(std::ostream& out, const ASTParameter& parameter)
{
	return out << parameter.Name << ": " << parameter.Type;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<ASTType>& type)
{
	if (auto t = std::dynamic_pointer_cast<ASTArrayType>(type))
		return out << t;

	return out << type->Name;
}

std::ostream& csaw::operator<<(std::ostream& out, const std::shared_ptr<ASTArrayType>& type)
{
	return out << type->Type << "[" << type->Size << "]";
}
