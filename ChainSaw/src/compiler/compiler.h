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
	struct type_t
	{
		type_t()
		{
			this->type = this->element = nullptr;
		}

		type_t(llvm::Type* type)
		{
			this->type = type;
			this->element = nullptr;
		}

		type_t(llvm::Type* type, llvm::Type* element)
		{
			this->type = type;
			this->element = element;
		}

		llvm::Type* type;
		llvm::Type* element;
	};

	inline bool operator!=(const type_t& a, const type_t& b)
	{
		return a.type != b.type || (!(a.type->isPointerTy() && !a.element) && a.element != b.element);
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
			value = nullptr;
		}

		value_t(llvm::Value* value)
		{
			this->value = value;
			this->ptrType = { value->getType(), nullptr };
		}

		value_t(llvm::Value* value, type_t ptrType)
		{
			this->value = value;
			this->ptrType = ptrType;
		}

		llvm::Value* operator()() const { return value; }
		bool operator!() const { return !value; }
		operator bool() const { return value; }

		llvm::Value* value;
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
		bool isconstructor;
	};

	struct thing_t
	{
		llvm::StructType* type;
		std::vector<std::string> fields;
	};

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
		std::shared_ptr<Environment> m_Parent;
		std::map<std::string, value_t> m_Variables;
		llvm::Value* m_VarArgs = nullptr;

	public:
		static void InitEnvironment();

		static void CreateFunction(const type_t& memberof, const std::string& name, const fun_t& fun);
		static fun_t GetFunction(const type_t& memberof, const std::string& name, const std::vector<type_t>& argtypes);
		static void CreateType(const std::string& name, const std::string& group, llvm::StructType* type, const std::vector<std::string>& fields);
		static const thing_t* GetType(const std::string& name, const std::string& group);
		static const thing_t* GetType(llvm::StructType* strtype);

		static value_t GetNull(const type_t& type);

		static value_t CreateCall(const type_t& memberof, const std::string& name, const std::vector<value_t>& args, bool justAsking = false);
		static void Optimize(llvm::Function* function) { m_FPM->run(*function, *m_FAM); }

		static double Run();
		static void Compile(const std::string& filename);

		static llvm::LLVMContext& Context() { return *m_Context; }
		static llvm::IRBuilder<>& Builder() { return *m_Builder; }
		static llvm::Module& Module() { return *m_Module; }

	private:
		static llvm::TargetMachine* SetTargetTriple();
		static void CreateGlobalFunction();
		static void FinishGlobalFunction();

	private:
		static std::map<type_t, std::map<std::string, std::vector<fun_t>>> m_Functions;
		static std::map<std::string, std::map<std::string, thing_t>> m_Types;

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
	type_t GenIR(const std::shared_ptr<ASTType>& type);
	type_t GenIR(const std::shared_ptr<ASTArrayType>& type);

	// GenIR for Statements
	void GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<Stmt>& stmt);
	void GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<AliasStmt>& stmt);
	void GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<EnclosedStmt>& stmt);
	void GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<ForStmt>& stmt);
	void GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<FunStmt>& stmt);
	void GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<IfStmt>& stmt);
	void GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<IncStmt>& stmt);
	void GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<RetStmt>& stmt);
	void GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<ThingStmt>& stmt);
	void GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<VarStmt>& stmt);
	void GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<WhileStmt>& stmt);

	// GenIR for Expressions
	value_t GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<Expr>& expr);
	value_t GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<BinExpr>& expr);
	value_t GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<CallExpr>& expr);
	value_t GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<ChrExpr>& expr);
	value_t GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<ConExpr>& expr);
	value_t GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<IdExpr>& expr);
	value_t GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<IndexExpr>& expr);
	value_t GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<LambdaExpr>& expr);
	value_t GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<MemExpr>& expr);
	value_t GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<NumExpr>& expr);
	value_t GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<StrExpr>& expr);
	value_t GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<UnExpr>& expr);
	value_t GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<VarArgExpr>& expr);

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
