#include "../lang/csawstd.h"
#include "compiler.h"

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>

std::map<std::string, std::vector<llvm::Function*>> csaw::Environment::m_Functions;

std::unique_ptr<llvm::LLVMContext> csaw::Environment::m_Context;
std::unique_ptr<llvm::IRBuilder<>> csaw::Environment::m_Builder;
std::unique_ptr<llvm::Module> csaw::Environment::m_Module;

std::unique_ptr<llvm::FunctionPassManager> csaw::Environment::m_FPM;
std::unique_ptr<llvm::LoopAnalysisManager> csaw::Environment::m_LAM;
std::unique_ptr<llvm::FunctionAnalysisManager> csaw::Environment::m_FAM;
std::unique_ptr<llvm::CGSCCAnalysisManager> csaw::Environment::m_CGAM;
std::unique_ptr<llvm::ModuleAnalysisManager> csaw::Environment::m_MAM;
std::unique_ptr<llvm::PassInstrumentationCallbacks> csaw::Environment::m_PIC;
std::unique_ptr<llvm::StandardInstrumentations> csaw::Environment::m_SI;

llvm::Value* csaw::Environment::CreateCall(const std::string& name, std::vector<llvm::Value*> args)
{
	std::vector<llvm::Type*> types;
	for (auto arg : args)
		types.push_back(arg->getType());

	auto fun = GetFunction(name, types);
	if (!fun)
		throw;

	return Environment::Builder().CreateCall(fun, args);
}

bool csaw::Environment::Compile(const std::string& filename)
{
	auto triple = llvm::sys::getDefaultTargetTriple();

	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();

	std::string error;
	auto target = llvm::TargetRegistry::lookupTarget(triple, error);

	if (!target)
	{
		llvm::errs() << "failed to get target for triple '" << triple << "': " << error;
		return false;
	}

	auto CPU = "generic";
	auto FEATURES = "";

	llvm::TargetOptions opt;
	auto machine = target->createTargetMachine(triple, CPU, FEATURES, opt, llvm::Reloc::PIC_);

	m_Module->setDataLayout(machine->createDataLayout());
	m_Module->setTargetTriple(triple);

	std::error_code ec;
	llvm::raw_fd_ostream dst(filename, ec, llvm::sys::fs::OF_None);

	if (ec)
	{
		llvm::errs() << "failed to open file '" << filename << "': " << ec.message();
		return false;
	}

	llvm::legacy::PassManager pm;
	auto type = llvm::CodeGenFileType::ObjectFile;

	if (machine->addPassesToEmitFile(pm, dst, nullptr, type))
	{
		llvm::errs() << "failed to emit file of type '" << (type == llvm::CodeGenFileType::AssemblyFile ? "AssemblyFile" : type == llvm::CodeGenFileType::ObjectFile ? "ObjectFile" : "Null") << "'";
		return false;
	}

	pm.run(*m_Module);
	dst.flush();

	return true;
}

double csaw::Environment::Run()
{
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetAsmParser();

	if (llvm::verifyModule(*m_Module, &llvm::errs())) {
		llvm::errs() << "Error in the module. Aborting.\n";
		return 1;
	}

	// Create an LLJIT instance
	auto builder = llvm::orc::LLJITBuilder().create();
	auto& jit = builder.get();
	if (!jit) {
		llvm::errs() << "Failed to create LLJIT instance. Aborting.\n";
		return 1;
	}

	auto& dylib = jit->getMainJITDylib();
	llvm::orc::MangleAndInterner mangle(dylib.getExecutionSession(), jit->getDataLayout());

	auto s = [](llvm::orc::MangleAndInterner interner)
		{
			llvm::orc::SymbolMap symbolMap;
			symbolMap[interner("readf")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&readf)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("num")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&num)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("random")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&random)),
				llvm::JITSymbolFlags(),
			};
			return llvm::orc::absoluteSymbols(symbolMap);
		};
	auto map = s(mangle);
	dylib.define(map);

	// Add the module to the JIT
	if (auto err = jit->addIRModule(llvm::orc::ThreadSafeModule(std::move(m_Module), std::move(m_Context))))
	{
		llvm::errs() << "Failed to add module to JIT: " << err << "\n";
		return 1;
	}

	// Find the address of the main function
	auto mainSymbol = jit->lookup("main");
	if (!mainSymbol) {
		llvm::errs() << "Could not find main function. Aborting.\n";
		return 1;
	}

	// Get a function pointer to the main function
	using MainFuncType = double (*)();
	auto mainFunc = reinterpret_cast<MainFuncType>(mainSymbol.get().getValue());

	// Call the main function
	return mainFunc();
}

void csaw::Environment::CreateVariable(const std::string& name, llvm::Value* value)
{
	if (m_Variables.contains(name))
		throw;
	if (!value)
		throw;

	if (!Builder().GetInsertBlock())
	{
		m_Variables[name] = { value->getType(), value, true };
		return;
	}

	auto ptr = Builder().CreateAlloca(value->getType());
	Builder().CreateStore(value, ptr);
	m_Variables[name] = { value->getType(), ptr, false };
}

void csaw::Environment::CreateVariable(const std::string& name, llvm::Type* type)
{
	llvm::Value* value = nullptr;
	if (type->isDoubleTy()) // num
		value = llvm::ConstantFP::get(type, 0.0);
	else if (type->isIntegerTy()) // chr
		value = llvm::ConstantInt::get(type, 0);
	else if (type->isPointerTy()) // str, ...
		value = llvm::ConstantPointerNull::get(llvm::dyn_cast<llvm::PointerType>(type));

	CreateVariable(name, value);
}

llvm::Value* csaw::Environment::SetVariable(const std::string& name, llvm::Value* value)
{
	auto& var = GetVar(name);
	if (std::get<2>(var))
		throw;
	if (std::get<0>(var) != value->getType())
		throw;
	Builder().CreateStore(value, std::get<1>(var));
	return value;
}

llvm::Value* csaw::Environment::GetVariable(const std::string& name)
{
	auto& var = GetVar(name);
	if (std::get<2>(var))
		return std::get<1>(var);
	return Builder().CreateLoad(std::get<0>(var), std::get<1>(var));
}

csaw::var_t& csaw::Environment::GetVar(const std::string& name)
{
	if (m_Variables.contains(name))
		return m_Variables[name];
	if (m_Parent)
		return m_Parent->GetVar(name);
	throw;
}

void csaw::Environment::InitEnvironment()
{
	m_Context = std::make_unique<llvm::LLVMContext>();
	m_Module = std::make_unique<llvm::Module>("ChainSaw", *m_Context);

	m_Builder = std::make_unique<llvm::IRBuilder<>>(*m_Context);

	m_FPM = std::make_unique<llvm::FunctionPassManager>();
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

	llvm::PassBuilder passes;
	passes.registerModuleAnalyses(*m_MAM);
	passes.registerFunctionAnalyses(*m_FAM);
	passes.crossRegisterProxies(*m_LAM, *m_FAM, *m_CGAM, *m_MAM);
}

void csaw::Environment::CreateFunction(const std::string& name, llvm::Function* fun)
{
	m_Functions[name].push_back(fun);
}

llvm::Function* csaw::Environment::GetFunction(const std::string& name, const std::vector<llvm::Type*> types)
{
	auto& funs = m_Functions[name];
	if (funs.empty())
		return nullptr;

	for (auto& fun : funs)
	{
		if (fun->arg_size() > types.size() || (fun->arg_size() < types.size() && !fun->isVarArg()))
			continue;

		size_t i = 0;
		for (auto& arg : fun->args())
			if (arg.getType() != types[i++])
				break;

		if (i < fun->arg_size())
			continue;

		return fun;
	}

	return nullptr;
}
