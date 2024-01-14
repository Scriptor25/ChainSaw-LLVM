#include <csawir/context.h>

csawir::Context::Context()
{
	CreateFunction("+", GetFunctionType(GetNumType(), { GetNumType(), GetNumType() }, false));
	CreateFunction("-", GetFunctionType(GetNumType(), { GetNumType(), GetNumType() }, false));
	CreateFunction("*", GetFunctionType(GetNumType(), { GetNumType(), GetNumType() }, false));
	CreateFunction("/", GetFunctionType(GetNumType(), { GetNumType(), GetNumType() }, false));
	CreateFunction("==", GetFunctionType(GetNumType(), { GetNumType(), GetNumType() }, false));
	CreateFunction("&&", GetFunctionType(GetNumType(), { GetNumType(), GetNumType() }, false));
	CreateFunction("!=", GetFunctionType(GetNumType(), { GetStrType(), GetStrType() }, false));
	CreateFunction("?", GetFunctionType(GetNumType(), { GetNumType(), GetNumType(), GetNumType() }, false));
	CreateFunction("random", GetFunctionType(GetNumType(), {}, false));
	CreateFunction("floor", GetFunctionType(GetNumType(), { GetNumType() }, false));
	CreateFunction("num", GetFunctionType(GetNumType(), { GetStrType() }, false));
	CreateFunction("printf", GetFunctionType(GetVoidType(), { GetStrType() }, true));
	CreateFunction("readf", GetFunctionType(GetStrType(), { GetStrType() }, true));
}

csawir::Type* csawir::Context::GetType(const std::string& name)
{
	if (auto ty = m_Types[name])
		return ty;

	if (name == "void")
		return GetVoidType();
	if (name == "num")
		return GetNumType();
	if (name == "chr")
		return GetChrType();
	if (name == "str")
		return GetStrType();

	if (name.find('[') != std::string::npos)
	{
		auto idx = name.find_last_of('[');
		Type* element = GetType(name.substr(0, idx));
		auto substr = name.substr(idx + 1);
		substr = substr.substr(0, substr.length() - 1);
		size_t size = std::stoull(substr);
		return GetArrayType(element, size);
	}

	return nullptr;
}

csawir::Type* csawir::Context::GetOrCreateType(const std::string& name, TypeSupplier create)
{
	if (auto ty = m_Types[name])
		return ty;
	return m_Types[name] = create();
}

csawir::Type* csawir::Context::GetVoidType()
{
	return GetOrCreateType("void", []() { return new Type("void"); });
}

csawir::Type* csawir::Context::GetNumType()
{
	return GetOrCreateType("num", []() { return new Type("num"); });
}

csawir::Type* csawir::Context::GetChrType()
{
	return GetOrCreateType("chr", []() { return new Type("chr"); });
}

csawir::Type* csawir::Context::GetStrType()
{
	return GetOrCreateType("str", []() { return new Type("str"); });
}

csawir::ArrayType* csawir::Context::GetArrayType(Type* element, size_t size)
{
	std::string name(element->Name + '[' + std::to_string(size) + ']');
	return (ArrayType*)GetOrCreateType(name, [element, size]() { return new ArrayType(element, size); });
}

csawir::FunctionType* csawir::Context::GetFunctionType(Type* rettype, const std::vector<Type*>& args, bool varargs)
{
	std::string argstr;
	bool first = true;
	for (auto arg : args)
	{
		if (first)first = false;
		else argstr += ", ";
		argstr += arg->Name;
	}

	std::string name(rettype->Name + '(' + argstr + ')' + (varargs ? "?" : ""));
	return (FunctionType*)GetOrCreateType(name, [name, rettype, args, varargs]() { return new FunctionType(name, rettype, args, varargs); });
}

csawir::Type* csawir::Context::GetThingType(const std::string& name)
{
	return nullptr;
}

csawir::Type* csawir::Context::CreateThingType(const std::string& name, const std::map<std::string, Type*>& elements)
{
	return nullptr;
}

csawir::Function* csawir::Context::GetFunction(const std::string& name, FunctionType* type)
{
	return m_Functions[name][type];
}

csawir::Function* csawir::Context::GetFunction(const std::string& name, const std::vector<Type*>& args)
{
	auto& functions = m_Functions[name];
	for (auto& funpair : functions)
	{
		auto funtype = funpair.first;

		if (funtype->Args.size() > args.size() || (funtype->Args.size() < args.size() && !funtype->VarArgs))
			continue;

		size_t i;
		for (i = 0; i < funtype->Args.size(); i++)
			if (funtype->Args[i] != args[i])
				break;
		if (i != funtype->Args.size())
			continue;

		return funpair.second;
	}
	return nullptr;
}

csawir::Function* csawir::Context::CreateFunction(const std::string& name, FunctionType* type)
{
	auto function = new Function(name, type);
	m_OrderedFunctions.push_back(function);
	return m_Functions[name][type] = function;
}

std::ostream& csawir::operator<<(std::ostream& out, const Context& context)
{
	out << "---------- Types: ----------" << std::endl;
	for (auto& pairnametype : context.GetTypes())
		out << pairnametype.first << ": " << *pairnametype.second << std::endl;
	out << "----------------------------" << std::endl << std::endl;

	for (auto function : context.GetFunctions())
		out << *function << std::endl;

	return out;
}
