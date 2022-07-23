#include <evaluator/EvalDefs.h>
#include <evaluator/AST.h>

#include <cassert>

namespace eval
{
void ASTNode::alloc(size_t s)
{
    children.resize(s);
    for (auto &c : children)
        c = std::make_shared<ASTNode>();
}

OptrType ASTNode::getOptr() const
{
    assert(value.index() == 0);
    return std::get<0>(value);
}

bool ASTNode::isOptr() const
{
    return value.index() == 0;
}

bool ASTNode::isDecimal() const
{
    return value.index() == 1;
}

bool ASTNode::isIdent() const
{
    return value.index() == 2;
}

decimal_t ASTNode::getDecimal() const
{
    assert(value.index() == 1);
    return std::get<1>(value);
}

std::string ASTNode::getIdent() const
{
    assert(value.index() == 2);
    return std::get<2>(value);
}

JsonNode ASTNode::toJson() const
{
    if (value.index() == 1)
        return getDecimal();
    if (value.index() == 2)
        return getIdent();
    switch (getOptr())
    {
    case OptrType::ASSIGN:
        return {{"TYPE", jsptr("ASSIGN")},
                {"IDENT", jsptr(children[0]->getIdent())},
                {"EXPR", jsptr(children[1]->toJson())}};
    case OptrType::ASSIGN_LAMBDA:
        return {{"TYPE", jsptr("ASSIGN_LAMBDA")},
                {"IDENT", jsptr(children[0]->getIdent())},
                {"PARAM_LIST", jsptr(children[1]->toJson())},
                {"EXPR", jsptr(children[2]->toJson())}};
    case OptrType::NEG:
        return {{"TYPE", jsptr("NEG")},
                {"EXPR", jsptr(children[0]->toJson())}};
    case OptrType::ADD:
        return {{"TYPE", jsptr("ADD")},
                {"LHS", jsptr(children[0]->toJson())},
                {"RHS", jsptr(children[1]->toJson())}};
    case OptrType::SUB:
        return {{"TYPE", jsptr("SUB")},
                {"LHS", jsptr(children[0]->toJson())},
                {"RHS", jsptr(children[1]->toJson())}};
    case OptrType::MUL:
        return {{"TYPE", jsptr("MUL")},
                {"LHS", jsptr(children[0]->toJson())},
                {"RHS", jsptr(children[1]->toJson())}};
    case OptrType::DIV:
        return {{"TYPE", jsptr("DIV")},
                {"LHS", jsptr(children[0]->toJson())},
                {"RHS", jsptr(children[1]->toJson())}};
    case OptrType::POW:
        return {{"TYPE", jsptr("POW")},
                {"LHS", jsptr(children[0]->toJson())},
                {"RHS", jsptr(children[1]->toJson())}};
    case OptrType::CALL:
        return {{"TYPE", jsptr("CALL")},
                {"BY", jsptr(children[0]->toJson())},
                {"PARAMS", jsptr(children[1]->toJson())}};
    case OptrType::INDEX:
        return {{"TYPE", jsptr("INDEX")},
                {"BY", jsptr(children[0]->toJson())},
                {"INDEX", jsptr(children[1]->toJson())}};
    case OptrType::LAMBDA:
        return {{"TYPE", jsptr("LAMBDA")},
                {"PARAM_LIST", jsptr(children[0]->toJson())},
                {"RETURN", jsptr(children[1]->toJson())}};
    case OptrType::LIST:
    {
        JsonArr_t arr;
        for (auto &c : children)
            arr.push_back(jsptr(c->toJson()));
        return {{"TYPE", jsptr("LIST")},
                {"LIST", jsptr(arr)}};
    }
    case OptrType::EXPR_LIST:
    {
        JsonArr_t arr;
        for (auto &c : children)
            arr.push_back(jsptr(c->toJson()));
        return {{"TYPE", jsptr("EXPR_LIST")},
                {"LIST", jsptr(arr)}};
    }
    case OptrType::PARAM_LIST:
    {
        JsonArr_t arr;
        for (auto &c : children)
            arr.push_back(jsptr(c->getIdent()));
        return {{"TYPE", jsptr("PARAM_LIST")},
                {"PARAMS", jsptr(arr)}};
    }
    default:
        return JsonNull;
    }
}

} // namespace eval