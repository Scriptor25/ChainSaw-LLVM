#pragma once

#include "ast.h"

namespace csaw
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

		std::shared_ptr<ASTType> NextType();
		std::shared_ptr<ASTType> NextType(const std::shared_ptr<Expr>& type);
		ASTParameter NextParameter();

		std::shared_ptr<Stmt> NextStmt(bool end);
		std::shared_ptr<EnclosedStmt> NextEnclosedStmt();
		std::shared_ptr<AliasStmt> NextAliasStmt(bool end);
		std::shared_ptr<ForStmt> NextForStmt(bool end);
		std::shared_ptr<FunStmt> NextFunStmt();
		std::shared_ptr<IfStmt> NextIfStmt();
		std::shared_ptr<IncStmt> NextIncStmt(bool end);
		std::shared_ptr<RetStmt> NextRetStmt(bool end);
		std::shared_ptr<ThingStmt> NextThingStmt(bool end);
		std::shared_ptr<WhileStmt> NextWhileStmt(bool end);
		std::shared_ptr<VarStmt> NextVarStmt(const std::shared_ptr<Expr>& expr, bool end);

		std::shared_ptr<Expr> NextExpr();
		std::shared_ptr<Expr> NextConExpr();
		std::shared_ptr<Expr> NextBinAndExpr();
		std::shared_ptr<Expr> NextBinOrExpr();
		std::shared_ptr<Expr> NextBinXOrExpr();
		std::shared_ptr<Expr> NextBinCmpExpr();
		std::shared_ptr<Expr> NextBinSumExpr();
		std::shared_ptr<Expr> NextBinProExpr();
		std::shared_ptr<Expr> NextCallExpr();
		std::shared_ptr<Expr> NextIndexExpr();
		std::shared_ptr<Expr> NextIndexExpr(std::shared_ptr<Expr> expr);
		std::shared_ptr<Expr> NextMemExpr();
		std::shared_ptr<Expr> NextMemExpr(std::shared_ptr<Expr> expr);
		std::shared_ptr<Expr> NextPrimExpr();

	private:
		std::istream& m_Stream;
		int m_Limit = 0;
		int m_Index = 0;
		char* m_Peek = nullptr;

		std::shared_ptr<Token> m_Current;
		int m_Line = 1;
	};
}
