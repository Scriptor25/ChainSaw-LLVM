#include <csaw/env.h>

csaw::Environment::Environment(
	const std::string& path,
	const std::string& filename,
	const std::vector<std::string>& args,
	const std::vector<std::string>& flags,
	const std::map<std::string, std::string>& options)
	: m_FilePath(filename), m_IsGlobal(true)
{
	m_Context = std::make_unique<llvm::LLVMContext>();
	m_Module = std::make_unique<llvm::Module>(filename, Context());
	m_Builder = std::make_unique<llvm::IRBuilder<>>(Context());
}
