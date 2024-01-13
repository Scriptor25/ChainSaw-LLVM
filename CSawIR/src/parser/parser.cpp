#include <csawir/parser.h>

#include <fstream>
#include <iostream>

bool cir::ParseFile(const std::string& filename)
{
	std::ifstream stream(filename);
	if (!stream)
		return false;

	ParseStream(stream);
	return true;
}

void cir::ParseStream(std::istream& stream)
{
	Parser parser(stream);
	parser.Next();
	while (!parser.AtEof())
	{
		parser.Next();
	}
}

int cir::Parser::read()
{
	int c = m_Stream.get();
	if (m_Index < m_Limit)
		m_Peek[m_Index++] = c;
	return c;
}

void cir::Parser::mark(int limit)
{
	m_Peek = new char[limit];
	m_Index = 0;
	m_Limit = limit;
}

void cir::Parser::reset()
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

std::shared_ptr<cir::Token> cir::Parser::Next()
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

std::shared_ptr<cir::Token> cir::Parser::Current()
{
	return m_Current;
}

bool cir::Parser::AtEof() const
{
	return m_Current->Type == TOKEN_EOF;
}

bool cir::Parser::At(const std::string& value) const
{
	return !AtEof() && m_Current->Value == value;
}

bool cir::Parser::At(const TokenType type) const
{
	return !AtEof() && m_Current->Type == type;
}

bool cir::Parser::Expect(const std::string& value) const
{
	if (At(value))
		return true;
	std::cerr << "unexpected token " << m_Current << ", expected value '" << value << "'" << std::endl;
	throw;
}

bool cir::Parser::Expect(const TokenType type) const
{
	if (At(type))
		return true;
	std::cerr << "unexpected token " << m_Current << ", expected type '" << type << "'" << std::endl;
	throw;
}

bool cir::Parser::ExpectAndNext(const std::string& value)
{
	Expect(value);
	Next();
	return true;
}

bool cir::Parser::ExpectAndNext(const TokenType type)
{
	Expect(type);
	Next();
	return true;
}
