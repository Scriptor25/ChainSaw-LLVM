#include "parser.h"

#include <fstream>
#include <iostream>

bool csaw::Parse(const std::shared_ptr<Environment>& env, const std::string& filename)
{
	std::ifstream stream(filename);
	if (!stream)
		return false;

	return Parse(env, stream);
}

bool csaw::Parse(const std::shared_ptr<Environment>& env, std::istream& stream)
{
	Parser parser(stream);
	parser.Next();
	while (!parser.AtEof())
	{
		auto stmt = parser.NextStmt(true);
		GenIR(env, stmt);
	}

	return true;
}

bool csaw::ParseInc(const std::shared_ptr<Environment>& env, const std::filesystem::path& filepath)
{
	std::ifstream stream(filepath);
	if (!stream)
		return false;

	auto prev = env->Path();
	env->Path(filepath);

	Parser parser(stream);
	parser.Next();
	while (!parser.AtEof())
	{
		auto stmt = parser.NextStmt(true);
		GenIR(env, stmt);
	}

	env->Path(prev);

	return true;
}

int csaw::Parser::read()
{
	int c = m_Stream.get();
	if (m_Index < m_Limit)
		m_Peek[m_Index++] = c;
	return c;
}

void csaw::Parser::mark(int limit)
{
	m_Peek = new char[limit];
	m_Index = 0;
	m_Limit = limit;
}

void csaw::Parser::reset()
{
	for (int i = m_Index - 1; i >= 0; i--)
		m_Stream.putback(m_Peek[i]);
	m_Limit = 0;
}

static bool isignore(int c)
{
	return 0x00 <= c && c <= 0x20;
}

static int escape(int c)
{
	switch (c) {
	case 'a':
		return '\a';
	case 'b':
		return '\b';
	case 't':
		return '\t';
	case 'n':
		return '\n';
	case 'v':
		return '\v';
	case 'f':
		return '\f';
	case 'r':
		return '\r';
	default:
		return c;
	}
}

std::shared_ptr<csaw::Token> csaw::Parser::Next()
{
	int c = read();

	for (; isignore(c); c = read())
		if (c == '\n')
			m_Line++;

	if (c < 0)
		return m_Current = std::make_shared<Token>(m_Line);

	if (c == '#') {
		c = read();
		const char LIMIT = c == '#' ? '\n' : '#';
		while ((c = read()) != LIMIT && c >= 0)
			if (c == '\n')
				m_Line++;
		if (LIMIT == '\n')
			m_Line++;
		return Next();
	}

	if (isalpha(c) || c == '_') {
		std::string value;
		do {
			value += c;
			mark(1);
			c = read();
		} while (isalnum(c) || c == '_');
		reset();

		return m_Current = std::make_shared<Token>(TOKEN_IDENTIFIER, value, m_Line);
	}

	if (isdigit(c)) {
		std::string value;
		do {
			value += c;
			mark(1);
			int p = c;
			c = read();
			if (((p == 'e' || p == 'E') && c == '-')) {
				value += c;
				mark(1);
				c = read();
			}
		} while (isxdigit(c) || c == '.' || c == 'x' || c == 'b');
		reset();

		return m_Current = std::make_shared<Token>(TOKEN_NUMBER, value, m_Line);
	}

	if (c == '"') {
		std::string value;
		c = read();
		while (c != '"' && c >= 0) {
			if (c == '\\') {
				value += escape(read());
				c = read();
				continue;
			}
			value += c;
			c = read();
		}

		return m_Current = std::make_shared<Token>(TOKEN_STRING, value, m_Line);
	}

	if (c == '\'') {
		std::string value;
		c = read();
		while (c != '\'' && c >= 0) {
			if (c == '\\') {
				value += escape(read());
				c = read();
			}

			value += c;
			c = read();
		}

		return m_Current = std::make_shared<Token>(TOKEN_CHAR, value, m_Line);
	}

	return m_Current = std::make_shared<Token>(TOKEN_OPERATOR, std::string{ (char)c }, m_Line);
}

std::shared_ptr<csaw::Token> csaw::Parser::Current()
{
	return m_Current;
}

bool csaw::Parser::AtEof() const
{
	return m_Current->Type == TOKEN_EOF;
}

bool csaw::Parser::At(const std::string& value) const
{
	return !AtEof() && m_Current->Value == value;
}

bool csaw::Parser::At(const TokenType type) const
{
	return !AtEof() && m_Current->Type == type;
}

bool csaw::Parser::Expect(const std::string& value) const
{
	if (At(value))
		return true;
	std::cerr << "unexpected token " << m_Current << ", expected value '" << value << "'" << std::endl;
	throw;
}

bool csaw::Parser::Expect(const TokenType type) const
{
	if (At(type))
		return true;
	std::cerr << "unexpected token " << m_Current << ", expected type '" << type << "'" << std::endl;
	throw;
}

bool csaw::Parser::ExpectAndNext(const std::string& value)
{
	Expect(value);
	Next();
	return true;
}

bool csaw::Parser::ExpectAndNext(const TokenType type)
{
	Expect(type);
	Next();
	return true;
}

std::shared_ptr<csaw::ASTType> csaw::Parser::NextType()
{
	return NextType(NextIndexExpr());
}

std::shared_ptr<csaw::ASTType> csaw::Parser::NextType(const std::shared_ptr<Expr>& type)
{
	if (auto t = std::dynamic_pointer_cast<IdExpr>(type))
		return ASTType::Get(t->Value);
	if (auto t = std::dynamic_pointer_cast<IndexExpr>(type)) {
		auto type = NextType(t->Object);
		auto size = (size_t)std::dynamic_pointer_cast<NumExpr>(t->Index)->Value;
		return ASTType::Get(type, size);
	}

	std::cerr << "unsupported type expression" << std::endl;
	throw;
}

csaw::ASTParameter csaw::Parser::NextParameter()
{
	auto pname = m_Current->Value;
	ExpectAndNext(TOKEN_IDENTIFIER); // skip name
	ExpectAndNext(":"); // skip :
	auto ptype = NextType();
	return ASTParameter(pname, ptype);
}
