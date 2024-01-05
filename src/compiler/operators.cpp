#include "compiler.h"

llvm::Value* csaw::OpAdd(llvm::Value* left, llvm::Value* right)
{
	if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
		return Environment::Builder().CreateFAdd(left, right);

	return nullptr;
}

llvm::Value* csaw::OpSub(llvm::Value* left, llvm::Value* right)
{
	if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
		return Environment::Builder().CreateFSub(left, right);

	return nullptr;
}

llvm::Value* csaw::OpMul(llvm::Value* left, llvm::Value* right)
{
	if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
		return Environment::Builder().CreateFMul(left, right);

	return nullptr;
}

llvm::Value* csaw::OpDiv(llvm::Value* left, llvm::Value* right)
{
	if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
		return Environment::Builder().CreateFDiv(left, right);

	return nullptr;
}

llvm::Value* csaw::OpEqu(llvm::Value* left, llvm::Value* right)
{
	if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
		return Environment::Builder().CreateFCmpOEQ(left, right);

	if (left->getType()->isPointerTy() && right->getType()->isPointerTy())
	{
		auto t = llvm::IntegerType::get(Environment::Context(), 64);
		left = Environment::Builder().CreatePtrToInt(left, t);
		right = Environment::Builder().CreatePtrToInt(right, t);
		return Environment::Builder().CreateICmpEQ(left, right);
	}

	return nullptr;
}

llvm::Value* csaw::OpNeq(llvm::Value* left, llvm::Value* right)
{
	if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
		return Environment::Builder().CreateFCmpONE(left, right);

	if (left->getType()->isPointerTy() && right->getType()->isPointerTy())
	{
		auto t = llvm::IntegerType::get(Environment::Context(), 64);
		left = Environment::Builder().CreatePtrToInt(left, t);
		right = Environment::Builder().CreatePtrToInt(right, t);
		return Environment::Builder().CreateICmpNE(left, right);
	}

	return nullptr;
}

llvm::Value* csaw::OpAnd(llvm::Value* left, llvm::Value* right)
{
	if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy())
		return Environment::Builder().CreateAnd(left, right);

	return nullptr;
}

llvm::Value* csaw::OpOr(llvm::Value* left, llvm::Value* right)
{
	if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy())
		return Environment::Builder().CreateOr(left, right);

	return nullptr;
}

llvm::Value* csaw::OpNot(llvm::Value* value)
{
	if (value->getType()->isIntegerTy())
		return Environment::Builder().CreateNot(value);
	if (value->getType()->isDoubleTy())
		return Environment::Builder().CreateFCmpOEQ(value, llvm::ConstantFP::get(llvm::Type::getDoubleTy(Environment::Context()), 0.0));

	return nullptr;
}

llvm::Value* csaw::OpNeg(llvm::Value* value)
{
	if (value->getType()->isDoubleTy())
		return Environment::Builder().CreateNeg(value);

	return nullptr;
}

llvm::Value* csaw::OpInv(llvm::Value* value)
{
	if (value->getType()->isIntegerTy())
		return Environment::Builder().CreateNot(value);

	return nullptr;
}
