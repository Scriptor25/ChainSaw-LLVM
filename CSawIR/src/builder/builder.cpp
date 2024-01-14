#include <csawir/builder.h>

void csawir::Builder::Append(Inst* inst)
{
	m_InsertFunction->Append(m_InsertLabel, inst);
}

csawir::RetInst* csawir::Builder::AppendRet(Value* value)
{
	auto inst = new RetInst(value);
	Append(inst);
	return inst;
}

csawir::CallInst* csawir::Builder::AppendCall(Function* callee, const std::vector<Value*>& args)
{
	auto inst = new CallInst(callee, args);
	Append(inst);
	return inst;
}

csawir::ConstNum* csawir::Builder::CreateNum(double value)
{
	return new ConstNum(m_Context.GetNumType(), value);
}
