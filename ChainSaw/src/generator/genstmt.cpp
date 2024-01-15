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
	auto result = GenIR(env, stmt->RetType);
	auto memberof = GenIR(env, stmt->MemberOf);

	std::vector<type_t*> args;
	for (auto& param : stmt->Parameters)
	{
		auto type = GenIR(env, param.Type);
		args.push_back(type);
	}

	auto function = env->GetFunction(stmt->Name, memberof, result, args, stmt->IsVarArg, stmt->IsConstructor);
	if (!function)
	{
		function = env->CreateFunction(stmt->Name, memberof, result, args, stmt->IsVarArg, stmt->IsConstructor);

		size_t i = 0;
		for (auto& arg : (**function)->args())
			arg.setName(stmt->Parameters[i++].Name);
	}

	if (!function || !**function)
	{
		std::cerr << "Failed to generate ir for function '" << stmt->Name << "': function creation failed" << std::endl;
		return;
	}

	if (!stmt->Body)
		return;

	if (!(**function)->empty())
	{
		std::cerr << "Failed to generate ir for function '" << stmt->Name << "': function already defined" << std::endl;
		return;
	}

	auto entryblock = llvm::BasicBlock::Create(env->Context(), "function.entry", **function);
	env->Builder().SetInsertPoint(entryblock);
	env->IsGlobal() = false;

	env->LocalVariables().clear();
	size_t i = 0;
	for (auto& arg : (**function)->args())
	{
		auto type = GenIR(env, stmt->Parameters[i++].Type);
		auto ptrty = new type_t(llvm::PointerType::get(*type, 0), type);
		auto ptr = env->Builder().CreateAlloca(*type, nullptr, arg.getName());
		env->Builder().CreateStore(&arg, ptr);
		env->LocalVariables()[std::string(arg.getName())] = new value_t(ptr, ptrty);
	}

	GenIR(env, stmt->Body);

	env->IsGlobal() = true;

	if (llvm::verifyFunction(***function, &llvm::errs()))
	{
		function->Function->dump();
		(**function)->eraseFromParent();
		std::cerr << "Failed to generate ir for function '" << stmt->Name << "': function verification failed" << std::endl;
		return;
	}

	env->Optimize(**function);
}

void csaw::GenIR(const EnvPtr& env, const IfStmtPtr& stmt)
{
	auto parent = env->Builder().GetInsertBlock()->getParent();
	auto trueblock = llvm::BasicBlock::Create(env->Context(), "if.true", parent);
	auto falseblock = llvm::BasicBlock::Create(env->Context(), "if.false", parent);
	auto exitblock = llvm::BasicBlock::Create(env->Context(), "if.exit", parent);

	auto condition = GenIRBool(env, *GenIR(env, stmt->Condition));
	env->Builder().CreateCondBr(condition, trueblock, falseblock);

	env->Builder().SetInsertPoint(trueblock);
	GenIR(env, stmt->True);
	if (!env->Builder().GetInsertBlock()->getTerminator())
		env->Builder().CreateBr(exitblock);

	env->Builder().SetInsertPoint(falseblock);
	if (stmt->False) GenIR(env, stmt->False);
	if (!env->Builder().GetInsertBlock()->getTerminator())
		env->Builder().CreateBr(exitblock);

	env->Builder().SetInsertPoint(exitblock);
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
	else value = env->GetEmptyValue(type);

	llvm::Value* ptr = nullptr;

	if (env->IsGlobal())
	{
		llvm::Constant* constant = nullptr;
		if (*value) constant = llvm::dyn_cast<llvm::Constant>(value->Value);
		ptr = new llvm::GlobalVariable(env->Module(), *type, false, llvm::GlobalValue::PrivateLinkage, constant, stmt->Name);
		env->Variables()[stmt->Name] = new value_t(ptr, new type_t(ptr->getType(), type));
	}
	else
	{
		ptr = env->Builder().CreateAlloca(*type, nullptr, stmt->Name);
		if (*value) env->Builder().CreateStore(*value, ptr);
		env->LocalVariables()[stmt->Name] = new value_t(ptr, new type_t(ptr->getType(), type));
	}
}

void csaw::GenIR(const EnvPtr& env, const WhileStmtPtr& stmt)
{
	auto parent = env->Builder().GetInsertBlock()->getParent();
	auto condblock = llvm::BasicBlock::Create(env->Context(), "while.condition", parent);
	auto loopblock = llvm::BasicBlock::Create(env->Context(), "while.loop", parent);
	auto exitblock = llvm::BasicBlock::Create(env->Context(), "while.exit", parent);

	env->Builder().CreateBr(condblock);

	env->Builder().SetInsertPoint(condblock);
	auto condition = GenIRBool(env, *GenIR(env, stmt->Condition));
	env->Builder().CreateCondBr(condition, loopblock, exitblock);

	env->Builder().SetInsertPoint(loopblock);
	GenIR(env, stmt->Body);
	if (!env->Builder().GetInsertBlock()->getTerminator())
		env->Builder().CreateBr(condblock);

	env->Builder().SetInsertPoint(exitblock);
}
