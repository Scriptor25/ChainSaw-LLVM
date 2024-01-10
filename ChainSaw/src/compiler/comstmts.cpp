#include "../parser/parser.h"
#include "compiler.h"

#include <iostream>
#include <llvm/IR/Verifier.h>

void csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<Stmt>& stmt)
{
	if (auto s = std::dynamic_pointer_cast<AliasStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<EnclosedStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<ForStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<FunStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<IfStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<IncStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<RetStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<ThingStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<VarStmt>(stmt))
		return GenIR(env, s);
	if (auto s = std::dynamic_pointer_cast<WhileStmt>(stmt))
		return GenIR(env, s);

	if (auto e = std::dynamic_pointer_cast<Expr>(stmt))
	{
		GenIR(env, e);
		return;
	}

	throw "TODO";
}

void csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<AliasStmt>& stmt)
{
	throw "TODO";
}

void csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<EnclosedStmt>& stmt)
{
	auto e = std::make_shared<Environment>(env);
	for (auto& s : stmt->Body)
		GenIR(e, s);
}

void csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<ForStmt>& stmt)
{
	auto fun = Environment::Builder().GetInsertBlock()->getParent();
	auto bcondition = llvm::BasicBlock::Create(Environment::Context(), "for_condition");
	auto bloop = llvm::BasicBlock::Create(Environment::Context(), "for_loop");
	auto bmerge = llvm::BasicBlock::Create(Environment::Context(), "for_merge");

	auto e = std::make_shared<Environment>(env);

	GenIR(e, stmt->Begin);
	Environment::Builder().CreateBr(bcondition);

	fun->insert(fun->end(), bcondition);
	Environment::Builder().SetInsertPoint(bcondition);
	auto vcondition = GenIR(e, stmt->Condition);
	vcondition.value = NumToBool(vcondition());
	Environment::Builder().CreateCondBr(vcondition(), bloop, bmerge);

	fun->insert(fun->end(), bloop);
	Environment::Builder().SetInsertPoint(bloop);
	GenIR(e, stmt->Body);
	GenIR(e, stmt->Loop);
	Environment::Builder().CreateBr(bcondition);

	fun->insert(fun->end(), bmerge);
	Environment::Builder().SetInsertPoint(bmerge);
}

void csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<FunStmt>& stmt)
{
	std::vector<llvm::Type*> types;
	std::vector<type_t> argtypes;

	auto ret = GenIR(stmt->RetType);

	type_t memberof;
	bool hasExtra = stmt->MemberOf || stmt->IsConstructor;
	if (stmt->MemberOf)
	{
		memberof = GenIR(stmt->MemberOf);
		types.push_back(memberof.type);
		argtypes.push_back(memberof);
	}
	else if (stmt->IsConstructor)
	{
		types.push_back(ret.type);
	}

	for (auto& parameter : stmt->Parameters)
	{
		auto param = GenIR(parameter.Type);
		types.push_back(param.type);
		argtypes.push_back(param);
	}

	auto fun = Environment::GetFunction(memberof, stmt->Name, argtypes);
	if (!fun)
	{
		auto funtype = llvm::FunctionType::get(ret.type, types, stmt->IsVarArg);
		fun.fun = llvm::Function::Create(funtype, llvm::Function::ExternalLinkage, stmt->Name, Environment::Module());
		fun.type = ret;
		fun.argtypes = argtypes;
		fun.isconstructor = stmt->IsConstructor;
		Environment::CreateFunction(memberof, stmt->Name, fun);
	}

	if (!stmt->Body)
		return;

	if (!(fun()->empty()))
		throw "cannot redefine function";

	auto entry = llvm::BasicBlock::Create(Environment::Context(), "fun_entry", fun());
	Environment::Builder().SetInsertPoint(entry);

	auto e = std::make_shared<Environment>(env);

	int i = hasExtra ? -1 : 0;
	for (auto& arg : fun()->args())
	{
		auto& name = i < 0 ? "my" : stmt->Parameters[i].Name;
		arg.setName(name);
		auto type = i < 0 ? (stmt->IsConstructor ? ret : memberof) : GenIR(stmt->Parameters[i].Type);
		e->CreateVariable(name, value_t(&arg, type));
		i++;
	}

	llvm::Value* valist = nullptr;
	if (fun()->isVarArg())
	{
		auto i8ty = Environment::Builder().getInt8Ty();
		auto ptrty = llvm::PointerType::get(i8ty, 0);
		valist = env->SetVarArgs(Environment::Builder().CreateAlloca(ptrty, nullptr, "valist"));

		auto vastartfun = Environment::Module().getFunction("llvm.va_start");
		if (!vastartfun)
		{
			auto funtype = llvm::FunctionType::get(Environment::Builder().getVoidTy(), { ptrty }, false);
			vastartfun = llvm::Function::Create(funtype, llvm::Function::ExternalLinkage, "llvm.va_start", Environment::Module());
		}

		auto vaendfun = Environment::Module().getFunction("llvm.va_end");
		if (!vaendfun)
		{
			auto funtype = llvm::FunctionType::get(Environment::Builder().getVoidTy(), { ptrty }, false);
			vaendfun = llvm::Function::Create(funtype, llvm::Function::ExternalLinkage, "llvm.va_end", Environment::Module());
		}

		Environment::Builder().CreateCall(vastartfun, { valist });
	}

	GenIR(e, stmt->Body);

	for (auto& bb : *fun())
	{
		auto terminator = bb.getTerminator();
		if (terminator)
		{
			if (fun()->isVarArg())
				Environment::Builder().CreateCall(Environment::Module().getFunction("llvm.va_end"), { valist })->moveBefore(terminator);
			continue;
		}
		if (fun()->getReturnType()->isVoidTy())
		{
			Environment::Builder().SetInsertPoint(&bb);
			if (fun()->isVarArg())
				Environment::Builder().CreateCall(Environment::Module().getFunction("llvm.va_end"), { valist });

			Environment::Builder().CreateRetVoid();
			continue;
		}
		if (stmt->IsConstructor)
		{
			Environment::Builder().SetInsertPoint(&bb);
			if (fun()->isVarArg())
				Environment::Builder().CreateCall(Environment::Module().getFunction("llvm.va_end"), { valist });

			Environment::Builder().CreateRet(e->GetVariable("my").value);
			continue;
		}
		throw "missing non-void terminator";
	}

	if (llvm::verifyFunction(*fun(), &llvm::errs()))
	{
		fun()->print(llvm::errs());
		throw "failed to verify function";
	}

	Environment::Optimize(fun());

	Environment::Builder().SetInsertPoint(&Environment::Module().getFunction("__global__")->back()); // insert global stuff into the global init function
}

void csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<IfStmt>& stmt)
{
	auto fun = Environment::Builder().GetInsertBlock()->getParent();
	auto bthen = llvm::BasicBlock::Create(Environment::Context(), "if_then", fun);
	auto belse = llvm::BasicBlock::Create(Environment::Context(), "if_else");
	auto bmerge = llvm::BasicBlock::Create(Environment::Context(), "if_merge");

	auto vcondition = GenIR(env, stmt->Condition);
	vcondition.value = NumToBool(vcondition());
	Environment::Builder().CreateCondBr(vcondition(), bthen, belse);

	Environment::Builder().SetInsertPoint(bthen);
	GenIR(env, stmt->Then);
	if (!bthen->getTerminator())
		Environment::Builder().CreateBr(bmerge);

	fun->insert(fun->end(), belse);
	Environment::Builder().SetInsertPoint(belse);
	if (stmt->Else)
		GenIR(env, stmt->Else);
	if (!belse->getTerminator())
		Environment::Builder().CreateBr(bmerge);

	fun->insert(fun->end(), bmerge);
	Environment::Builder().SetInsertPoint(bmerge);
}

void csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<IncStmt>& stmt)
{
	if (!env->IsTopLevel())
		throw "environment must be top level = no parent";

	auto path = env->Path().parent_path() / stmt->Path;
	if (!ParseInc(env, path))
		throw "failed to parse included file";
}

void csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<RetStmt>& stmt)
{
	if (!stmt)
		Environment::Builder().CreateRetVoid();
	else
		Environment::Builder().CreateRet(GenIR(env, stmt->Value).value);
}

void csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<ThingStmt>& stmt)
{
	if (stmt->Fields.empty())
	{
		auto strtype = llvm::StructType::create(Environment::Context(), stmt->Name);
		Environment::CreateType(stmt->Name, stmt->Group, strtype, {});
		return;
	}

	std::vector<llvm::Type*> elements;
	std::vector<std::string> fields;
	for (auto& field : stmt->Fields)
	{
		elements.push_back(GenIR(field.Type).type);
		fields.push_back(field.Name);
	}

	auto strtype = llvm::StructType::create(Environment::Context(), elements, stmt->Name);
	Environment::CreateType(stmt->Name, stmt->Group, strtype, fields);
}

void csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<VarStmt>& stmt)
{
	auto type = GenIR(stmt->Type);

	if (env->IsTopLevel())
	{
		llvm::Constant* initializer = nullptr;
		if (stmt->Value)
			initializer = llvm::dyn_cast<llvm::Constant>(GenIR(env, stmt->Value)());
		auto value = value_t(new llvm::GlobalVariable(Environment::Module(), type.type, false, llvm::GlobalValue::ExternalLinkage, initializer, stmt->Name), type);
		env->CreateVariable(stmt->Name, value, true);
		return;
	}

	value_t value;
	if (stmt->Value) value = GenIR(env, stmt->Value);
	else value = Environment::GetNull(type);

	env->CreateVariable(stmt->Name, value);
}

void csaw::GenIR(const std::shared_ptr<Environment>& env, const std::shared_ptr<WhileStmt>& stmt)
{
	auto fun = Environment::Builder().GetInsertBlock()->getParent();
	auto bcondition = llvm::BasicBlock::Create(Environment::Context(), "while_condition");
	auto bloop = llvm::BasicBlock::Create(Environment::Context(), "while_loop");
	auto bmerge = llvm::BasicBlock::Create(Environment::Context(), "while_merge");

	Environment::Builder().CreateBr(bcondition);

	fun->insert(fun->end(), bcondition);
	Environment::Builder().SetInsertPoint(bcondition);
	auto vcondition = GenIR(env, stmt->Condition);
	vcondition.value = NumToBool(vcondition());
	Environment::Builder().CreateCondBr(vcondition(), bloop, bmerge);

	fun->insert(fun->end(), bloop);
	Environment::Builder().SetInsertPoint(bloop);
	GenIR(env, stmt->Body);
	Environment::Builder().CreateBr(bcondition);

	fun->insert(fun->end(), bmerge);
	Environment::Builder().SetInsertPoint(bmerge);
}
