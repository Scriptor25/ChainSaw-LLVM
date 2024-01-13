#pragma once

#include <istream>

namespace cir
{
	bool ParseFile(const std::string& filename);
	void ParseStream(std::istream& stream);

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
			: Type(type), Value(value), Line(line) {}
		Token(int line)
			: Token(TOKEN_EOF, "", line) {}

		const TokenType Type;
		const std::string Value;
		const int Line;
	};

	std::ostream& operator<<(std::ostream& out, const Token& token);
	std::ostream& operator<<(std::ostream& out, const std::shared_ptr<Token>& token);
	std::ostream& operator<<(std::ostream& out, const TokenType& type);

	class Parser
	{
	public:
		Parser(std::istream& stream)
			: m_Stream(stream) {}

	private:
		int read();
		void mark(int limit);
		void reset();

	public:
		std::shared_ptr<Token> Next();
		std::shared_ptr<Token> Current();

		bool AtEof() const;
		bool At(const std::string& value) const;
		bool At(const TokenType type) const;
		bool Expect(const std::string& value) const;
		bool Expect(const TokenType type) const;
		bool ExpectAndNext(const std::string& value);
		bool ExpectAndNext(const TokenType type);

	private:
		std::istream& m_Stream;
		int m_Limit = 0;
		int m_Index = 0;
		char* m_Peek = nullptr;

		std::shared_ptr<Token> m_Current;
		int m_Line = 1;
	};
}
