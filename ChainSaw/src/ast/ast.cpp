#include <csaw/ast.h>

#include <iostream>

std::ostream& csaw::operator<<(std::ostream& out, const Parameter& parameter)
{
	return out << parameter.Name << ": " << parameter.Type;
}

std::ostream& csaw::operator<<(std::ostream& out, const TypePtr& type)
{
	if (auto t = std::dynamic_pointer_cast<ArrayType>(type))
		return out << t;

	return out << type->Name;
}

std::ostream& csaw::operator<<(std::ostream& out, const ArrayTypePtr& type)
{
	return out << type->Element << "[" << type->Size << "]";
}
