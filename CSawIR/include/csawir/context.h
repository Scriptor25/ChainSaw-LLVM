#pragma once

#include <csawir/csawir.h>
#include <functional>
#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace csawir
{
	struct Type;
	struct ArrayType;
	struct Function;
	struct FunctionType;

	typedef std::function<Type* ()> TypeSupplier;

	class Context
	{
	public:
		Context();

		Type* GetType(const std::string& name);
		Type* GetOrCreateType(const std::string& name, TypeSupplier create);

		Type* GetVoidType();
		Type* GetNumType();
		Type* GetChrType();
		Type* GetStrType();

		ArrayType* GetArrayType(Type* element, size_t size);

		FunctionType* GetFunctionType(Type* rettype, const std::vector<Type*>& args, bool varargs);

		Type* GetThingType(const std::string& name);
		Type* CreateThingType(const std::string& name, const std::map<std::string, Type*>& elements);

		Function* GetFunction(const std::string& name, FunctionType* type);
		Function* GetFunction(const std::string& name, const std::vector<Type*>& args);
		Function* CreateFunction(const std::string& name, FunctionType* type);

		const std::map<std::string, Type*>& GetTypes() const { return m_Types; }
		const std::vector<Function*>& GetFunctions() const { return m_OrderedFunctions; }

	private:
		std::map<std::string, Type*> m_Types;
		std::vector<Function*> m_OrderedFunctions;
		std::map<std::string, std::map<FunctionType*, Function*>> m_Functions;
	};

	std::ostream& operator<<(std::ostream& out, const Context& context);
}
