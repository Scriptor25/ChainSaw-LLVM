#pragma once

#include "ast.h"

#include <filesystem>
#include <map>

#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>

namespace csaw
{
	struct type_t
	{
		type_t()
		{
		}

		type_t(const std::string& name, llvm::Type* type)
		{
			this->name = name;
			this->type = type;
		}

		type_t(const std::string& name, llvm::Type* type, llvm::Type* element)
		{
			this->name = name;
			this->type = type;
			this->element = element;
		}

		std::string name;
		llvm::Type* type = nullptr;
		llvm::Type* element = nullptr;
	};

	inline bool operator!=(const type_t& a, const type_t& b)
	{
		return a.name != b.name || a.type != b.type || (!(a.type->isPointerTy() && !a.element) && a.element != b.element);
	}

	inline bool operator<(const type_t& a, const type_t& b)
	{
		if (!a.type) return b.type;
		if (!b.type) return false;
		if (!a.element) return b.element;
		if (!b.element) return false;
		return a.type < b.type;
	}

	struct value_t
	{
		value_t()
		{
		}

		value_t(llvm::Value* value, type_t ptrType)
		{
			this->value = value;
			this->ptrType = ptrType;
		}

		llvm::Value* operator()() const { return value; }
		bool operator!() const { return !value; }
		operator bool() const { return value; }

		llvm::Value* value = nullptr;
		type_t ptrType;
	};

	struct fun_t
	{
		llvm::Function* operator()() { return fun; }
		bool operator!() const { return !fun; }
		operator bool() const { return fun; }

		llvm::Function* fun = nullptr;
		type_t type;
		std::vector<type_t> argtypes;
		bool isconstructor = false;
	};

	struct thing_t
	{
		llvm::StructType* type = nullptr;
		std::vector<std::pair<std::string, type_t>> fields;
	};

	class Environment;

	typedef std::shared_ptr<Environment> EnvironmentPtr;

	class Environment
	{
	public:
		Environment(const EnvironmentPtr& parent)
			: m_Parent(parent) {}

		Environment(const std::filesystem::path& path)
			: Environment(EnvironmentPtr())
		{
			m_Path = path;
		}

		value_t CreateVariable(const std::string& name, const value_t& value, bool isGlobal = false);
		value_t SetVariable(const std::string& name, const value_t& value);
		value_t GetVariable(const std::string& name);

		llvm::Value* SetVarArgs(llvm::Value* valist);
		llvm::Value* GetVarArgs();

		bool IsTopLevel() const { return !m_Parent.get(); }
		std::filesystem::path Path() const { return m_Path; }
		void Path(const std::filesystem::path& path) { m_Path = path; }

	private:
		value_t& GetVar(const std::string& name);

	private:
		std::filesystem::path m_Path;
		EnvironmentPtr m_Parent;
		std::map<std::string, value_t> m_Variables;
		llvm::Value* m_VarArgs = nullptr;

	public:
		static void InitEnvironment();

		static void CreateFunction(const type_t& memberof, const std::string& name, const fun_t& fun);
		static fun_t GetFunction(const type_t& memberof, const std::string& name, const std::vector<type_t>& argtypes);

		static void CreateType(const std::string& name, llvm::StructType* type, const std::vector<std::pair<std::string, type_t>>& fields);
		static const thing_t* GetType(const std::string& name);
		static const thing_t* GetType(llvm::StructType* strtype);

		static void CreateAlias(const std::string& alias, const type_t& origin);
		static bool HasAlias(const std::string& alias);
		static const type_t& GetAlias(const std::string& alias);

		static value_t GetNull(const type_t& type);

		static value_t CreateCall(const type_t& memberof, const std::string& name, const std::vector<value_t>& args, bool justAsking = false);
		static value_t NextVarArg(const type_t& type, llvm::Value* vaptr);
		static void Optimize(llvm::Function* function) { m_FPM->run(*function, *m_FAM); }

		static double Run();
		static void Compile(const std::string& filename);

		static llvm::LLVMContext& Context() { return *m_Context; }
		static llvm::IRBuilder<>& Builder() { return *m_Builder; }
		static llvm::Module& Module() { return *m_Module; }

	private:
		static void CreateGlobalFunction();
		static void FinishGlobalFunction();

	private:
		static std::map<type_t, std::map<std::string, std::vector<fun_t>>> m_Functions;
		static std::map<std::string, thing_t> m_Types;
		static std::map<std::string, type_t> m_Alias;

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
	type_t GenIR(const std::shared_ptr<Type>& type);
	type_t GenIR(const std::string& type);
	type_t GenIR(const std::shared_ptr<ArrayType>& type);

	// GenIR for Statements
	void GenIR(const EnvironmentPtr& env, const StmtPtr& stmt);
	void GenIR(const EnvironmentPtr& env, const AliasStmtPtr& stmt);
	void GenIR(const EnvironmentPtr& env, const EnclosedStmtPtr& stmt);
	void GenIR(const EnvironmentPtr& env, const ForStmtPtr& stmt);
	void GenIR(const EnvironmentPtr& env, const FunStmtPtr& stmt);
	void GenIR(const EnvironmentPtr& env, const IfStmtPtr& stmt);
	void GenIR(const EnvironmentPtr& env, const IncStmtPtr& stmt);
	void GenIR(const EnvironmentPtr& env, const RetStmtPtr& stmt);
	void GenIR(const EnvironmentPtr& env, const ThingStmtPtr& stmt);
	void GenIR(const EnvironmentPtr& env, const VarStmtPtr& stmt);
	void GenIR(const EnvironmentPtr& env, const WhileStmtPtr& stmt);

	// GenIR for Expressions
	value_t GenIR(const EnvironmentPtr& env, const ExprPtr& expr);
	value_t GenIR(const EnvironmentPtr& env, const BinExprPtr& expr);
	value_t GenIR(const EnvironmentPtr& env, const CallExprPtr& expr);
	value_t GenIR(const EnvironmentPtr& env, const ChrExprPtr& expr);
	value_t GenIR(const EnvironmentPtr& env, const ConExprPtr& expr);
	value_t GenIR(const EnvironmentPtr& env, const IdExprPtr& expr);
	value_t GenIR(const EnvironmentPtr& env, const IndexExprPtr& expr);
	value_t GenIR(const EnvironmentPtr& env, const LambdaExprPtr& expr);
	value_t GenIR(const EnvironmentPtr& env, const MemExprPtr& expr);
	value_t GenIR(const EnvironmentPtr& env, const NumExprPtr& expr);
	value_t GenIR(const EnvironmentPtr& env, const StrExprPtr& expr);
	value_t GenIR(const EnvironmentPtr& env, const UnExprPtr& expr);
	value_t GenIR(const EnvironmentPtr& env, const VarArgExprPtr& expr);

	// Predefined Binary Operators
	value_t OpAdd(value_t left, value_t right);
	value_t OpSub(value_t left, value_t right);
	value_t OpMul(value_t left, value_t right);
	value_t OpDiv(value_t left, value_t right);
	value_t OpMod(value_t left, value_t right);
	value_t OpEQ(value_t left, value_t right);
	value_t OpNE(value_t left, value_t right);
	value_t OpLT(value_t left, value_t right);
	value_t OpGT(value_t left, value_t right);
	value_t OpLTE(value_t left, value_t right);
	value_t OpGTE(value_t left, value_t right);
	value_t OpLAnd(value_t left, value_t right);
	value_t OpLOr(value_t left, value_t right);
	value_t OpAnd(value_t left, value_t right);
	value_t OpOr(value_t left, value_t right);
	value_t OpXOr(value_t left, value_t right);
	value_t OpShL(value_t left, value_t right);
	value_t OpShR(value_t left, value_t right);

	// Predefined Unary Operators
	value_t OpNot(value_t value);
	value_t OpNeg(value_t value);
	value_t OpInv(value_t value);

	llvm::Value* BoolToNum(llvm::Value* value);
	llvm::Value* NumToBool(llvm::Value* value);
	llvm::Value* IntToNum(llvm::Value* value);
	llvm::Value* NumToInt(llvm::Value* value);
}
