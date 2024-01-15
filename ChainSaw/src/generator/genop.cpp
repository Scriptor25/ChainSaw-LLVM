#include <csaw/gen.h>

csaw::value_t* csaw::GenIRAdd(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(env->Builder().CreateFAdd(*left, *right), env->GetNumType());
}

csaw::value_t* csaw::GenIRSub(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(env->Builder().CreateFSub(*left, *right), env->GetNumType());
}

csaw::value_t* csaw::GenIRMul(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(env->Builder().CreateFMul(*left, *right), env->GetNumType());
}

csaw::value_t* csaw::GenIRDiv(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(env->Builder().CreateFDiv(*left, *right), env->GetNumType());
}

csaw::value_t* csaw::GenIRRem(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(env->Builder().CreateFRem(*left, *right), env->GetNumType());
}

csaw::value_t* csaw::GenIRCmpEQ(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(GenIRNum(env, env->Builder().CreateFCmpOEQ(*left, *right)), env->GetNumType());
}

csaw::value_t* csaw::GenIRCmpNE(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(GenIRNum(env, env->Builder().CreateFCmpONE(*left, *right)), env->GetNumType());
}

csaw::value_t* csaw::GenIRCmpGT(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(GenIRNum(env, env->Builder().CreateFCmpOGT(*left, *right)), env->GetNumType());
}

csaw::value_t* csaw::GenIRCmpLT(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(GenIRNum(env, env->Builder().CreateFCmpOLT(*left, *right)), env->GetNumType());
}

csaw::value_t* csaw::GenIRCmpGE(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(GenIRNum(env, env->Builder().CreateFCmpOGE(*left, *right)), env->GetNumType());
}

csaw::value_t* csaw::GenIRCmpLE(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(GenIRNum(env, env->Builder().CreateFCmpOLE(*left, *right)), env->GetNumType());
}

csaw::value_t* csaw::GenIRLogAnd(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(GenIRNum(env, env->Builder().CreateLogicalAnd(GenIRBool(env, *left), GenIRBool(env, *right))), env->GetNumType());
}

csaw::value_t* csaw::GenIRLogOr(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(GenIRNum(env, env->Builder().CreateLogicalOr(GenIRBool(env, *left), GenIRBool(env, *right))), env->GetNumType());
}

csaw::value_t* csaw::GenIRNot(const EnvPtr& env, value_t* value)
{
	return new value_t(GenIRNum(env, env->Builder().CreateNot(GenIRBool(env, *value))), env->GetNumType());
}

csaw::value_t* csaw::GenIRNeg(const EnvPtr& env, value_t* value)
{
	return new value_t(GenIRNum(env, env->Builder().CreateFNeg(*value)), env->GetNumType());
}

csaw::value_t* csaw::GenIRInv(const EnvPtr& env, value_t* value)
{
	return new value_t(GenIRNum(env, env->Builder().CreateNot(*value)), env->GetNumType());
}

llvm::Value* csaw::GenIRBool(const EnvPtr& env, llvm::Value* value)
{
	if (value->getType()->isDoubleTy())
		return env->Builder().CreateFCmpONE(value, llvm::ConstantFP::get(llvm::Type::getDoubleTy(env->Context()), 0.0));

	throw;
}

llvm::Value* csaw::GenIRNum(const EnvPtr& env, llvm::Value* value)
{
	if (value->getType()->isDoubleTy())
		return value;
	if (value->getType()->isIntegerTy())
		return env->Builder().CreateUIToFP(value, env->Builder().getDoubleTy());
	if (value->getType()->isPointerTy())
		return GenIRNum(env, env->Builder().CreatePtrToInt(value, env->Builder().getInt64Ty()));

	throw;
}
