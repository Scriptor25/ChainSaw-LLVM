#include <csaw/ast.h>

#include <map>

csaw::TypePtr csaw::Type::GetAny()
{
	return Get("any");
}

csaw::TypePtr csaw::Type::GetNum()
{
	return Get("num");
}

csaw::TypePtr csaw::Type::GetStr()
{
	return Get("str");
}

csaw::TypePtr csaw::Type::GetChr()
{
	return Get("chr");
}

csaw::TypePtr csaw::Type::GetLambda()
{
	return Get("lambda");
}

csaw::TypePtr csaw::Type::Get(const std::string& name)
{
	static std::map<std::string, TypePtr> types;

	auto& type = types[name];
	if (!type)
		return types[name] = TypePtr(new Type(name));

	return type;
}

csaw::ArrayTypePtr csaw::Type::Get(const std::string& name, const size_t size)
{
	return Get(Get(name), size);
}

csaw::ArrayTypePtr csaw::Type::Get(const TypePtr& type, const size_t size)
{
	static std::map<TypePtr, ArrayTypePtr> types;

	auto& arraytype = types[type];
	if (!arraytype)
		return arraytype = ArrayTypePtr(new ArrayType(type, size));

	return arraytype;
}
