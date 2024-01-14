#include <csaw/gen.h>

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>

csaw::type_t* csaw::GenIR(const EnvPtr& env, const TypePtr& type)
{
	if (!type) return new type_t(llvm::Type::getVoidTy(env->Context()));

	if (auto ty = std::dynamic_pointer_cast<ArrayType>(type))
		return GenIR(env, ty);

	if (env->Alias().contains(type->Name))
		return env->Alias()[type->Name];

	if (type->Name == "num") return new type_t(llvm::Type::getDoubleTy(env->Context()));
	if (type->Name == "str") return new type_t(llvm::PointerType::get(llvm::Type::getInt8Ty(env->Context()), 0), new type_t(llvm::Type::getInt8Ty(env->Context())));

	throw;
}

csaw::type_t* csaw::GenIR(const EnvPtr& env, const ArrayTypePtr& type)
{
	auto element = GenIR(env, type->Element);
	return new type_t(llvm::ArrayType::get(element->Type, type->Size), element);
}
