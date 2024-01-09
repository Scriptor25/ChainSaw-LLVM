#include "ast.h"

#include <map>

std::shared_ptr<csaw::ASTType> csaw::ASTType::GetAny()
{
	return Get("any");
}

std::shared_ptr<csaw::ASTType> csaw::ASTType::GetNum()
{
	return Get("num");
}

std::shared_ptr<csaw::ASTType> csaw::ASTType::GetStr()
{
	return Get("str");
}

std::shared_ptr<csaw::ASTType> csaw::ASTType::GetChr()
{
	return Get("chr");
}

std::shared_ptr<csaw::ASTType> csaw::ASTType::GetLambda()
{
	return Get("lambda");
}

std::shared_ptr<csaw::ASTType> csaw::ASTType::Get(const std::string& name)
{
	static std::map<std::string, std::shared_ptr<ASTType>> types;

	auto& type = types[name];
	if (!type)
		return types[name] = std::shared_ptr<ASTType>(new ASTType(name));

	return type;
}

std::shared_ptr<csaw::ASTArrayType> csaw::ASTType::Get(const std::string& name, const size_t size)
{
	return Get(Get(name), size);
}

std::shared_ptr<csaw::ASTArrayType> csaw::ASTType::Get(const std::shared_ptr<ASTType>& type, const size_t size)
{
	static std::map<std::shared_ptr<ASTType>, std::shared_ptr<ASTArrayType>> types;

	auto& arraytype = types[type];
	if (!arraytype)
		return arraytype = std::shared_ptr<ASTArrayType>(new ASTArrayType(type, size));

	return arraytype;
}
