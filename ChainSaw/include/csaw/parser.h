#pragma once

#include "ast.h"

namespace csaw
{
	bool Parse(const std::string& filename);
	bool Parse(std::istream& stream);

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

	typedef std::shared_ptr<Token> TokenPtr;

	std::ostream& operator<<(std::ostream& out, const Token& token);
	std::ostream& operator<<(std::ostream& out, const TokenPtr& token);
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
		TokenPtr Next();
		TokenPtr Current();

		bool AtEof() const;
		bool At(const std::string& value) const;
		bool At(const TokenType type) const;
		bool Expect(const std::string& value) const;
		bool Expect(const TokenType type) const;
		bool ExpectAndNext(const std::string& value);
		bool ExpectAndNext(const TokenType type);

		TypePtr NextType();
		TypePtr NextType(const ExprPtr& type);
		Parameter NextParameter();

		StmtPtr NextStmt(bool end);
		EnclosedStmtPtr NextEnclosedStmt();
		AliasStmtPtr NextAliasStmt(bool end);
		ForStmtPtr NextForStmt(bool end);
		FunctionStmtPtr NextFunStmt();
		IfStmtPtr NextIfStmt();
		IncStmtPtr NextIncStmt(bool end);
		RetStmtPtr NextRetStmt(bool end);
		ThingStmtPtr NextThingStmt(bool end);
		WhileStmtPtr NextWhileStmt(bool end);
		VariableStmtPtr NextVarStmt(const ExprPtr& expr, bool end);

		ExprPtr NextExpr();
		ExprPtr NextConExpr();
		ExprPtr NextBinAndExpr();
		ExprPtr NextBinOrExpr();
		ExprPtr NextBinXOrExpr();
		ExprPtr NextBinCmpExpr();
		ExprPtr NextBinSumExpr();
		ExprPtr NextBinProExpr();
		ExprPtr NextCallExpr();
		ExprPtr NextIndexExpr();
		ExprPtr NextIndexExpr(ExprPtr expr);
		ExprPtr NextMemExpr();
		ExprPtr NextMemExpr(ExprPtr expr);
		ExprPtr NextPrimExpr();

	private:
		std::istream& m_Stream;
		int m_Limit = 0;
		int m_Index = 0;
		char* m_Peek = nullptr;

		TokenPtr m_Current;
		int m_Line = 1;
	};
}
