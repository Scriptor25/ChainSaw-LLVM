#pragma once

#include <csawir/csawir.h>

namespace csawir
{
	class Builder
	{
	public:
		Builder(Context& context)
			: m_Context(context)
		{}

		Context& GetContext() { return m_Context; }
		const Context& GetContext() const { return m_Context; }

		Function* GetInsertFunction() const { return m_InsertFunction; }
		const std::string& GetInsertLabel() const { return m_InsertLabel; }

		void StartInsert(Function* function, const std::string& label) { m_InsertFunction = function; m_InsertLabel = label; }
		void SetInsertFunction(Function* function) { m_InsertFunction = function; }
		void SetInsertLabel(const std::string& label) { m_InsertLabel = label; }

		void Append(Inst* inst);

		RetInst* AppendRet(Value* value);
		CallInst* AppendCall(Function* callee, const std::vector<Value*>& args);

		ConstNum* CreateNum(double value);

	private:
		Context& m_Context;

		Function* m_InsertFunction = nullptr;
		std::string m_InsertLabel;
	};
}
