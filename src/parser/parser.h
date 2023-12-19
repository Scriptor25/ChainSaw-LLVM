#pragma once

#include <iostream>
#include <vector>

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

		const TokenType Type;
		const std::string Value;
		const int Line;
	};

	std::ostream& operator<<(std::ostream& out, const Token& token);

	class Parser
	{
	public:
		Parser(std::istream& stream)
			: m_Stream(stream)
		{}

	private:
		int read();
		void mark(int limit);
		void reset();

	public:
		std::shared_ptr<Token> Next();
		std::shared_ptr<Token> Current();

	private:
		std::istream& m_Stream;
		int m_Limit = 0;
		int m_Index = 0;
		char* m_Peek = nullptr;

		std::shared_ptr<Token> m_Current;
		int m_Line = 1;
	};
}
