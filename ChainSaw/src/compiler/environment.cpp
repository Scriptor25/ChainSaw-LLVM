#include <csaw/compiler.h>
#include <csawstd/csawstd.h>

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
#include <llvm/Transforms/Scalar/LoopUnrollPass.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>

std::map<csaw::type_t, std::map<std::string, std::vector<csaw::fun_t>>> csaw::Environment::m_Functions;
std::map<std::string, csaw::thing_t> csaw::Environment::m_Types;
std::map<std::string, csaw::type_t> csaw::Environment::m_Alias;

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
	{
		llvm::errs() << "Cannot redefine variable '" << name << "'\r\n";
		throw;
	}

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

llvm::Value* csaw::Environment::SetVarArgs(llvm::Value* valist)
{
	return m_VarArgs = valist;
}

llvm::Value* csaw::Environment::GetVarArgs()
{
	if (m_VarArgs)
		return m_VarArgs;
	if (m_Parent)
		return m_Parent->GetVarArgs();
	throw;
}

csaw::value_t& csaw::Environment::GetVar(const std::string& name)
{
	if (m_Variables.contains(name))
		return m_Variables[name];
	if (m_Parent)
		return m_Parent->GetVar(name);
	llvm::errs() << "Undefined variable '" << name << "'\r\n";
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
	m_FPM->addPass(llvm::DCEPass());
	m_FPM->addPass(llvm::LoopSimplifyPass());
	m_FPM->addPass(llvm::LoopUnrollPass());

	llvm::PassBuilder passes;
	passes.registerModuleAnalyses(*m_MAM);
	passes.registerFunctionAnalyses(*m_FAM);
	passes.crossRegisterProxies(*m_LAM, *m_FAM, *m_CGAM, *m_MAM);

	auto triple = llvm::sys::getDefaultTargetTriple();
	m_Module->setTargetTriple(triple);

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
		return fun_t();

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

	return fun_t();
}

void csaw::Environment::CreateType(const std::string& name, llvm::StructType* type, const std::vector<std::pair<std::string, type_t>>& fields)
{
	m_Types[name] = { type, fields };
}

const csaw::thing_t* csaw::Environment::GetType(const std::string& name)
{
	if (!m_Types.contains(name))
		return nullptr;
	return &m_Types[name];
}

const csaw::thing_t* csaw::Environment::GetType(llvm::StructType* strtype)
{
	for (auto& type : m_Types)
		if (type.second.type == strtype)
			return &type.second;
	return nullptr;
}

void csaw::Environment::CreateAlias(const std::string& alias, const type_t& origin)
{
	m_Alias[alias] = origin;
}

bool csaw::Environment::HasAlias(const std::string& alias)
{
	return m_Alias.contains(alias);
}

const csaw::type_t& csaw::Environment::GetAlias(const std::string& alias)
{
	if (HasAlias(alias))
		return GetAlias(m_Alias[alias].name);
	return GenIR(alias);
}

csaw::value_t csaw::Environment::GetNull(const type_t& type)
{
	if (type.type->isPointerTy())
	{
		auto ptr = Builder().CreateAlloca(type.element);
		Builder().CreateStore(llvm::Constant::getNullValue(type.element), ptr);
		return value_t(ptr, type);
	}

	return value_t(llvm::Constant::getNullValue(type.type), type);
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
			return value_t();

		llvm::errs() << "Undefined function '" << name << "'\r\n";
		throw;
	}

	if (fun.isconstructor)
	{
		auto my = Environment::GetNull(fun.type);
		values.insert(values.begin(), my());
	}

	return value_t(Environment::Builder().CreateCall(fun(), values), fun.type);
}

csaw::value_t csaw::Environment::NextVarArg(const type_t& type, llvm::Value* vaptr)
{
	////////////////////////////////////
	// custom varargs implementation: //
	//        (stolen from C)         //
	//                                //
	// va_list = alloca ptr           //
	// va_start(va_list)              //
	//                                //
	// addr0 = load ptr from va_list  //
	// elem0 = gep i8 from addr0 + 8  //
	// store elem0 in va_list         //
	// ?0 = load i32 from addr0       //
	//                                //
	// addr1 = load ptr from va_list  //
	// elem1 = gep i8 from addr1 + 8  //
	// store elem1 in va_list		  //
	// ?1 = load i32 from addr1		  //
	// 								  //
	// addr2 = load ptr from va_list  //
	// elem2 = gep i8 from addr2 + 8  //
	// store elem2 in va_list		  //
	// ?2 = load i32 from addr2       //
	//                                //
	// ...                            //
	//                                //
	// va_end(va_list)                //
	////////////////////////////////////

#define CSAW_CUSTOM_VAARG
#ifdef CSAW_CUSTOM_VAARG
	auto addr = Builder().CreateGEP(Builder().getInt8Ty(), vaptr, { Builder().getInt64(8) }, "addr");
	auto current = Builder().CreateLoad(Builder().getPtrTy(), addr, "current");
	auto value = Builder().CreateLoad(type.type, current, "value");
	auto currentaddr = Builder().CreateGEP(Builder().getInt64Ty(), current, { Builder().getInt64(1) }, "currentaddr");
	Builder().CreateStore(currentaddr, current);
#else
	auto value = Builder().CreateVAArg(vaptr, type.type);
#endif

	return value_t(value, type);
}

double csaw::Environment::Run()
{
	FinishGlobalFunction();

	if (llvm::verifyModule(Module(), &llvm::errs())) {
		llvm::errs() << "Failed to verify module\r\n";
		return 1;
	}

	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetAsmParser();

	// Create an LLJIT instance
	auto builder = llvm::orc::LLJITBuilder().create();
	if (!builder)
	{
		llvm::errs() << "Failed to create LLJIT instance : " << builder.takeError() << "\r\n";
		return 1;
	}

	std::unique_ptr<llvm::orc::LLJIT> jit = std::move(*builder);

	auto& es = jit->getExecutionSession();
	auto& dl = jit->getDataLayout();
	llvm::orc::MangleAndInterner mangle(es, dl);

	auto& jd = jit->getMainJITDylib();

	llvm::orc::SymbolMap symbols
	{
		{ mangle("csaw_vprintf"),  { llvm::orc::ExecutorAddr::fromPtr(&csaw_vprintf), llvm::JITSymbolFlags() } },
		{ mangle("csaw_vreadf"),  { llvm::orc::ExecutorAddr::fromPtr(&csaw_vreadf), llvm::JITSymbolFlags() } },
		{ mangle("csaw_printf"),  { llvm::orc::ExecutorAddr::fromPtr(&csaw_printf), llvm::JITSymbolFlags() } },
		{ mangle("csaw_readf"),  { llvm::orc::ExecutorAddr::fromPtr(&csaw_readf), llvm::JITSymbolFlags() } },
		{ mangle("csaw_random"),  { llvm::orc::ExecutorAddr::fromPtr(&csaw_random), llvm::JITSymbolFlags() } },
		{ mangle("csaw_str_to_num"),  { llvm::orc::ExecutorAddr::fromPtr(&csaw_str_to_num), llvm::JITSymbolFlags() } },
		{ mangle("csaw_chr_to_num"), { llvm::orc::ExecutorAddr::fromPtr(&csaw_chr_to_num), llvm::JITSymbolFlags() } },
		{ mangle("csaw_num_to_str"), { llvm::orc::ExecutorAddr::fromPtr(&csaw_num_to_str), llvm::JITSymbolFlags() } },
		{ mangle("csaw_num_to_chr"), { llvm::orc::ExecutorAddr::fromPtr(&csaw_num_to_chr), llvm::JITSymbolFlags() } },
		{ mangle("csaw_str_cmp"), { llvm::orc::ExecutorAddr::fromPtr(&csaw_str_cmp), llvm::JITSymbolFlags() } },
		{ mangle("csaw_str_len"), { llvm::orc::ExecutorAddr::fromPtr(&csaw_str_len), llvm::JITSymbolFlags() } },
		{ mangle("csaw_str_get"), { llvm::orc::ExecutorAddr::fromPtr(&csaw_str_get), llvm::JITSymbolFlags() } },
	};

	if (auto error = jd.define(llvm::orc::absoluteSymbols(symbols)))
	{
		llvm::errs() << "Failed to define symbol map in Main JIT Dylib: " << error << "\r\n";
		return 1;
	}

	// Add the module to the JIT
	if (auto error = jit->addIRModule(llvm::orc::ThreadSafeModule(std::move(m_Module), std::move(m_Context))))
	{
		llvm::errs() << "Failed to add module to JIT: " << error << "\r\n";
		return 1;
	}

	auto globalSymbol = jit->lookup("__global__");
	if (!globalSymbol) {
		llvm::errs() << "Could not find function '__global__': " << globalSymbol.takeError() << "\r\n";
		return 1;
	}

	using GlobalFuncType = void (*)();
	auto globalFunc = reinterpret_cast<GlobalFuncType>(globalSymbol->getValue());
	globalFunc();

	// Find the address of the main function
	auto mainSymbol = jit->lookup("main");
	if (!mainSymbol) {
		llvm::errs() << "Could not find function 'main':" << mainSymbol.takeError() << "\r\n";
		return 1;
	}

	// Get a function pointer to the main function
	using MainFuncType = double (*)();
	auto mainFunc = reinterpret_cast<MainFuncType>(mainSymbol->getValue());

	// Call the main function
	return mainFunc();
}

void csaw::Environment::Compile(const std::string& filename)
{
	FinishGlobalFunction();

	if (llvm::verifyModule(Module(), &llvm::errs())) {
		llvm::errs() << "Failed to verify module\r\n";
		return;
	}

	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();

	auto& triple = Module().getTargetTriple();

	//llvm::outs() << triple << "\r\n";
	//llvm::TargetRegistry::printRegisteredTargetsForVersion(llvm::outs());

	std::string error;
	auto target = llvm::TargetRegistry::lookupTarget(triple, error);

	if (!target)
	{
		llvm::errs() << "Failed to get target for triple '" << triple << "': " << error << "\r\n";
		return;
	}

	auto cpu = "generic";
	auto features = "";

	llvm::TargetOptions opt;
	auto machine = target->createTargetMachine(triple, cpu, features, opt, llvm::Reloc::PIC_);
	Module().setDataLayout(machine->createDataLayout());

	std::error_code ec;
	llvm::raw_fd_ostream dst(filename, ec, llvm::sys::fs::OF_None);

	if (ec)
	{
		llvm::errs() << "Failed to open file '" << filename << "': " << ec.message() << "\r\n";
		return;
	}

	llvm::legacy::PassManager pm;
	auto type = llvm::CodeGenFileType::ObjectFile;

	if (machine->addPassesToEmitFile(pm, dst, nullptr, type))
	{
		llvm::errs() << "Failed to emit file of type '" << (type == llvm::CodeGenFileType::AssemblyFile ? "AssemblyFile" : type == llvm::CodeGenFileType::ObjectFile ? "ObjectFile" : "Null") << "'\r\n";
		return;
	}

	pm.run(Module());
	dst.flush();
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
		llvm::errs() << "Failed to verify function '__global__':\r\n";
		fun->print(llvm::errs());
		return;
	}

	Optimize(fun);
}
