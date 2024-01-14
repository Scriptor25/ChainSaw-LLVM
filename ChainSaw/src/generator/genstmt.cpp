#include <csaw/gen.h>
#include <csaw/parser.h>

#include <filesystem>
#include <iostream>

#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>

void csaw::GenIR(const EnvPtr& env, const StmtPtr& stmt)
{
	if (auto s = std::dynamic_pointer_cast<AliasStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<EnclosedStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<ForStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<FunctionStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<IfStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<IncStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<RetStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<ThingStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<VariableStmt>(stmt)) return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<WhileStmt>(stmt)) return GenIR(env, s);

	if (auto e = std::dynamic_pointer_cast<Expr>(stmt))
	{
		GenIR(env, e);
		return;
	}

	throw;
}

void csaw::GenIR(const EnvPtr& env, const AliasStmtPtr& stmt)
{
	env->Alias()[stmt->Alias] = GenIR(env, stmt->Origin);
}

void csaw::GenIR(const EnvPtr& env, const EnclosedStmtPtr& stmt)
{
	for (auto& s : stmt->Body)
		GenIR(env, s);
}

void csaw::GenIR(const EnvPtr& env, const ForStmtPtr& stmt)
{
	throw;
}

void csaw::GenIR(const EnvPtr& env, const FunctionStmtPtr& stmt)
{
	auto function = env->Module().getFunction(stmt->Name);
	if (!function)
	{
		auto result = GenIR(env, stmt->RetType);

		std::vector<llvm::Type*> params;
		for (auto& param : stmt->Parameters)
			params.push_back(*GenIR(env, param.Type));

		auto functiontype = llvm::FunctionType::get(*result, params, stmt->IsVarArg);
		function = llvm::Function::Create(functiontype, llvm::GlobalValue::ExternalLinkage, stmt->Name, env->Module());

		size_t i = 0;
		for (auto& arg : function->args())
			arg.setName(stmt->Parameters[i++].Name);
	}

	if (!function)
	{
		std::cerr << "Failed to generate ir for function '" << stmt->Name << "': function creation failed" << std::endl;
		return;
	}

	if (!stmt->Body)
		return;

	if (!function->empty())
	{
		std::cerr << "Failed to generate ir for function '" << stmt->Name << "': function already defined" << std::endl;
		return;
	}

	auto entryblock = llvm::BasicBlock::Create(env->Context(), "entry", function);
	env->Builder().SetInsertPoint(entryblock);
	env->IsGlobal() = false;

	env->Values().clear();
	size_t i = 0;
	for (auto& arg : function->args())
		env->Values()[std::string(arg.getName())] = new value_t(&arg, GenIR(env, stmt->Parameters[i++].Type));

	GenIR(env, stmt->Body);

	env->IsGlobal() = true;

	if (llvm::verifyFunction(*function, &llvm::errs()))
	{
		function->eraseFromParent();
		std::cerr << "Failed to generate ir for function '" << stmt->Name << "': function verification failed" << std::endl;
		return;
	}
}

void csaw::GenIR(const EnvPtr& env, const IfStmtPtr& stmt)
{
	throw;
}

void csaw::GenIR(const EnvPtr& env, const IncStmtPtr& stmt)
{
	std::filesystem::path original = env->GetFilePath();
	auto filepath = original.parent_path() / stmt->Path;

	env->GetFilePath() = filepath;
	if (!Parse(env, filepath))
	{
		env->GetFilePath() = original;
		std::cerr << "Failed to parse file '" << filepath << "'" << std::endl;
		return;
	}
	env->GetFilePath() = original;
}

void csaw::GenIR(const EnvPtr& env, const RetStmtPtr& stmt)
{
	if (!stmt->Value) env->Builder().CreateRetVoid();
	else env->Builder().CreateRet(*GenIR(env, stmt->Value));
}

void csaw::GenIR(const EnvPtr& env, const ThingStmtPtr& stmt)
{
	throw;
}

void csaw::GenIR(const EnvPtr& env, const VariableStmtPtr& stmt)
{
	auto type = GenIR(env, stmt->Type);

	value_t* value = nullptr;
	if (stmt->Value) value = GenIR(env, stmt->Value);
	else value = new value_t(llvm::Constant::getNullValue(*type), type);

	llvm::Value* ptr = nullptr;

	if (env->IsGlobal())
	{
		llvm::Constant* constant = nullptr;
		if (*value) constant = llvm::dyn_cast<llvm::Constant>(value->Value);
		ptr = new llvm::GlobalVariable(*type, false, llvm::GlobalValue::InternalLinkage, constant, stmt->Name);
	}
	else
	{
		ptr = env->Builder().CreateAlloca(*type, nullptr, stmt->Name);
		if (*value) env->Builder().CreateStore(*value, ptr);
	}

	env->Variables()[stmt->Name] = new value_t(ptr, new type_t(ptr->getType(), type));
}

void csaw::GenIR(const EnvPtr& env, const WhileStmtPtr& stmt)
{
	auto parent = env->Builder().GetInsertBlock()->getParent();
	auto condblock = llvm::BasicBlock::Create(env->Context(), "condition", parent);
	auto loopblock = llvm::BasicBlock::Create(env->Context(), "loop", parent);
	auto exitblock = llvm::BasicBlock::Create(env->Context(), "exit", parent);

	env->Builder().CreateBr(condblock);

	env->Builder().SetInsertPoint(condblock);
	auto condition = GenIRBool(env, *GenIR(env, stmt->Condition));
	env->Builder().CreateCondBr(condition, loopblock, exitblock);

	env->Builder().SetInsertPoint(loopblock);
	GenIR(env, stmt->Body);
	env->Builder().CreateBr(condblock);

	env->Builder().SetInsertPoint(exitblock);
}
