#include "parser.h"

int csaw::Parser::read()
{
	return m_Stream.get();
}

static bool isignore(int c)
{
	return 0x00 <= c && c <= 0x20;
}

std::shared_ptr<csaw::Token> csaw::Parser::next()
{
	int c = read();

	while (isignore(c)) {
		if (c == '\n')
			m_Line++;
		c = read();
	}

	if (c < 0)
		return m_Current = std::make_shared<Token>(m_Line);

	if (c == '#') {
		c = read();
		const char LIMIT = c == '#' ? '\n' : '#';
		while ((c = read()) != LIMIT && c >= 0)
			if (c == '\n')
				m_Line++;
		return next();
	}

	if (isalpha(c) || c == '_') {
		std::string value;
		do {
			value += (char)c;
			mark(1);
			c = read();
		} while (isalnum(c) || c == '_');
		reset();

		return m_Current = std::make_shared<Token>(TOKEN_IDENTIFIER, value, m_Line);
	}

	if (isdigit(c)) {
		std::string value;
		do {
			value += (char)c;
			mark(1);
			int p = c;
			c = read();
			if (((p == 'e' || p == 'E') && c == '-')) {
				value += (char)c;
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
				c = read();
				switch (c) {
				case 't':
					value += '\t';
					c = read();
					continue;
				case 'r':
					value += '\r';
					c = read();
					continue;
				case 'n':
					value += '\n';
					c = read();
					continue;
				case 'f':
					value += '\f';
					c = read();
					continue;
				case 'b':
					value += '\b';
					c = read();
					continue;
				}
			}

			value += ((char)c);
			c = read();
		}

		return m_Current = std::make_shared<Token>(TOKEN_STRING, value, m_Line);
	}

	if (c == '\'') {
		std::string value;
		c = read();
		while (c != '\'' && c >= 0) {
			if (c == '\\') {
				c = read();
				switch (c) {
				case 't':
					value += '\t';
					c = read();
					continue;
				case 'r':
					value += '\r';
					c = read();
					continue;
				case 'n':
					value += '\n';
					c = read();
					continue;
				case 'f':
					value += '\f';
					c = read();
					continue;
				case 'b':
					value += '\b';
					c = read();
					continue;
				}
			}

			value += ((char)c);
			c = read();
		}

		return m_Current = std::make_shared<Token>(TOKEN_CHAR, value, m_Line);
	}

	return m_Current = std::make_shared<Token>(TOKEN_OPERATOR, std::string{ (char)c }, m_Line);

}
