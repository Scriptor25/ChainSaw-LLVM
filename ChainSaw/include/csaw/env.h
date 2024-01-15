#pragma once

#include "gen.h"

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Passes/StandardInstrumentations.h>

namespace csaw
{
	struct type_t;
	struct value_t;
	struct function_t;

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

		bool& IsGlobal() { return m_IsGlobal; }
		const bool& IsGlobal() const { return m_IsGlobal; }

		std::filesystem::path& GetFilePath() { return m_FilePath; }
		const std::filesystem::path& GetFilePath() const { return m_FilePath; }

		std::map<std::string, value_t*>& Variables() { return m_Variables; }
		const std::map<std::string, value_t*>& Variables() const { return m_Variables; }

		std::map<std::string, value_t*>& LocalVariables() { return m_LocalVariables; }
		const std::map<std::string, value_t*>& LocalVariables() const { return m_LocalVariables; }

		std::map<std::string, type_t*>& Alias() { return m_Alias; }
		const std::map<std::string, type_t*>& Alias() const { return m_Alias; }

		function_t* GetFunction(const std::string& name, type_t* member_of, type_t* result, const std::vector<type_t*>& args, bool is_var_args, bool is_constructor);
		function_t* GetFunction(const std::string& name, type_t* member_of, const std::vector<type_t*>& args);
		function_t* CreateFunction(const std::string& name, type_t* member_of, type_t* result, const std::vector<type_t*>& args, bool is_var_args, bool is_constructor);

		type_t* GetType(const std::string& name);
		type_t* GetVoidType();
		type_t* GetNumType();
		type_t* GetChrType();
		type_t* GetStrType();
		type_t* CreateType(const std::string& name, type_t* type);
		type_t* CreateType(const std::string& name, llvm::Type* type);
		type_t* CreateType(const std::string& name, llvm::Type* type, type_t* element);
		type_t* CreateType(const std::string& name, llvm::Type* type, llvm::Type* element);

		value_t* GetNum(double value);
		value_t* GetChr(char value);
		value_t* GetStr(const std::string& value);
		value_t* GetEmptyValue(type_t* type);

		void Optimize(llvm::Function* function) { m_FPM->run(*function, *m_FAM); }
		void Optimize(llvm::Module* module) { m_MPM->run(*module, *m_MAM); }

		double RunJIT();
		void Compile(const std::string& filename);

	private:
		std::unique_ptr<llvm::LLVMContext> m_Context;
		std::unique_ptr<llvm::Module> m_Module;
		std::unique_ptr<llvm::IRBuilder<>> m_Builder;

		std::unique_ptr<llvm::FunctionPassManager> m_FPM;
		std::unique_ptr<llvm::ModulePassManager> m_MPM;
		std::unique_ptr<llvm::LoopAnalysisManager> m_LAM;
		std::unique_ptr<llvm::FunctionAnalysisManager> m_FAM;
		std::unique_ptr<llvm::CGSCCAnalysisManager> m_CGAM;
		std::unique_ptr<llvm::ModuleAnalysisManager> m_MAM;
		std::unique_ptr<llvm::PassInstrumentationCallbacks> m_PIC;
		std::unique_ptr<llvm::StandardInstrumentations> m_SI;

		bool m_IsGlobal;
		std::filesystem::path m_FilePath;

		std::map<std::string, value_t*> m_Variables;
		std::map<std::string, value_t*> m_LocalVariables;
		std::map<std::string, type_t*> m_Alias;
		std::map<type_t*, std::map<std::string, std::vector<function_t*>>> m_Functions;
		std::map<std::string, type_t*> m_Types;
	};

	typedef std::shared_ptr<Environment> EnvPtr;
}
