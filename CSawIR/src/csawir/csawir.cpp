#include <csawir/csawir.h>

void csawir::Inst::Append(Inst* inst)
{
	if (!Next)
	{
		Next = inst;
		Next->Prev = this;
		return;
	}
	Next->Append(inst);
}

void csawir::Function::Append(const std::string& label, Inst* inst)
{
	if (auto ptr = Labels[label])
		ptr->Append(inst);
	else
		Labels[label] = inst;

	if (!Entry)
		Entry = inst;
}

csawir::Const* csawir::Const::Parse(Type* type, const std::string& value)
{
	if (type->Name == "num")
		return new ConstNum(type, std::stod(value));
	if (type->Name == "str")
		return new ConstStr(type, value);

	throw;
}

std::ostream& csawir::operator<<(std::ostream& out, const Type& type)
{
	return out << type.Name;
}

std::ostream& csawir::operator<<(std::ostream& out, const Value& value)
{
	if (auto v = dynamic_cast<const RegValue*>(&value))
		return out << *v;

	if (auto v = dynamic_cast<const Const*>(&value))
		return out << *v;
	if (auto v = dynamic_cast<const Inst*>(&value))
		return out << *v;

	throw;
}

std::ostream& csawir::operator<<(std::ostream& out, const RegValue& value)
{
	return out << '%' << value.Name << ' ' << *value.Type;
}

std::ostream& csawir::operator<<(std::ostream& out, const Const& cnst)
{
	if (auto c = dynamic_cast<const ConstNum*>(&cnst))
		return out << *c;
	if (auto c = dynamic_cast<const ConstStr*>(&cnst))
		return out << *c;

	throw;
}

std::ostream& csawir::operator<<(std::ostream& out, const ConstNum& cnst)
{
	return out << "const num " << std::to_string(cnst.Value);
}

static void replace(std::string& str, const std::string& from, const std::string& to)
{
	size_t start_pos;
	while ((start_pos = str.find(from)) != std::string::npos)
		str.replace(start_pos, from.length(), to);
}

std::ostream& csawir::operator<<(std::ostream& out, const ConstStr& cnst)
{
	std::string value = cnst.Value;
	replace(value, "\a", "\\a");
	replace(value, "\b", "\\b");
	replace(value, "\t", "\\t");
	replace(value, "\n", "\\n");
	replace(value, "\v", "\\v");
	replace(value, "\f", "\\f");
	replace(value, "\r", "\\r");
	return out << "const str \"" << value << '"';
}

std::ostream& csawir::operator<<(std::ostream& out, const Inst& inst)
{
	if (auto i = dynamic_cast<const CallInst*>(&inst))
		return out << *i;
	if (auto i = dynamic_cast<const RetInst*>(&inst))
		return out << *i;
	if (auto i = dynamic_cast<const SetInst*>(&inst))
		return out << *i;
	if (auto i = dynamic_cast<const AllocInst*>(&inst))
		return out << *i;
	if (auto i = dynamic_cast<const StoreInst*>(&inst))
		return out << *i;
	if (auto i = dynamic_cast<const JmpInst*>(&inst))
		return out << *i;
	if (auto i = dynamic_cast<const CJmpInst*>(&inst))
		return out << *i;
	if (auto i = dynamic_cast<const LoadInst*>(&inst))
		return out << *i;

	throw;
}

std::ostream& csawir::operator<<(std::ostream& out, const CallInst& inst)
{
	out << "call " << inst.Callee->Name;
	for (auto& arg : inst.Args)
		out << ' ' << *arg;
	return out;
}

std::ostream& csawir::operator<<(std::ostream& out, const RetInst& inst)
{
	out << "ret ";
	if (!inst.RetValue)
		return out << "void";
	return out << *inst.RetValue;
}

std::ostream& csawir::operator<<(std::ostream& out, const SetInst& inst)
{
	return out << '%' << inst.Name << ' ' << *inst.Value;
}

std::ostream& csawir::operator<<(std::ostream& out, const AllocInst& inst)
{
	return out << "alloc " << *inst.Type;
}

std::ostream& csawir::operator<<(std::ostream& out, const StoreInst& inst)
{
	return out << "store " << *inst.Ptr << ' ' << *inst.Value;
}

std::ostream& csawir::operator<<(std::ostream& out, const JmpInst& inst)
{
	return out << "jmp " << inst.Label;
}

std::ostream& csawir::operator<<(std::ostream& out, const CJmpInst& inst)
{
	return out << "jmp " << *inst.Condition << ' ' << inst.LabelTrue << ' ' << inst.LabelFalse;
}

std::ostream& csawir::operator<<(std::ostream& out, const LoadInst& inst)
{
	return out << "load " << *inst.Ptr;
}

std::ostream& csawir::operator<<(std::ostream& out, const Function& function)
{
	out << (function.IsDefined() ? "def " : "dec ") << *function.Type->RetType << ' ' << function.Name;
	for (size_t i = 0; i < function.Type->Args.size(); i++)
		out << ' ' << *function.Type->Args[i] << ' ' << (i >= function.Args.size() ? "v" + std::to_string(i) : function.Args[i]);
	out << std::endl;

	if (!function.IsDefined())
		return out;

	{
		auto ptr = function.Entry;
		while (ptr)
		{
			out << "\t" << *ptr << std::endl;
			ptr = ptr->Next;
		}
	}

	for (auto& label : function.Labels)
	{
		if (label.second == function.Entry)
			continue;
		out << '@' << label.first << std::endl;
		auto ptr = label.second;
		while (ptr)
		{
			out << "\t" << *ptr << std::endl;
			ptr = ptr->Next;
		}
	}

	return out << "enddef" << std::endl;
}
