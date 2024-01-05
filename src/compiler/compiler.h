#pragma once

#include "../ast/ast.h"

#include <filesystem>
#include <map>

#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>

namespace csaw
{
	typedef std::tuple<llvm::Type*, llvm::Value*, bool> var_t;

	class Environment
	{
	public:
		Environment(const std::shared_ptr<Environment>& parent)
			: m_Parent(parent) {}

		Environment(const std::filesystem::path& path)
			: Environment(std::shared_ptr<Environment>())
		{
			m_Path = path;
		}

		void CreateVariable(const std::string& name, llvm::Value* value);
		void CreateVariable(const std::string& name, llvm::Type* type);
		llvm::Value* SetVariable(const std::string& name, llvm::Value* value);
		llvm::Value* GetVariable(const std::string& name);

		std::filesystem::path Path() const { return m_Path; }
		void Path(const std::filesystem::path& path) { m_Path = path; }

	private:
		var_t& GetVar(const std::string& name);

	private:
		std::filesystem::path m_Path;
		std::shared_ptr<Environment> m_Parent;
		std::map<std::string, var_t> m_Variables;

	public:
		static void InitEnvironment();

		static void CreateFunction(const std::string& name, llvm::Function* fun);
		static llvm::Function* GetFunction(const std::string& name, const std::vector<llvm::Type*> types);

		static llvm::Value* CreateCall(const std::string& name, std::vector<llvm::Value*> args);
		static void Optimize(llvm::Function* function) { m_FPM->run(*function, *m_FAM); }

		static double Run();
		static bool Compile(const std::string& filename);

		static llvm::LLVMContext& Context() { return *m_Context; }
		static llvm::IRBuilder<>& Builder() { return *m_Builder; }
		static llvm::Module& Module() { return *m_Module; }

	private:
		static std::map<std::string, std::vector<llvm::Function*>> m_Functions;

		static std::unique_ptr<llvm::LLVMContext> m_Context;
		static std::unique_ptr<llvm::IRBuilder<>> m_Builder;
		static std::unique_ptr<llvm::Module> m_Module;

		static std::unique_ptr<llvm::FunctionPassManager> m_FPM;
		static std::unique_ptr<llvm::LoopAnalysisManager> m_LAM;
		static std::unique_ptr<llvm::FunctionAnalysisManager> m_FAM;
		static std::unique_ptr<llvm::CGSCCAnalysisManager> m_CGAM;
		static std::unique_ptr<llvm::ModuleAnalysisManager> m_MAM;
		static std::unique_ptr<llvm::PassInstrumentationCallbacks> m_PIC;
		static std::unique_ptr<llvm::StandardInstrumentations> m_SI;
	};

	// GenIR for Types
	llvm::Type* GenIR(const std::shared_ptr<ASTType>& type);
	llvm::Type* GenIR(const std::shared_ptr<ASTArrayType>& type);

	// GenIR for Statements
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<Stmt>& stmt);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<AliasStmt>& stmt);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<EnclosedStmt>& stmt);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<ForStmt>& stmt);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<FunStmt>& stmt);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<IfStmt>& stmt);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<IncStmt>& stmt);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<RetStmt>& stmt);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<ThingStmt>& stmt);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<VarStmt>& stmt);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<WhileStmt>& stmt);

	// GenIR for Expressions
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<Expr>& expr);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<AssignExpr>& expr);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<BinExpr>& expr);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<CallExpr>& expr);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<ChrExpr>& expr);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<ConExpr>& expr);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<IdExpr>& expr);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<IndexExpr>& expr);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<LambdaExpr>& expr);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<MemExpr>& expr);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<NumExpr>& expr);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<StrExpr>& expr);
	llvm::Value* GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<UnExpr>& expr);

	// Predefined Binary Operators
	llvm::Value* OpAdd(llvm::Value* left, llvm::Value* right);
	llvm::Value* OpSub(llvm::Value* left, llvm::Value* right);
	llvm::Value* OpMul(llvm::Value* left, llvm::Value* right);
	llvm::Value* OpDiv(llvm::Value* left, llvm::Value* right);
	llvm::Value* OpEqu(llvm::Value* left, llvm::Value* right);
	llvm::Value* OpNeq(llvm::Value* left, llvm::Value* right);
	llvm::Value* OpAnd(llvm::Value* left, llvm::Value* right);
	llvm::Value* OpOr(llvm::Value* left, llvm::Value* right);

	// Predefined Unary Operators
	llvm::Value* OpNot(llvm::Value* value);
	llvm::Value* OpNeg(llvm::Value* value);
	llvm::Value* OpInv(llvm::Value* value);

}
