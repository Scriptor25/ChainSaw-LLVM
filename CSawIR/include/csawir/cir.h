#pragma once

#include <string>
#include <vector>

namespace cir
{
	struct Type
	{
		virtual ~Type() {}
	};

	struct FunctionType : Type
	{
		Type* RetType;
		std::vector<Type*> Args;
	};

	struct Value
	{
		virtual ~Value() {}
		virtual Type* GetType() const = 0;
	};

	struct Function
	{
		std::string Name;
		FunctionType* Type;
	};

	struct Inst : Value
	{
	};

	struct CallInst : Inst
	{
		Type* GetType() const override { return Callee->Type->RetType; };

		Function* Callee;
		std::vector<Value*> Args;
	};

	struct RetInst : Inst
	{
		Type* GetType() const override { return RetValue->GetType(); };

		Value* RetValue;
	};
}
