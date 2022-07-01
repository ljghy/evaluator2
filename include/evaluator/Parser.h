#ifndef EVAL_PARSER_H_
#define EVAL_PARSER_H_

#include <evaluator/EvalDefs.h>
#include <evaluator/Tokenizer.h>

#include <variant>
#include <memory>

#ifndef NDEBUG
#include <JsonParser.hpp>
#endif

namespace eval
{

enum class OptrType
{
    ASSIGN,
    ASSIGN_LAMBDA,
    NEG,
    ADD,
    SUB,
    MUL,
    DIV,
    POW,
    CALL,
    INDEX,
    LIST,
    LAMBDA,
    EXPR_LIST,
    PARAM_LIST
};

struct ASTNode
{
    std::variant<OptrType, decimal_t, std::string> value;
    std::vector<std::shared_ptr<ASTNode>> children;

    ASTNode() = default;
    template <typename ValType>
    ASTNode(const ValType &v) : value(v) {}

    ASTNode(const ASTNode &) = default;

    void alloc(size_t s);

    bool isOptr() const;
    bool isDecimal() const;
    bool isIdent() const;
    OptrType getOptr() const;
    decimal_t getDecimal() const;
    std::string getIdent() const;

#ifndef NDEBUG
    JsonNode toJson() const;
#endif
};

class Parser
{
public:
    std::shared_ptr<ASTNode> parse(const TokenList &);

private:
    bool parseAssign(std::shared_ptr<ASTNode> &);
    bool parseExpr(std::shared_ptr<ASTNode> &);

    bool parseTerm(std::shared_ptr<ASTNode> &);
    bool parseList(std::shared_ptr<ASTNode> &);
    bool parseLambda(std::shared_ptr<ASTNode> &);
    bool parseParamList(std::shared_ptr<ASTNode> &);
    bool parseExprList(std::shared_ptr<ASTNode> &);

    bool parseExprUnary(std::shared_ptr<ASTNode> &);
    bool parseExprL1(std::shared_ptr<ASTNode> &);
    bool parseExprL2(std::shared_ptr<ASTNode> &);
    bool parseExprL3(std::shared_ptr<ASTNode> &);

private:
    TokenList::const_iterator m_pos;
    TokenList::const_iterator m_end;
};

} // namespace eval

#endif
