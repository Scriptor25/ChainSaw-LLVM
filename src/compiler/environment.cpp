#include "../lang/csawstd.h"
#include "compiler.h"

#include <format>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/DCE.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>

std::map<csaw::type_t, std::map<std::string, std::vector<csaw::fun_t>>> csaw::Environment::m_Functions;
std::map<std::string, std::map<std::string, csaw::thing_t>> csaw::Environment::m_Types;

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

csaw::value_t csaw::Environment::CreateVariable(const std::string& name, const value_t& value, bool isGlobal)
{
	if (m_Variables.contains(name))
		throw "cannot redefine variable";

	if (isGlobal)
		return m_Variables[name] = value;

	auto ptr = Builder().CreateAlloca(value.ptrType.type);
	Builder().CreateStore(value(), ptr);

	m_Variables[name] = value_t(ptr, value.ptrType);

	return value;
}

csaw::value_t csaw::Environment::SetVariable(const std::string& name, const value_t& value)
{
	auto& var = GetVar(name);
	auto ptr = var();

	Builder().CreateStore(value(), ptr);

	return value;
}

csaw::value_t csaw::Environment::GetVariable(const std::string& name)
{
	auto& var = GetVar(name);
	auto ptr = var();

	return value_t(Builder().CreateLoad(var.ptrType.type, ptr, name), var.ptrType);
}

csaw::value_t& csaw::Environment::GetVar(const std::string& name)
{
	if (m_Variables.contains(name))
		return m_Variables[name];
	if (m_Parent)
		return m_Parent->GetVar(name);
	throw "undefined variable";
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
	m_FPM->addPass(llvm::DCEPass());
	m_FPM->addPass(llvm::LoopSimplifyPass());

	llvm::PassBuilder passes;
	passes.registerModuleAnalyses(*m_MAM);
	passes.registerFunctionAnalyses(*m_FAM);
	passes.crossRegisterProxies(*m_LAM, *m_FAM, *m_CGAM, *m_MAM);

	CreateGlobalFunction();
}

void csaw::Environment::CreateFunction(const type_t& memberof, const std::string& name, const fun_t& fun)
{
	m_Functions[memberof][name].push_back(fun);
}

csaw::fun_t csaw::Environment::GetFunction(const type_t& memberof, const std::string& name, const std::vector<type_t>& argtypes)
{
	auto& funs = m_Functions[memberof][name];
	if (funs.empty())
		return {};

	for (auto& fun : funs)
	{
		if (fun.argtypes.size() > argtypes.size() || (fun.argtypes.size() < argtypes.size() && !fun()->isVarArg()))
			continue;

		size_t i = 0;
		for (; i < fun.argtypes.size(); i++)
			if (fun.argtypes[i] != argtypes[i])
				break;

		if (i == fun.argtypes.size())
			return fun;
	}

	return {};
}

void csaw::Environment::CreateType(const std::string& name, const std::string& group, llvm::StructType* type, const std::vector<std::string>& fields)
{
	m_Types[group][name] = { type, fields };
}

const csaw::thing_t* csaw::Environment::GetType(const std::string& name, const std::string& group)
{
	if (!m_Types.contains(group) || !m_Types[group].contains(name))
		return nullptr;
	return &m_Types[group][name];
}

const csaw::thing_t* csaw::Environment::GetType(llvm::StructType* strtype)
{
	for (auto& group : m_Types)
		for (auto& type : group.second)
			if (type.second.type == strtype)
				return &type.second;
	return nullptr;
}

csaw::value_t csaw::Environment::GetNull(const type_t& type)
{
	if (type.type->isPointerTy())
	{
		auto ptr = Builder().CreateAlloca(type.element);
		Builder().CreateStore(llvm::Constant::getNullValue(type.element), ptr);
		return value_t(ptr, type);
	}

	return llvm::Constant::getNullValue(type.type);
}

csaw::value_t csaw::Environment::CreateCall(const type_t& memberof, const std::string& name, const std::vector<value_t>& args, bool justAsking)
{
	std::vector<type_t> types;
	std::vector<llvm::Value*> values;
	for (auto& arg : args)
	{
		types.push_back(arg.ptrType);
		values.push_back(arg.value);
	}

	auto fun = GetFunction(memberof, name, types);
	if (!fun)
	{
		if (justAsking)
			return {};
		throw "undefined function";
	}

	if (fun.isconstructor)
	{
		auto my = Environment::GetNull(fun.type);
		values.insert(values.begin(), my());
	}

	return value_t(Environment::Builder().CreateCall(fun(), values), fun.type);
}

double csaw::Environment::Run()
{
	FinishGlobalFunction();

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
			symbolMap[interner("round")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::round)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("floor")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::floor)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("ceil")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::ceil)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("sin")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::sin)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("cos")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::cos)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("tan")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::tan)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("asin")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::asin)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("acos")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::acos)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("atan")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::atan)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("atan2")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::atan2)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("random")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::random)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("printf")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::printf)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("readf")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::readf)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("numFromStr")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::numFromStr)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("numFromChr")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::numFromChr)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("strFromNum")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::strFromNum)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("chrFromNum")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::chrFromNum)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("streq")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::streq)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("strlng")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::strlng)),
				llvm::JITSymbolFlags(),
			};
			symbolMap[interner("strat")] = {
				llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(&csaw::strat)),
				llvm::JITSymbolFlags(),
			};
			return llvm::orc::absoluteSymbols(symbolMap);
		};
	auto map = s(mangle);

	llvm::ExitOnError()(dylib.define(map));

	// Add the module to the JIT
	if (auto err = jit->addIRModule(llvm::orc::ThreadSafeModule(std::move(m_Module), std::move(m_Context))))
	{
		llvm::errs() << "Failed to add module to JIT: " << err << "\n";
		return 1;
	}

	auto globalSymbol = jit->lookup("__global__");
	if (!globalSymbol) {
		llvm::errs() << "Could not find __global__ function. Aborting.\n";
		return 1;
	}

	using GlobalFuncType = void (*)();
	auto globalFunc = reinterpret_cast<GlobalFuncType>(globalSymbol.get().getValue());
	globalFunc();

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

bool csaw::Environment::Compile(const std::string& filename)
{
	FinishGlobalFunction();

	if (llvm::verifyModule(Module(), &llvm::errs())) {
		llvm::errs() << "Error in the module. Aborting.\n";
		return false;
	}

	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();

	auto triple = llvm::sys::getDefaultTargetTriple();

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

	pm.run(Module());
	dst.flush();

	return true;
}

void csaw::Environment::CreateGlobalFunction()
{ // create function for global initializers
	auto funtype = llvm::FunctionType::get(Builder().getVoidTy(), false);
	auto fun = llvm::Function::Create(funtype, llvm::Function::ExternalLinkage, "__global__", Module());

	auto entry = llvm::BasicBlock::Create(Context(), "fun_entry", fun);
	Builder().SetInsertPoint(entry);
}

void csaw::Environment::FinishGlobalFunction()
{ // verify and optimize global initializer function
	auto fun = Module().getFunction("__global__");

	Builder().SetInsertPoint(&fun->back());
	Builder().CreateRetVoid();

	if (llvm::verifyFunction(*fun, &llvm::errs()))
	{
		fun->print(llvm::errs());
		throw "failed to verify global initializers function";
	}

	Optimize(fun);
}
