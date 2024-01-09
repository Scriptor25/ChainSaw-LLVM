#include "compiler.h"

csaw::type_t csaw::GenIR(const std::shared_ptr<ASTType>& type)
{
	if (auto t = std::dynamic_pointer_cast<ASTArrayType>(type))
		return GenIR(t);

	if (!type)
		return Environment::Builder().getVoidTy();

	// num = double
	// chr = i8
	// str = i8*
	// {}  = {}*

	if (type->Name == "num")
		return type_t(Environment::Builder().getDoubleTy());
	if (type->Name == "chr")
		return type_t(Environment::Builder().getInt8Ty());
	if (type->Name == "str")
		return type_t(llvm::PointerType::get(Environment::Builder().getInt8Ty(), 0), Environment::Builder().getInt8Ty());

	if (auto t = llvm::StructType::getTypeByName(Environment::Context(), type->Name))
		return type_t(llvm::PointerType::get(t, 0), t);

	throw "TODO";
}

csaw::type_t csaw::GenIR(const std::shared_ptr<ASTArrayType>& type)
{
	auto t = GenIR(type->Type);
	return type_t(llvm::ArrayType::get(t.type, type->Size), t.element);
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
