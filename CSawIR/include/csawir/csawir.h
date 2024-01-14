#pragma once

#include <csawir/context.h>
#include <map>
#include <string>
#include <vector>

namespace csawir
{
	struct Type
	{
		virtual ~Type() {}
		Type(const std::string& name)
			: Name(name)
		{}

		std::string Name;
	};

	struct ArrayType : Type
	{
		ArrayType(Type* element, size_t size)
			: Element(element), Size(size), Type(element->Name + '[' + std::to_string(size) + ']')
		{}

		Type* Element;
		size_t Size;
	};

	struct FunctionType : Type
	{
		FunctionType(const std::string& name, Type* rettype, const std::vector<Type*>& args, bool varargs)
			: RetType(rettype), Args(args), VarArgs(varargs), Type(name)
		{}

		Type* RetType;
		std::vector<Type*> Args;
		bool VarArgs;
	};

	struct Value
	{
		virtual ~Value() {}
		virtual Type* GetType() const = 0;
	};

	struct RegValue : Value
	{
		RegValue(const std::string& name, Type* type)
			: Name(name), Type(type)
		{}

		Type* GetType() const override { return Type; }

		std::string Name;
		Type* Type;
	};

	struct GlobalPtr : Value
	{
		GlobalPtr(const std::string& name, Type* type)
			: Name(name), Type(type)
		{}

		Type* GetType() const override { return Type; }

		std::string Name;
		Type* Type;
	};

	struct Const : Value
	{
		static Const* Parse(Type* type, const std::string& value);
	};

	struct ConstNum : Const
	{
		ConstNum(Type* type, double value)
			: ValueType(type), Value(value)
		{}

		Type* GetType() const override { return ValueType; }

		Type* ValueType;
		double Value;
	};

	struct ConstStr : Const
	{
		ConstStr(Type* type, const std::string& value)
			: Type(type), Value(value)
		{}

		Type* GetType() const override { return Type; }

		Type* Type;
		std::string Value;
	};

	struct Global : Value
	{
		Global(const std::string& name, Const* value)
			: Name(name), Value(value)
		{}

		Type* GetType() const override { return Value->GetType(); }

		std::string Name;
		Value* Value;
	};

	struct Inst : Value
	{
		void Append(Inst* inst);

		Inst* Next = nullptr;
		Inst* Prev = nullptr;
	};

	struct Function
	{
		Function(const std::string& name, FunctionType* type)
			: Name(name), Type(type)
		{}

		bool IsDefined() const { return Entry; }
		void Append(const std::string& label, Inst* inst);

		std::string Name;
		FunctionType* Type;
		std::vector<std::string> Args;

		Inst* Entry = nullptr;
		std::map<std::string, Inst*> Labels;
	};

	struct CallInst : Inst
	{
		CallInst(Function* callee, const std::vector<Value*>& args)
			: Callee(callee), Args(args)
		{}

		Type* GetType() const override { return Callee->Type->RetType; }

		Function* Callee;
		std::vector<Value*> Args;
	};

	struct RetInst : Inst
	{
		RetInst(Value* retvalue)
			: RetValue(retvalue)
		{}

		RetInst()
			: RetInst(nullptr)
		{}

		Type* GetType() const override { return RetValue->GetType(); }

		Value* RetValue;
	};

	struct SetInst : Inst
	{
		SetInst(const std::string& name, Value* value)
			: Name(name), Value(value)
		{}

		Type* GetType() const override { return Value->GetType(); }

		std::string Name;
		Value* Value;
	};

	struct AllocInst : Inst
	{
		AllocInst(Type* type)
			: Type(type)
		{}

		Type* GetType() const override { return Type; }

		Type* Type;
	};

	struct StoreInst : Inst
	{
		StoreInst(Value* ptr, Value* value)
			: Ptr(ptr), Value(value)
		{}

		Type* GetType() const override { return nullptr; }

		Value* Ptr;
		Value* Value;
	};

	struct JmpInst : Inst
	{
		JmpInst(const std::string& label)
			: Label(label)
		{}

		Type* GetType() const override { return nullptr; }

		std::string Label;
	};

	struct CJmpInst : Inst
	{
		CJmpInst(Value* condition, const std::string& labeltrue, const std::string& labelfalse)
			: Condition(condition), LabelTrue(labeltrue), LabelFalse(labelfalse)
		{}

		Type* GetType() const override { return nullptr; }

		Value* Condition;
		std::string LabelTrue;
		std::string LabelFalse;
	};

	struct LoadInst : Inst
	{
		LoadInst(Value* ptr)
			: Ptr(ptr)
		{}

		Type* GetType() const override { return Ptr->GetType(); }

		Value* Ptr;
	};

	std::ostream& operator<<(std::ostream& out, const Type& type);

	std::ostream& operator<<(std::ostream& out, const Value& value);
	std::ostream& operator<<(std::ostream& out, const RegValue& value);
	std::ostream& operator<<(std::ostream& out, const GlobalPtr& value);

	std::ostream& operator<<(std::ostream& out, const Const& cnst);
	std::ostream& operator<<(std::ostream& out, const ConstNum& cnst);
	std::ostream& operator<<(std::ostream& out, const ConstStr& cnst);

	std::ostream& operator<<(std::ostream& out, const Global& global);

	std::ostream& operator<<(std::ostream& out, const Inst& inst);
	std::ostream& operator<<(std::ostream& out, const CallInst& inst);
	std::ostream& operator<<(std::ostream& out, const RetInst& inst);
	std::ostream& operator<<(std::ostream& out, const SetInst& inst);
	std::ostream& operator<<(std::ostream& out, const AllocInst& inst);
	std::ostream& operator<<(std::ostream& out, const StoreInst& inst);
	std::ostream& operator<<(std::ostream& out, const JmpInst& inst);
	std::ostream& operator<<(std::ostream& out, const CJmpInst& inst);
	std::ostream& operator<<(std::ostream& out, const LoadInst& inst);

	std::ostream& operator<<(std::ostream& out, const Function& function);
}
