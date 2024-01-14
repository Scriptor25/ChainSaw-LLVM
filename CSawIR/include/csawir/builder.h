#pragma once

#include <csawir/csawir.h>

namespace csawir
{
	class Builder
	{
	public:
		ConstNum* CreateNum(double value);

		CallInst* CreateCall(Function* callee, const std::vector<Value*>& args);
		RetInst* CreateRet(Value* value);
	};
}
