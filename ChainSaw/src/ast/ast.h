#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace csaw
{
	struct ASTArrayType;

	struct ASTType
	{
	public:
		virtual ~ASTType() {}

		static std::shared_ptr<ASTType> GetAny();
		static std::shared_ptr<ASTType> GetNum();
		static std::shared_ptr<ASTType> GetStr();
		static std::shared_ptr<ASTType> GetChr();
		static std::shared_ptr<ASTType> GetLambda();

		static std::shared_ptr<ASTType> Get(const std::string& name);
		static std::shared_ptr<ASTArrayType> Get(const std::string& name, const size_t size);
		static std::shared_ptr<ASTArrayType> Get(const std::shared_ptr<ASTType>& type, const size_t size);

	protected:
		ASTType(const std::string& name)
			: Name(name) {}

	public:
		const std::string Name;
	};

	struct ASTArrayType : ASTType
	{
		ASTArrayType(const std::shared_ptr<ASTType>& type, const size_t size)
			: ASTType(type->Name), Type(type), Size(size) {}

		const std::shared_ptr<ASTType> Type;
		const size_t Size;
	};

	struct ASTParameter
	{
		ASTParameter(const std::string& name, const std::shared_ptr<ASTType>& type)
			: Name(name), Type(type) {}

		const std::string Name;
		const std::shared_ptr<ASTType> Type;
	};

	struct Stmt
	{
		virtual ~Stmt() {}

		virtual std::ostream& operator>>(std::ostream& out) const = 0;
	};

	struct Expr : Stmt
	{
		virtual ~Expr() {}

		virtual std::ostream& operator>>(std::ostream& out) const = 0;
	};

	struct AliasStmt : Stmt
	{
		AliasStmt(const std::string& alias, const std::shared_ptr<ASTType>& origin)
			: Alias(alias), Origin(origin) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::string Alias;
		const std::shared_ptr<ASTType> Origin;
	};

	struct EnclosedStmt : Stmt
	{
		EnclosedStmt(const std::vector<std::shared_ptr<Stmt>>& body)
			: Body(body) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::vector<std::shared_ptr<Stmt>> Body;
	};

	struct ForStmt : Stmt
	{
		ForStmt(const std::shared_ptr<Stmt>& begin, const std::shared_ptr<Expr>& condition, const std::shared_ptr<Stmt>& loop, const std::shared_ptr<Stmt>& body)
			: Begin(begin), Condition(condition), Loop(loop), Body(body) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::shared_ptr<Stmt> Begin;
		const std::shared_ptr<Expr> Condition;
		const std::shared_ptr<Stmt> Loop;
		const std::shared_ptr<Stmt> Body;
	};

	struct FunStmt : Stmt
	{
		FunStmt(const bool is_constructor, const std::string& name, const std::shared_ptr<ASTType>& ret_type, const std::vector<ASTParameter>& parameters, const bool is_var_arg, const std::shared_ptr<ASTType>& member_of, const std::shared_ptr<EnclosedStmt>& body)
			: IsConstructor(is_constructor), Name(name), RetType(ret_type), Parameters(parameters), IsVarArg(is_var_arg), MemberOf(member_of), Body(body) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const bool IsConstructor;
		const std::string Name;
		const std::shared_ptr<ASTType> RetType;
		const std::vector<ASTParameter> Parameters;
		const bool IsVarArg;
		const std::shared_ptr<ASTType> MemberOf;
		const std::shared_ptr<EnclosedStmt> Body;
	};

	struct IfStmt : Stmt
	{
		IfStmt(const std::shared_ptr<Expr>& condition, const std::shared_ptr<Stmt>& then, const std::shared_ptr<Stmt>& else_)
			: Condition(condition), Then(then), Else(else_) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::shared_ptr<Expr> Condition;
		const std::shared_ptr<Stmt> Then;
		const std::shared_ptr<Stmt> Else;
	};

	struct IncStmt : Stmt
	{
		IncStmt(const std::string& path)
			: Path(path) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::string Path;
	};

	struct RetStmt : Stmt
	{
		RetStmt(const std::shared_ptr<Expr >& value)
			: Value(value) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::shared_ptr<Expr> Value;
	};

	struct ThingStmt : Stmt
	{
		ThingStmt(const std::string& name, const std::string& group, const std::vector<ASTParameter>& fields)
			: Name(name), Group(group), Fields(fields) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::string Name;
		const std::string Group;
		const std::vector<ASTParameter> Fields;
	};

	struct VarStmt : Stmt
	{
		VarStmt(const std::shared_ptr<ASTType>& type, const std::string& name, const std::shared_ptr<Expr>& value)
			: Type(type), Name(name), Value(value) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::shared_ptr<ASTType> Type;
		const std::string Name;
		const std::shared_ptr<Expr> Value;
	};

	struct WhileStmt : Stmt
	{
		WhileStmt(const std::shared_ptr<Expr >& condition, const std::shared_ptr<Stmt>& body)
			: Condition(condition), Body(body) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::shared_ptr<Expr> Condition;
		const std::shared_ptr<Stmt> Body;
	};

	/*struct AssignExpr : Expr
	{
		AssignExpr(const std::shared_ptr<Expr>& object, const std::shared_ptr<Expr>& value)
			: Object(object), Value(value) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::shared_ptr<Expr> Object;
		const std::shared_ptr<Expr> Value;
	};*/

	struct BinExpr : Expr
	{
		BinExpr(const std::shared_ptr<Expr>& left, const std::shared_ptr<Expr>& right, const std::string& operator_)
			: Left(left), Operator(operator_), Right(right) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::shared_ptr<Expr> Left;
		const std::string Operator;
		const std::shared_ptr<Expr> Right;
	};

	struct CallExpr : Expr
	{
		CallExpr(const std::shared_ptr<Expr>& function, const std::vector<std::shared_ptr<Expr>>& arguments)
			: Function(function), Arguments(arguments) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::shared_ptr<Expr> Function;
		const std::vector<std::shared_ptr<Expr>> Arguments;
	};

	struct ChrExpr : Expr
	{
		ChrExpr(const char value)
			: Value(value) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const char Value;
	};

	struct ConExpr : Expr
	{
		ConExpr(const std::shared_ptr<Expr>& condition, const std::shared_ptr<Expr>& then, const std::shared_ptr<Expr>& else_)
			: Condition(condition), Then(then), Else(else_) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::shared_ptr<Expr> Condition;
		const std::shared_ptr<Expr> Then;
		const std::shared_ptr<Expr> Else;
	};

	struct IdExpr : Expr
	{
		IdExpr(const std::string& value)
			: Value(value) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::string Value;
	};

	struct IndexExpr : Expr
	{
		IndexExpr(const std::shared_ptr<Expr>& object, const std::shared_ptr<Expr>& index)
			: Object(object), Index(index) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::shared_ptr<Expr> Object;
		const std::shared_ptr<Expr> Index;
	};

	struct LambdaExpr : Expr
	{
		LambdaExpr(const std::vector<std::shared_ptr<IdExpr>>& passed, const std::vector<ASTParameter>& parameters, const std::shared_ptr<Stmt>& body)
			: Passed(passed), Parameters(parameters), Body(body) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::vector<std::shared_ptr<IdExpr>> Passed;
		const std::vector<ASTParameter> Parameters;
		const std::shared_ptr<Stmt> Body;
	};

	struct MemExpr : Expr
	{
		MemExpr(const std::shared_ptr<Expr>& object, const std::string& member)
			: Object(object), Member(member) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::shared_ptr<Expr> Object;
		const std::string Member;
	};

	struct NumExpr : Expr
	{
		NumExpr(const double value)
			: Value(value) {}

		NumExpr(const std::string& value)
			: Value(std::stod(value)) {}

		NumExpr(const std::string& value, int radix)
			: Value(std::stol(value, nullptr, radix)) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const double Value;
	};

	struct StrExpr : Expr
	{
		StrExpr(const std::string& value)
			: Value(value) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::string Value;
	};

	struct UnExpr : Expr
	{
		UnExpr(const std::string& operator_, const std::shared_ptr<Expr>& value)
			: Operator(operator_), Value(value) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::string Operator;
		const std::shared_ptr<Expr> Value;
	};

	std::ostream& operator<<(std::ostream& out, const ASTParameter& parameter);
	std::ostream& operator<<(std::ostream& out, const std::shared_ptr<ASTType>& type);
	std::ostream& operator<<(std::ostream& out, const std::shared_ptr<ASTArrayType>& type);

	std::ostream& operator<<(std::ostream& out, const std::shared_ptr<Stmt>& stmt);
	std::ostream& operator<<(std::ostream& out, const std::shared_ptr<Expr>& expr);

}
