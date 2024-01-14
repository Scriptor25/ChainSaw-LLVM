#pragma once

#include <csawir/csawir.h>
#include <istream>

namespace csawir
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
		TOKEN_REGISTER,
		TOKEN_BLOCK,
		TOKEN_GLOBAL,
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
	std::ostream& operator<<(std::ostream& out, const TokenType& type);

	class Parser
	{
	public:
		Parser(std::istream& stream)
			: m_Stream(stream) {}

		Context& GetContext() { return m_Context; }
		const Context& GetContext() const { return m_Context; }

		std::vector<Token>& NextLine();
		void Next();
		const Token& Current() const;

		bool AtEol() const;
		bool AtEof() const;
		bool At(const std::string& value) const;
		bool At(const TokenType type) const;
		const Token& Expect(const std::string& value) const;
		const Token& Expect(const TokenType type) const;
		const Token& ExpectAndNext(const std::string& value);
		const Token& ExpectAndNext(const TokenType type);

		void NextFunction();
		void NextGlobal();
		Inst* NextHighLevel();
		Inst* NextInst();
		Value* NextValue();
		Const* NextConst();

	private:
		std::istream& m_Stream;
		std::vector<Token> m_Current;
		size_t m_Index = 0;
		int m_Line = 0;

		Context m_Context;
	};
}
