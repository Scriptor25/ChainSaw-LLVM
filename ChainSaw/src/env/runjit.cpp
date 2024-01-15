#include <csaw/env.h>
#include <csawstd/csawstd.h>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>

double csaw::Environment::RunJIT()
{
	if (llvm::verifyModule(Module(), &llvm::errs())) {
		llvm::errs() << "Failed to verify module\r\n";
		return 1;
	}

	Optimize(&Module());

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
		{ mangle("readf"),  { llvm::orc::ExecutorAddr::fromPtr(&readf), llvm::JITSymbolFlags() } },
		{ mangle("random"),  { llvm::orc::ExecutorAddr::fromPtr(&random), llvm::JITSymbolFlags() } },
		{ mangle("str_to_num"),  { llvm::orc::ExecutorAddr::fromPtr(&str_to_num), llvm::JITSymbolFlags() } },
		{ mangle("chr_to_num"), { llvm::orc::ExecutorAddr::fromPtr(&chr_to_num), llvm::JITSymbolFlags() } },
		{ mangle("num_to_str"), { llvm::orc::ExecutorAddr::fromPtr(&num_to_str), llvm::JITSymbolFlags() } },
		{ mangle("num_to_chr"), { llvm::orc::ExecutorAddr::fromPtr(&num_to_chr), llvm::JITSymbolFlags() } },
		{ mangle("csaw_strcmp"), { llvm::orc::ExecutorAddr::fromPtr(&csaw_strcmp), llvm::JITSymbolFlags() } },
		{ mangle("csaw_strlen"), { llvm::orc::ExecutorAddr::fromPtr(&csaw_strlen), llvm::JITSymbolFlags() } },
		{ mangle("csaw_strget"), { llvm::orc::ExecutorAddr::fromPtr(&csaw_strget), llvm::JITSymbolFlags() } },
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
