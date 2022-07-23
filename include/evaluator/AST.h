#ifndef EVAL_AST_H_
#define EVAL_AST_H_

#include <JsonParser.hpp>

#include <vector>
#include <variant>
#include <memory>

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

    JsonNode toJson() const;
};
} // namespace eval

#endif