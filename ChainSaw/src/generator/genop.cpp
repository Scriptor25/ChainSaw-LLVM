#include <csaw/gen.h>

csaw::value_t* csaw::GenIRAdd(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(env->Builder().CreateFAdd(*left, *right));
}

csaw::value_t* csaw::GenIRSub(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(env->Builder().CreateFSub(*left, *right));
}

csaw::value_t* csaw::GenIRMul(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(env->Builder().CreateFMul(*left, *right));
}

csaw::value_t* csaw::GenIRDiv(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(env->Builder().CreateFDiv(*left, *right));
}

csaw::value_t* csaw::GenIRRem(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(env->Builder().CreateFRem(*left, *right));
}

csaw::value_t* csaw::GenIRCmpEQ(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(env->Builder().CreateFCmpOEQ(*left, *right));
}

csaw::value_t* csaw::GenIRCmpNE(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(env->Builder().CreateFCmpONE(*left, *right));
}

csaw::value_t* csaw::GenIRCmpGT(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(env->Builder().CreateFCmpOGT(*left, *right));
}

csaw::value_t* csaw::GenIRCmpLT(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(env->Builder().CreateFCmpOLT(*left, *right));
}

csaw::value_t* csaw::GenIRCmpGE(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(env->Builder().CreateFCmpOGE(*left, *right));
}

csaw::value_t* csaw::GenIRCmpLE(const EnvPtr& env, value_t* left, value_t* right)
{
	return new value_t(env->Builder().CreateFCmpOLE(*left, *right));
}

llvm::Value* csaw::GenIRBool(const EnvPtr& env, llvm::Value* value)
{
	if (value->getType()->isDoubleTy())
		return env->Builder().CreateFCmpONE(value, llvm::ConstantFP::get(llvm::Type::getDoubleTy(env->Context()), 0.0));

	return env->Builder().CreateIsNotNull(value);
}
