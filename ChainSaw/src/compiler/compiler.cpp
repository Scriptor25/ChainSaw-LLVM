#include <csaw/compiler.h>

csaw::type_t csaw::GenIR(const TypePtr& type)
{
	if (auto t = std::dynamic_pointer_cast<ArrayType>(type))
		return GenIR(t);

	if (!type)
		return type_t("", Environment::Builder().getVoidTy());

	return GenIR(type->Name);
}

csaw::type_t csaw::GenIR(const std::string& type)
{
	// any = opaque*
	// num = double
	// chr = i8
	// str = i8*
	// {}  = {}*

	if (Environment::HasAlias(type))
		return Environment::GetAlias(type);

	if (type == "any")
		return type_t(type, Environment::Builder().getPtrTy());
	if (type == "num")
		return type_t(type, Environment::Builder().getDoubleTy());
	if (type == "chr")
		return type_t(type, Environment::Builder().getInt8Ty());
	if (type == "str")
	{
		auto i8ty = Environment::Builder().getInt8Ty();
		return type_t(type, llvm::PointerType::get(i8ty, 0), i8ty);
	}

	if (auto t = Environment::GetType(type))
		return type_t(type, llvm::PointerType::get(t->type, 0), t->type);

	llvm::errs() << "Undefined type '" << type << "'\r\n";
	throw;
}

csaw::type_t csaw::GenIR(const ArrayTypePtr& type)
{
	auto t = GenIR(type->Element);
	return type_t(t.name, llvm::ArrayType::get(t.type, type->Size), t.element);
}

llvm::Value* csaw::BoolToNum(llvm::Value* value)
{
	return Environment::Builder().CreateUIToFP(value, llvm::Type::getDoubleTy(Environment::Context()));
}

llvm::Value* csaw::NumToBool(llvm::Value* value)
{
	return Environment::Builder().CreateFPToUI(value, llvm::Type::getInt1Ty(Environment::Context()));
}

llvm::Value* csaw::IntToNum(llvm::Value* value)
{
	return Environment::Builder().CreateSIToFP(value, llvm::Type::getDoubleTy(Environment::Context()));
}

llvm::Value* csaw::NumToInt(llvm::Value* value)
{
	return Environment::Builder().CreateFPToSI(value, llvm::Type::getInt64Ty(Environment::Context()));
}
