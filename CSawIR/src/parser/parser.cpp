#include <csawir/parser.h>

#include <fstream>
#include <iostream>
#include <sstream>

bool csawir::ParseFile(const std::string& filename)
{
	std::ifstream stream(filename);
	if (!stream)
		return false;

	ParseStream(stream);
	return true;
}

void csawir::ParseStream(std::istream& stream)
{
	Parser parser(stream);
	parser.NextLine();
	while (!parser.AtEof())
	{
		if (parser.At("def") || parser.At("dec"))
		{
			parser.NextFunction();
			continue;
		}

		std::cout << "Warning: not parsed tokens: ";
		while (!parser.AtEol())
		{
			std::cout << parser.Current() << ", ";
			parser.Next();
		}

		parser.NextLine();
		std::cout << std::endl;
	}

	std::cout << parser.GetContext() << std::endl;
}

static void replace(std::string& str, const std::string& from, const std::string& to)
{
	size_t start_pos;
	while ((start_pos = str.find(from)) != std::string::npos)
		str.replace(start_pos, from.length(), to);
}

std::vector<csawir::Token>& csawir::Parser::NextLine()
{
	if (m_Stream.eof())
	{
		m_Current.clear();
		m_Current.push_back(Token(m_Line));
		m_Index = 0;
		return m_Current;
	}

	std::string line;
	std::getline(m_Stream, line);

	m_Index = 0;
	m_Line++;

	std::stringstream linestream(line);
	std::string segment;

	std::string fusestr;

	m_Current.clear();
	while (std::getline(linestream, segment, ' '))
		if (!segment.empty())
		{
			TokenType type;

			if (!fusestr.empty())
			{
				segment = fusestr + ' ' + segment;
				fusestr.clear();
			}

			replace(segment, "\t", "");
			if (isdigit(segment[0]) || segment[0] == '.')
			{
				type = TOKEN_NUMBER;
			}
			else if (isalpha(segment[0]) || segment[0] == '_')
			{
				type = TOKEN_IDENTIFIER;
			}
			else if (segment[0] == '"')
			{
				if (segment[segment.size() - 1] != '"')
				{
					fusestr = segment;
					continue;
				}

				segment = segment.substr(1, segment.size() - 2);
				replace(segment, "\\a", "\a");
				replace(segment, "\\b", "\b");
				replace(segment, "\\t", "\t");
				replace(segment, "\\n", "\n");
				replace(segment, "\\v", "\v");
				replace(segment, "\\f", "\f");
				replace(segment, "\\r", "\r");
				type = TOKEN_STRING;
			}
			else if (segment[0] == '\'')
			{
				segment = segment.substr(1, segment.size() - 2);
				replace(segment, "\\a", "\a");
				replace(segment, "\\b", "\b");
				replace(segment, "\\t", "\t");
				replace(segment, "\\n", "\n");
				replace(segment, "\\v", "\v");
				replace(segment, "\\f", "\f");
				replace(segment, "\\r", "\r");
				type = TOKEN_CHAR;
			}
			else if (segment[0] == '%')
			{
				segment = segment.substr(1);
				type = TOKEN_REGISTER;
			}
			else if (segment[0] == '@')
			{
				segment = segment.substr(1);
				type = TOKEN_BLOCK;
			}
			else if (segment[0] == '$')
			{
				segment = segment.substr(1);
				type = TOKEN_GLOBAL;
			}
			else
			{
				type = TOKEN_OPERATOR;
			}

			m_Current.push_back(Token(type, segment, m_Line));
		}

	if (m_Current.empty())
		return NextLine();

	return m_Current;
}

void csawir::Parser::Next()
{
	++m_Index;
}

const csawir::Token& csawir::Parser::Current() const
{
	return m_Current[m_Index];
}

bool csawir::Parser::AtEol() const
{
	return m_Index >= m_Current.size();
}

bool csawir::Parser::AtEof() const
{
	return Current().Type == TOKEN_EOF;
}

bool csawir::Parser::At(const std::string& value) const
{
	return !AtEol() && Current().Value == value;
}

bool csawir::Parser::At(const TokenType type) const
{
	return !AtEol() && Current().Type == type;
}

const csawir::Token& csawir::Parser::Expect(const std::string& value) const
{
	if (At(value))
		return Current();
	std::cerr << "unexpected token " << Current() << ", expected value '" << value << "'" << std::endl;
	throw;
}

const csawir::Token& csawir::Parser::Expect(const TokenType type) const
{
	if (At(type))
		return Current();
	std::cerr << "unexpected token " << Current() << ", expected type '" << type << "'" << std::endl;
	throw;
}

const csawir::Token& csawir::Parser::ExpectAndNext(const std::string& value)
{
	auto& token = Expect(value);
	Next();
	return token;
}

const csawir::Token& csawir::Parser::ExpectAndNext(const TokenType type)
{
	auto& token = Expect(type);
	Next();
	return token;
}

void csawir::Parser::NextFunction()
{
	bool declare = At("dec");
	if (!declare) ExpectAndNext("def");
	else Next();

	std::string rettype = Current().Value; Next();
	std::string funname = Current().Value; Next();

	bool varargs = false;

	std::vector<std::pair<std::string, std::string>> funargs;
	while (!AtEol())
	{
		if (At("?"))
		{
			varargs = true;
			break;
		}

		std::string argtype = Current().Value; Next();
		std::string argname = Current().Value; Next();
		funargs.push_back({ argtype, argname });
	}

	NextLine();

	std::vector<Type*> args;
	for (auto& arg : funargs)
		args.push_back(m_Context.GetType(arg.first));
	auto funtype = m_Context.GetFunctionType(m_Context.GetType(rettype), args, varargs);
	auto function = m_Context.GetFunction(funname, funtype);
	if (!function)
		function = m_Context.CreateFunction(funname, funtype);

	if (declare)
		return;

	if (function->IsDefined())
		throw;

	for (auto& arg : funargs)
		function->Args.push_back(arg.second);

	std::string block;
	while (!At("enddef"))
	{
		if (At(TOKEN_BLOCK))
		{
			block = Current().Value;
			NextLine();
		}
		function->Append(block, NextHighLevel());
	}

	NextLine();
}

csawir::Inst* csawir::Parser::NextHighLevel()
{
	if (At(TOKEN_REGISTER))
	{
		std::string reg = Current().Value;
		Next();

		auto inst = NextInst();
		if (!inst)
			throw;

		NextLine();
		return new SetInst(reg, inst);
	}

	if (At("ret"))
	{
		Next();
		if (AtEol())
			return new RetInst();

		auto value = NextValue();
		if (!value)
			throw;

		NextLine();
		return new RetInst(value);
	}

	if (At("store"))
	{
		Next();

		auto ptr = NextValue();
		if (!ptr)
			throw;

		auto value = NextValue();
		if (!value)
			throw;

		NextLine();
		return new StoreInst(ptr, value);
	}

	if (At("jmp"))
	{
		Next();
		std::string label = Current().Value;
		NextLine();
		return new JmpInst(label);
	}

	if (At("cjmp"))
	{
		Next();

		auto condition = NextValue();
		if (!condition)
			throw;

		std::string labeltrue = Current().Value;
		Next();
		std::string labelfalse = Current().Value;

		NextLine();
		return new CJmpInst(condition, labeltrue, labelfalse);
	}

	auto inst = NextInst();
	NextLine();
	return inst;
}

csawir::Inst* csawir::Parser::NextInst()
{
	if (At("call"))
	{
		Next();
		std::string callee = Current().Value;
		Next();
		std::vector<Value*> args;
		std::vector<Type*> argtypes;
		while (!AtEol())
		{
			auto value = NextValue();
			if (!value)
				throw;

			args.push_back(value);
			argtypes.push_back(value->GetType());
		}

		auto calleefun = m_Context.GetFunction(callee, argtypes);
		if (!calleefun)
			throw;

		return new CallInst(calleefun, args);
	}

	if (At("alloc"))
	{
		Next();

		auto type = m_Context.GetType(Current().Value);
		if (!type)
			throw;

		Next();
		return new AllocInst(type);
	}

	if (At("load"))
	{
		Next();

		auto ptr = NextValue();
		if (!ptr)
			throw;

		return new LoadInst(ptr);
	}

	throw;
}

csawir::Value* csawir::Parser::NextValue()
{
	if (At(TOKEN_REGISTER))
	{
		std::string reg = Current().Value;
		Next();

		auto type = m_Context.GetType(Current().Value);
		if (!type)
			throw;

		Next();
		return new RegValue(type, reg);
	}

	return NextConst();
}

csawir::Const* csawir::Parser::NextConst()
{
	if (At("const"))
	{
		Next();

		auto type = m_Context.GetType(Current().Value);
		if (!type)
			throw;

		Next();
		std::string value = Current().Value;
		Next();
		return Const::Parse(type, value);
	}

	throw;
}
