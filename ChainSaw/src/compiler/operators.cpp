#include "compiler.h"

csaw::value_t csaw::OpAdd(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { Environment::Builder().CreateFAdd(left(), right()) };

	return {};
}

csaw::value_t csaw::OpSub(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { Environment::Builder().CreateFSub(left(), right()) };

	return {};
}

csaw::value_t csaw::OpMul(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { Environment::Builder().CreateFMul(left(), right()) };

	return {};
}

csaw::value_t csaw::OpDiv(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { Environment::Builder().CreateFDiv(left(), right()) };

	return {};
}

csaw::value_t csaw::OpMod(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { Environment::Builder().CreateFRem(left(), right()) };

	return {};
}

csaw::value_t csaw::OpEQ(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { BoolToNum(Environment::Builder().CreateFCmpOEQ(left(), right())) };

	return {};
}

csaw::value_t csaw::OpNE(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { BoolToNum(Environment::Builder().CreateFCmpONE(left(), right())) };

	return {};
}

csaw::value_t csaw::OpLT(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { BoolToNum(Environment::Builder().CreateFCmpOLT(left(), right())) };

	return {};
}

csaw::value_t csaw::OpGT(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { BoolToNum(Environment::Builder().CreateFCmpOGT(left(), right())) };

	return {};
}

csaw::value_t csaw::OpLTE(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { BoolToNum(Environment::Builder().CreateFCmpOLE(left(), right())) };

	return {};
}

csaw::value_t csaw::OpGTE(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { BoolToNum(Environment::Builder().CreateFCmpOGE(left(), right())) };

	return {};
}

csaw::value_t csaw::OpLAnd(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { BoolToNum(Environment::Builder().CreateLogicalAnd(NumToBool(left()), NumToBool(right()))) };

	return {};
}

csaw::value_t csaw::OpLOr(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { BoolToNum(Environment::Builder().CreateLogicalOr(NumToBool(left()), NumToBool(right()))) };

	return {};
}

csaw::value_t csaw::OpAnd(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { IntToNum(Environment::Builder().CreateAnd(NumToInt(left()), NumToInt(right()))) };

	return {};
}

csaw::value_t csaw::OpOr(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { IntToNum(Environment::Builder().CreateOr(NumToInt(left()), NumToInt(right()))) };

	return {};
}

csaw::value_t csaw::OpXOr(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { IntToNum(Environment::Builder().CreateXor(NumToInt(left()), NumToInt(right()))) };

	return {};
}

csaw::value_t csaw::OpShL(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { IntToNum(Environment::Builder().CreateShl(NumToInt(left()), NumToInt(right()))) };

	return {};
}

csaw::value_t csaw::OpShR(value_t left, value_t right)
{
	if (left()->getType()->isDoubleTy() || right()->getType()->isDoubleTy())
		return { IntToNum(Environment::Builder().CreateAShr(NumToInt(left()), NumToInt(right()))) };

	return {};
}

csaw::value_t csaw::OpNot(value_t value)
{
	if (value()->getType()->isDoubleTy())
		return { BoolToNum(Environment::Builder().CreateNot(NumToBool(value()))) };

	return {};
}

csaw::value_t csaw::OpNeg(value_t value)
{
	if (value()->getType()->isDoubleTy())
		return { Environment::Builder().CreateFNeg(value()) };

	return {};
}

csaw::value_t csaw::OpInv(value_t value)
{
	if (value()->getType()->isDoubleTy())
		return { IntToNum(Environment::Builder().CreateNot(NumToInt(value()))) };

	return {};
}
