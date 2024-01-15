#include <csaw/env.h>

#include <llvm/Passes/PassBuilder.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/DCE.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>
#include <llvm/CodeGen/UnreachableBlockElim.h>

csaw::Environment::Environment(
	const std::string& path,
	const std::string& filename,
	const std::vector<std::string>& args,
	const std::vector<std::string>& flags,
	const std::map<std::string, std::string>& options)
	: m_IsGlobal(true), m_FilePath(filename)
{
	m_Context = std::make_unique<llvm::LLVMContext>();
	m_Module = std::make_unique<llvm::Module>(filename, Context());
	m_Builder = std::make_unique<llvm::IRBuilder<>>(Context());

	m_FPM = std::make_unique<llvm::FunctionPassManager>();
	m_MPM = std::make_unique<llvm::ModulePassManager>();
	m_LAM = std::make_unique<llvm::LoopAnalysisManager>();
	m_FAM = std::make_unique<llvm::FunctionAnalysisManager>();
	m_CGAM = std::make_unique<llvm::CGSCCAnalysisManager>();
	m_MAM = std::make_unique<llvm::ModuleAnalysisManager>();
	m_PIC = std::make_unique<llvm::PassInstrumentationCallbacks>();
	m_SI = std::make_unique<llvm::StandardInstrumentations>(*m_Context, true);

	m_SI->registerCallbacks(*m_PIC, m_MAM.get());

	m_FPM->addPass(llvm::PromotePass());
	m_FPM->addPass(llvm::InstCombinePass());
	m_FPM->addPass(llvm::ReassociatePass());
	m_FPM->addPass(llvm::GVNPass());
	m_FPM->addPass(llvm::SimplifyCFGPass());
	m_FPM->addPass(llvm::DCEPass());
	m_FPM->addPass(llvm::LoopSimplifyPass());
	m_FPM->addPass(llvm::UnreachableBlockElimPass());

	llvm::PassBuilder passes;
	passes.registerModuleAnalyses(*m_MAM);
	passes.registerFunctionAnalyses(*m_FAM);
	passes.crossRegisterProxies(*m_LAM, *m_FAM, *m_CGAM, *m_MAM);

	auto triple = llvm::sys::getDefaultTargetTriple();
	m_Module->setTargetTriple(triple);
}

csaw::function_t* csaw::Environment::GetFunction(const std::string& name, type_t* member_of, type_t* result, const std::vector<type_t*>& args, bool is_var_args, bool is_constructor)
{
	auto& functions = m_Functions[member_of][name];
	if (functions.empty())
		return nullptr;

	for (auto function : functions)
	{
		if (function->Result != result ||
			function->IsVarArgs != is_var_args ||
			function->IsConstructor != is_constructor)
			continue;

		if (!function->IsVarArgs &&
			function->Args.size() != args.size() ||
			function->Args.size() > args.size())
			continue;

		size_t i = 0;
		for (; i < function->Args.size(); i++)
			if (function->Args[i] != args[i])
				break;
		if (i < function->Args.size())
			continue;

		return function;
	}

	return nullptr;
}

csaw::function_t* csaw::Environment::GetFunction(const std::string& name, type_t* member_of, const std::vector<type_t*>& args)
{
	auto& functions = m_Functions[member_of][name];
	if (functions.empty())
		return nullptr;

	for (auto function : functions)
	{
		if (!function->IsVarArgs &&
			function->Args.size() != args.size() ||
			function->Args.size() > args.size())
			continue;

		size_t i = 0;
		for (; i < function->Args.size(); i++)
			if (function->Args[i] != args[i])
				break;
		if (i < function->Args.size())
			continue;

		return function;
	}

	return nullptr;
}

csaw::function_t* csaw::Environment::CreateFunction(const std::string& name, type_t* member_of, type_t* result, const std::vector<type_t*>& args, bool is_var_args, bool is_constructor)
{
	std::vector<llvm::Type*> params;
	for (auto& arg : args)
		params.push_back(*arg);

	auto functiontype = llvm::FunctionType::get(*result, params, is_var_args);
	auto function = llvm::Function::Create(functiontype, llvm::GlobalValue::ExternalLinkage, name, Module());

	auto fun = new function_t(function, result, member_of, args, is_var_args, is_constructor);
	m_Functions[member_of][name].push_back(fun);

	return fun;
}

csaw::type_t* csaw::Environment::GetType(const std::string& name)
{
	return m_Types[name];
}

csaw::type_t* csaw::Environment::GetVoidType()
{
	if (auto ty = m_Types[""])
		return ty;
	return m_Types[""] = new type_t(llvm::Type::getVoidTy(Context()));
}

csaw::type_t* csaw::Environment::GetNumType()
{
	if (auto ty = m_Types["num"])
		return ty;
	return m_Types["num"] = new type_t(llvm::Type::getDoubleTy(Context()));
}

csaw::type_t* csaw::Environment::GetChrType()
{
	if (auto ty = m_Types["chr"])
		return ty;
	return m_Types["chr"] = new type_t(llvm::Type::getInt8Ty(Context()));
}

csaw::type_t* csaw::Environment::GetStrType()
{
	if (auto ty = m_Types["str"])
		return ty;
	auto chrty = GetChrType();
	return m_Types["str"] = new type_t(llvm::PointerType::get(*chrty, 0), chrty);
}

csaw::type_t* csaw::Environment::CreateType(const std::string& name, type_t* type)
{
	return m_Types[name] = type;
}

csaw::type_t* csaw::Environment::CreateType(const std::string& name, llvm::Type* type)
{
	return CreateType(name, new type_t(type));
}

csaw::type_t* csaw::Environment::CreateType(const std::string& name, llvm::Type* type, type_t* element)
{
	return CreateType(name, new type_t(type, element));
}

csaw::type_t* csaw::Environment::CreateType(const std::string& name, llvm::Type* type, llvm::Type* element)
{
	return CreateType(name, type, new type_t(element));
}

csaw::value_t* csaw::Environment::GetNum(double value)
{
	auto type = GetNumType();
	return new value_t(llvm::ConstantFP::get(*type, value), type);
}

csaw::value_t* csaw::Environment::GetChr(char value)
{
	auto type = GetChrType();
	return new value_t(llvm::ConstantInt::get(*type, value), type);
}

csaw::value_t* csaw::Environment::GetStr(const std::string& value)
{
	auto str = Builder().CreateGlobalStringPtr(value);
	return new value_t(str, GetStrType());
}

csaw::value_t* csaw::Environment::GetEmptyValue(type_t* type)
{
	if (type == GetNumType())
		return GetNum(0.0);
	if (type == GetChrType())
		return GetChr('\00');
	if (type == GetStrType())
		return GetStr("");

	throw;
}
