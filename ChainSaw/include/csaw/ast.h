#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace csaw
{
	struct Type;
	struct ArrayType;

	struct Stmt;
	struct Expr;

	struct AliasStmt;
	struct EnclosedStmt;
	struct ForStmt;
	struct FunctionStmt;
	struct IfStmt;
	struct IncStmt;
	struct RetStmt;
	struct ThingStmt;
	struct VariableStmt;
	struct WhileStmt;

	struct BinaryExpr;
	struct CallExpr;
	struct ChrExpr;
	struct IdExpr;
	struct IndexExpr;
	struct LambdaExpr;
	struct MemberExpr;
	struct NumExpr;
	struct SelectExpr;
	struct StrExpr;
	struct UnaryExpr;
	struct VarArgExpr;

	typedef std::shared_ptr<Type> TypePtr;
	typedef std::shared_ptr<ArrayType> ArrayTypePtr;

	typedef std::shared_ptr<Stmt> StmtPtr;
	typedef std::shared_ptr<Expr> ExprPtr;

	typedef std::shared_ptr<AliasStmt> AliasStmtPtr;
	typedef std::shared_ptr<EnclosedStmt> EnclosedStmtPtr;
	typedef std::shared_ptr<ForStmt> ForStmtPtr;
	typedef std::shared_ptr<FunctionStmt> FunctionStmtPtr;
	typedef std::shared_ptr<IfStmt> IfStmtPtr;
	typedef std::shared_ptr<IncStmt> IncStmtPtr;
	typedef std::shared_ptr<RetStmt> RetStmtPtr;
	typedef std::shared_ptr<ThingStmt> ThingStmtPtr;
	typedef std::shared_ptr<VariableStmt> VariableStmtPtr;
	typedef std::shared_ptr<WhileStmt> WhileStmtPtr;

	typedef std::shared_ptr<BinaryExpr> BinaryExprPtr;
	typedef std::shared_ptr<CallExpr> CallExprPtr;
	typedef std::shared_ptr<ChrExpr> ChrExprPtr;
	typedef std::shared_ptr<IdExpr> IdExprPtr;
	typedef std::shared_ptr<IndexExpr> IndexExprPtr;
	typedef std::shared_ptr<LambdaExpr> LambdaExprPtr;
	typedef std::shared_ptr<MemberExpr> MemberExprPtr;
	typedef std::shared_ptr<NumExpr> NumExprPtr;
	typedef std::shared_ptr<SelectExpr> SelectExprPtr;
	typedef std::shared_ptr<StrExpr> StrExprPtr;
	typedef std::shared_ptr<UnaryExpr> UnaryExprPtr;
	typedef std::shared_ptr<VarArgExpr> VarArgExprPtr;

	struct Type
	{
	public:
		virtual ~Type() {}

		static TypePtr GetAny();
		static TypePtr GetNum();
		static TypePtr GetStr();
		static TypePtr GetChr();
		static TypePtr GetLambda();

		static TypePtr Get(const std::string& name);

		static ArrayTypePtr Get(const std::string& name, const size_t size);
		static ArrayTypePtr Get(const TypePtr& type, const size_t size);

	protected:
		Type(const std::string& name)
			: Name(name) {}

	public:
		const std::string Name;
	};

	struct ArrayType : Type
	{
		ArrayType(const TypePtr& type, const size_t size)
			: Type(type->Name), Element(type), Size(size) {}

		const TypePtr Element;
		const size_t Size;
	};

	struct Parameter
	{
		Parameter(const std::string& name, const TypePtr& type)
			: Name(name), Type(type) {}

		const std::string Name;
		const TypePtr Type;
	};

	struct Stmt
	{
		virtual ~Stmt() {}
		virtual std::ostream& operator>>(std::ostream& out) const = 0;
	};

	struct Expr : Stmt
	{
	};

	struct AliasStmt : Stmt
	{
		AliasStmt(const std::string& alias, const TypePtr& origin)
			: Alias(alias), Origin(origin) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::string Alias;
		const TypePtr Origin;
	};

	struct EnclosedStmt : Stmt
	{
		EnclosedStmt(const std::vector<StmtPtr>& body)
			: Body(body) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::vector<StmtPtr> Body;
	};

	struct ForStmt : Stmt
	{
		ForStmt(const StmtPtr& begin, const ExprPtr& condition, const StmtPtr& loop, const StmtPtr& body)
			: Begin(begin), Condition(condition), Loop(loop), Body(body) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const StmtPtr Begin;
		const ExprPtr Condition;
		const StmtPtr Loop;
		const StmtPtr Body;
	};

	struct FunctionStmt : Stmt
	{
		FunctionStmt(const bool is_constructor, const std::string& name, const TypePtr& ret_type, const std::vector<Parameter>& parameters, const bool is_var_arg, const TypePtr& member_of, const EnclosedStmtPtr& body)
			: IsConstructor(is_constructor), Name(name), RetType(ret_type), Parameters(parameters), IsVarArg(is_var_arg), MemberOf(member_of), Body(body) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const bool IsConstructor;
		const std::string Name;
		const TypePtr RetType;
		const std::vector<Parameter> Parameters;
		const bool IsVarArg;
		const TypePtr MemberOf;
		const EnclosedStmtPtr Body;
	};

	struct IfStmt : Stmt
	{
		IfStmt(const ExprPtr& condition, const StmtPtr& then, const StmtPtr& else_)
			: Condition(condition), Then(then), Else(else_) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const ExprPtr Condition;
		const StmtPtr Then;
		const StmtPtr Else;
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
		RetStmt(const ExprPtr& value)
			: Value(value) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const ExprPtr Value;
	};

	struct ThingStmt : Stmt
	{
		ThingStmt(const std::string& name, const std::string& group, const std::vector<Parameter>& fields)
			: Name(name), Group(group), Fields(fields) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::string Name;
		const std::string Group;
		const std::vector<Parameter> Fields;
	};

	struct VariableStmt : Stmt
	{
		VariableStmt(const TypePtr& type, const std::string& name, const ExprPtr& value)
			: Type(type), Name(name), Value(value) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const TypePtr Type;
		const std::string Name;
		const ExprPtr Value;
	};

	struct WhileStmt : Stmt
	{
		WhileStmt(const ExprPtr& condition, const StmtPtr& body)
			: Condition(condition), Body(body) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const ExprPtr Condition;
		const StmtPtr Body;
	};

	struct BinaryExpr : Expr
	{
		BinaryExpr(const ExprPtr& left, const ExprPtr& right, const std::string& operator_)
			: Left(left), Operator(operator_), Right(right) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const ExprPtr Left;
		const std::string Operator;
		const ExprPtr Right;
	};

	struct CallExpr : Expr
	{
		CallExpr(const ExprPtr& function, const std::vector<ExprPtr>& arguments)
			: Function(function), Arguments(arguments) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const ExprPtr Function;
		const std::vector<ExprPtr> Arguments;
	};

	struct ChrExpr : Expr
	{
		ChrExpr(const char value)
			: Value(value) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const char Value;
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
		IndexExpr(const ExprPtr& object, const ExprPtr& index)
			: Object(object), Index(index) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const ExprPtr Object;
		const ExprPtr Index;
	};

	struct LambdaExpr : Expr
	{
		LambdaExpr(const std::vector<IdExprPtr>& passed, const std::vector<Parameter>& parameters, const StmtPtr& body)
			: Passed(passed), Parameters(parameters), Body(body) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::vector<IdExprPtr> Passed;
		const std::vector<Parameter> Parameters;
		const StmtPtr Body;
	};

	struct MemberExpr : Expr
	{
		MemberExpr(const ExprPtr& object, const std::string& member)
			: Object(object), Member(member) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const ExprPtr Object;
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

	struct SelectExpr : Expr
	{
		SelectExpr(const ExprPtr& condition, const ExprPtr& then, const ExprPtr& else_)
			: Condition(condition), True(then), False(else_) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const ExprPtr Condition;
		const ExprPtr True;
		const ExprPtr False;
	};

	struct StrExpr : Expr
	{
		StrExpr(const std::string& value)
			: Value(value) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::string Value;
	};

	struct UnaryExpr : Expr
	{
		UnaryExpr(const std::string& operator_, const ExprPtr& value)
			: Operator(operator_), Value(value) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const std::string Operator;
		const ExprPtr Value;
	};

	struct VarArgExpr : Expr
	{
		VarArgExpr() {}

		VarArgExpr(const TypePtr& type)
			: Type(type) {}

		std::ostream& operator>>(std::ostream& out) const override;

		const TypePtr Type;
	};

	std::ostream& operator<<(std::ostream& out, const Parameter& parameter);

	std::ostream& operator<<(std::ostream& out, const TypePtr& type);
	std::ostream& operator<<(std::ostream& out, const ArrayTypePtr& type);

	std::ostream& operator<<(std::ostream& out, const StmtPtr& stmt);
	std::ostream& operator<<(std::ostream& out, const AliasStmtPtr& stmt);
	std::ostream& operator<<(std::ostream& out, const EnclosedStmtPtr& stmt);
	std::ostream& operator<<(std::ostream& out, const ForStmtPtr& stmt);
	std::ostream& operator<<(std::ostream& out, const FunctionStmtPtr& stmt);
	std::ostream& operator<<(std::ostream& out, const IfStmtPtr& stmt);
	std::ostream& operator<<(std::ostream& out, const IncStmtPtr& stmt);
	std::ostream& operator<<(std::ostream& out, const RetStmtPtr& stmt);
	std::ostream& operator<<(std::ostream& out, const ThingStmtPtr& stmt);
	std::ostream& operator<<(std::ostream& out, const VariableStmtPtr& stmt);
	std::ostream& operator<<(std::ostream& out, const WhileStmtPtr& stmt);

	std::ostream& operator<<(std::ostream& out, const ExprPtr& expr);
}
