#include <csaw/env.h>

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

void csaw::Environment::Compile(const std::string& filename)
{
	if (llvm::verifyModule(Module(), &llvm::errs())) {
		llvm::errs() << "Failed to verify module\r\n";
		return;
	}

	Optimize(&Module());

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
