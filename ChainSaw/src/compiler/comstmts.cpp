#include <csaw/compiler.h>
#include <csaw/parser.h>

#include <iostream>

#include <llvm/IR/Verifier.h>

void csaw::GenIR(const EnvironmentPtr& env, const StmtPtr& stmt)
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

void csaw::GenIR(const EnvironmentPtr& env, const AliasStmtPtr& stmt)
{
	auto origin = GenIR(stmt->Origin);
	return Environment::CreateAlias(stmt->Alias, origin);
}

void csaw::GenIR(const EnvironmentPtr& env, const EnclosedStmtPtr& stmt)
{
	auto e = std::make_shared<Environment>(env);
	for (auto& s : stmt->Body)
		GenIR(e, s);
}

void csaw::GenIR(const EnvironmentPtr& env, const ForStmtPtr& stmt)
{
	auto fun = Environment::Builder().GetInsertBlock()->getParent();
	auto bheader = llvm::BasicBlock::Create(Environment::Context(), "loop.header", fun);
	auto bbody = llvm::BasicBlock::Create(Environment::Context(), "loop.body", fun);
	auto bexit = llvm::BasicBlock::Create(Environment::Context(), "loop.exit", fun);

	auto e = std::make_shared<Environment>(env);

	GenIR(e, stmt->Begin);
	Environment::Builder().CreateBr(bheader);

	Environment::Builder().SetInsertPoint(bheader);
	auto vcondition = GenIR(e, stmt->Condition);
	auto condition = NumToBool(vcondition());
	Environment::Builder().CreateCondBr(condition, bbody, bexit);

	Environment::Builder().SetInsertPoint(bbody);
	GenIR(e, stmt->Body);
	GenIR(e, stmt->Loop);
	Environment::Builder().CreateBr(bheader);

	Environment::Builder().SetInsertPoint(bexit);
}

void csaw::GenIR(const EnvironmentPtr& env, const FunStmtPtr& stmt)
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

	auto entry = llvm::BasicBlock::Create(Environment::Context(), "entry", fun());
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

	GenIR(e, stmt->Body);

	for (auto& bb : *fun())
	{
		auto terminator = bb.getTerminator();
		if (terminator)
			continue;

		if (fun()->getReturnType()->isVoidTy())
		{
			Environment::Builder().SetInsertPoint(&bb);
			Environment::Builder().CreateRetVoid();
			continue;
		}

		if (stmt->IsConstructor)
		{
			Environment::Builder().SetInsertPoint(&bb);
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

void csaw::GenIR(const EnvironmentPtr& env, const IfStmtPtr& stmt)
{
	auto fun = Environment::Builder().GetInsertBlock()->getParent();
	auto bthen = llvm::BasicBlock::Create(Environment::Context(), "if.then", fun);
	auto belse = llvm::BasicBlock::Create(Environment::Context(), "if.else");
	auto bexit = llvm::BasicBlock::Create(Environment::Context(), "if.exit");

	auto vcondition = GenIR(env, stmt->Condition);
	vcondition.value = NumToBool(vcondition());
	Environment::Builder().CreateCondBr(vcondition(), bthen, belse);

	Environment::Builder().SetInsertPoint(bthen);
	GenIR(env, stmt->Then);
	if (!bthen->getTerminator())
		Environment::Builder().CreateBr(bexit);

	fun->insert(fun->end(), belse);
	Environment::Builder().SetInsertPoint(belse);
	if (stmt->Else)
		GenIR(env, stmt->Else);
	if (!belse->getTerminator())
		Environment::Builder().CreateBr(bexit);

	fun->insert(fun->end(), bexit);
	Environment::Builder().SetInsertPoint(bexit);
}

void csaw::GenIR(const EnvironmentPtr& env, const IncStmtPtr& stmt)
{
	if (!env->IsTopLevel())
		throw "environment must be top level = no parent";

	auto path = env->Path().parent_path() / stmt->Path;
	if (!ParseInc(env, path))
		throw "failed to parse included file";
}

void csaw::GenIR(const EnvironmentPtr& env, const RetStmtPtr& stmt)
{
	if (!stmt->Value)
		Environment::Builder().CreateRetVoid();
	else
		Environment::Builder().CreateRet(GenIR(env, stmt->Value).value);
}

void csaw::GenIR(const EnvironmentPtr& env, const ThingStmtPtr& stmt)
{
	if (stmt->Fields.empty())
	{
		auto strtype = llvm::StructType::create(Environment::Context(), stmt->Name);
		Environment::CreateType(stmt->Name, strtype, {});
		return;
	}

	std::vector<llvm::Type*> elements;
	std::vector<std::pair<std::string, type_t>> fields;
	for (auto& field : stmt->Fields)
	{
		auto type = GenIR(field.Type);
		elements.push_back(type.type);
		fields.push_back({ field.Name, type });
	}

	auto type = Environment::GetType(stmt->Name);
	llvm::StructType* strtype;
	if (!type)
		strtype = llvm::StructType::create(Environment::Context(), stmt->Name);
	else if (type->fields.empty())
		strtype = type->type;
	else
	{
		throw;
	}

	strtype->setBody(elements);
	Environment::CreateType(stmt->Name, strtype, fields);
}

void csaw::GenIR(const EnvironmentPtr& env, const VarStmtPtr& stmt)
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

void csaw::GenIR(const EnvironmentPtr& env, const WhileStmtPtr& stmt)
{
	auto fun = Environment::Builder().GetInsertBlock()->getParent();
	auto bheader = llvm::BasicBlock::Create(Environment::Context(), "loop.header", fun);
	auto bbody = llvm::BasicBlock::Create(Environment::Context(), "loop.body", fun);
	auto bexit = llvm::BasicBlock::Create(Environment::Context(), "loop.exit", fun);

	Environment::Builder().CreateBr(bheader);

	Environment::Builder().SetInsertPoint(bheader);
	auto vcondition = GenIR(env, stmt->Condition);
	auto condition = NumToBool(vcondition());
	Environment::Builder().CreateCondBr(condition, bbody, bexit);

	Environment::Builder().SetInsertPoint(bbody);
	GenIR(env, stmt->Body);
	Environment::Builder().CreateBr(bheader);

	Environment::Builder().SetInsertPoint(bexit);
}
