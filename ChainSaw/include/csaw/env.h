#pragma once

#include "gen.h"

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

namespace csaw
{
	struct type_t;
	struct value_t;

	class Environment
	{
	public:
		Environment(
			const std::string& path,
			const std::string& filename,
			const std::vector<std::string>& args,
			const std::vector<std::string>& flags,
			const std::map<std::string, std::string>& options);

		llvm::LLVMContext& Context() const { return *m_Context; }
		llvm::Module& Module() const { return *m_Module; }
		llvm::IRBuilder<>& Builder() const { return *m_Builder; }

		std::filesystem::path& GetFilePath() { return m_FilePath; }
		bool& IsGlobal() { return m_IsGlobal; }
		std::map<std::string, value_t*>& Values() { return m_Values; }
		std::map<std::string, value_t*>& Variables() { return m_Variables; }
		std::map<std::string, type_t*>& Alias() { return m_Alias; }

		const std::filesystem::path& GetFilePath() const { return m_FilePath; }
		const bool& IsGlobal() const { return m_IsGlobal; }
		const std::map<std::string, value_t*>& Values() const { return m_Values; }
		const std::map<std::string, value_t*>& Variables() const { return m_Variables; }
		const std::map<std::string, type_t*>& Alias() const { return m_Alias; }


	private:
		std::unique_ptr<llvm::LLVMContext> m_Context;
		std::unique_ptr<llvm::Module> m_Module;
		std::unique_ptr<llvm::IRBuilder<>> m_Builder;

		std::filesystem::path m_FilePath;
		bool m_IsGlobal;
		std::map<std::string, value_t*> m_Values;
		std::map<std::string, value_t*> m_Variables;
		std::map<std::string, type_t*> m_Alias;
	};

	typedef std::shared_ptr<Environment> EnvPtr;
}
