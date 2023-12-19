#pragma once

#include "iostream"

namespace csaw
{
	void Parse(std::istream& stream);

	enum TokenType
	{
		TOKEN_EOF = 0,
		TOKEN_IDENTIFIER,
		TOKEN_NUMBER,
		TOKEN_STRING,
		TOKEN_CHAR,
		TOKEN_OPERATOR,
	};

	struct Token
	{
		Token(TokenType type, const std::string& value, int line)
			: Type(type), Value(value), Line(line)
		{}

		Token(int line)
			: Token(TOKEN_EOF, "", line)
		{}

		std::ostream& operator<<(std::ostream& out) const;

		const TokenType Type;
		const std::string Value;
		const int Line;
	};

	class Parser
	{
	public:
		Parser(std::istream& stream)
			: m_Stream(stream)
		{}

	private:
		void mark(int limit);
		void reset();
		int read();
		std::shared_ptr<Token> next();

	private:
		std::istream& m_Stream;
		std::shared_ptr<Token> m_Current;
		int m_Line = 1;
	};
}
