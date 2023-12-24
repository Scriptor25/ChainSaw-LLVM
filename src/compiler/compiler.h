#pragma once

#include "../ast/ast.h"

#include <llvm/IR/Value.h>

namespace csaw
{
	class Environment
	{

	};

	llvm::Value* Gen(const std::shared_ptr<Environment>& env, const std::shared_ptr<Stmt>& stmt);
}
