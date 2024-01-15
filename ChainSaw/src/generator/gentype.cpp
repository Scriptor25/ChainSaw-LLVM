#include <csaw/gen.h>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

csaw::type_t* csaw::GenIR(const EnvPtr& env, const TypePtr& type)
{
	if (!type) return env->GetVoidType();

	if (auto ty = env->GetType(type->Name))
		return ty;

	if (auto ty = std::dynamic_pointer_cast<ArrayType>(type))
		return GenIR(env, ty);

	if (env->Alias().contains(type->Name))
		return env->Alias()[type->Name];

	if (type->Name == "num")
	{
		return env->CreateType(type->Name, llvm::Type::getDoubleTy(env->Context()));
	}
	if (type->Name == "str")
	{
		auto i8ty = llvm::Type::getInt8Ty(env->Context());
		return env->CreateType(type->Name, llvm::PointerType::get(i8ty, 0), i8ty);
	}

	throw;
}

csaw::type_t* csaw::GenIR(const EnvPtr& env, const ArrayTypePtr& type)
{
	if (auto ty = env->GetType(type->Name))
		return ty;

	auto element = GenIR(env, type->Element);
	return env->CreateType(type->Name, llvm::ArrayType::get(element->Type, type->Size), element);
}
