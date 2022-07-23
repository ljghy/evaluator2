#include <evaluator/Parser.h>
#include <stdexcept>

namespace eval
{

#define CHECK_END           \
    do                      \
    {                       \
        if (m_pos == m_end) \
            return false;   \
    } while (0)

std::shared_ptr<ASTNode> Parser::parse(const TokenList &tkl)
{
    std::shared_ptr<ASTNode> ast = std::make_shared<ASTNode>();

    m_pos = tkl.begin();
    m_end = tkl.end();

    auto p0 = m_pos;
    if (parseAssign(ast) && m_pos == m_end)
        return ast;

    m_pos = p0;
    if (parseExpr(ast) && m_pos == m_end)
        return ast;

    throw EvalExcept(EVAL_PARSE_FAILED);
    return nullptr;
}

bool Parser::parseAssign(std::shared_ptr<ASTNode> &ast)
{
    ast->alloc(2);
    if (m_pos != m_end && m_pos->type == TokenType::IDENT)
    {
        ast->value = OptrType::ASSIGN;
        ast->children[0]->value = m_pos->getIdent();
        ++m_pos;
    }
    else
        return false;

    CHECK_END;
    if (m_pos->type == TokenType::LPAR)
    {
        ++m_pos;
        ast->value = OptrType::ASSIGN_LAMBDA;
        if (parseParamList(ast->children[1]) && m_pos != m_end && m_pos->type == TokenType::RPAR)
            ++m_pos;
        else
            return false;
        ast->children.push_back(std::make_shared<ASTNode>());
    }
    if (m_pos != m_end && m_pos->type == TokenType::ASSIGN)
        ++m_pos;
    else
        return false;

    return parseExpr(ast->children.back());
}

bool Parser::parseExpr(std::shared_ptr<ASTNode> &ast)
{
    return parseExprL1(ast);
}

bool Parser::parseExprUnary(std::shared_ptr<ASTNode> &ast)
{
    CHECK_END;
    if (m_pos->type == TokenType::SUB)
    {
        ast->alloc(1);
        ast->value = OptrType::NEG;
        ++m_pos;
        return parseExprL2(ast->children[0]);
    }
    return parseExprL2(ast);
}

bool Parser::parseExprL1(std::shared_ptr<ASTNode> &ast)
{
    if (!parseExprUnary(ast))
        return false;
    while (m_pos != m_end && (m_pos->type == TokenType::ADD || m_pos->type == TokenType::SUB))
    {
        auto cpy = std::make_shared<ASTNode>(*ast);
        ast->alloc(2);
        ast->children[0] = cpy;
        ast->value = m_pos->type == TokenType::ADD ? OptrType::ADD : OptrType::SUB;
        ++m_pos;
        if (!parseExprUnary(ast->children[1]))
            return false;
    }
    return true;
}

bool Parser::parseExprL2(std::shared_ptr<ASTNode> &ast)
{
    if (!parseExprL3(ast))
        return false;
    while (m_pos != m_end && (m_pos->type == TokenType::MUL || m_pos->type == TokenType::DIV))
    {
        auto cpy = std::make_shared<ASTNode>(*ast);
        ast->alloc(2);
        ast->children[0] = cpy;
        ast->value = m_pos->type == TokenType::MUL ? OptrType::MUL : OptrType::DIV;
        ++m_pos;
        if (!parseExprL3(ast->children[1]))
            return false;
    }
    return true;
}

bool Parser::parseExprL3(std::shared_ptr<ASTNode> &ast)
{
    auto node = ast;
    while (parseTerm(node))
    {
        if (m_pos == m_end || m_pos->type != TokenType::POW)
            return true;
        ++m_pos;
        auto cpy = std::make_shared<ASTNode>(*node);
        node->alloc(2);
        node->children[0] = cpy;
        node->value = OptrType::POW;
        node = node->children[1];
    }
    return false;
}

bool Parser::parseTerm(std::shared_ptr<ASTNode> &ast)
{
    CHECK_END;

    auto p0 = m_pos;
    if (m_pos->type == TokenType::DECIMAL)
    {
        ast->value = m_pos->getDecimal();
        ++m_pos;
        return true;
    }

    if (m_pos->type == TokenType::IDENT)
    {
        ast->value = m_pos->getIdent();
        ++m_pos;
    }
    else if (m_pos->type == TokenType::LPAR)
    {
        ++m_pos;
        if (!parseExpr(ast))
            return false;
        if (m_pos == m_end || m_pos->type != TokenType::RPAR)
            return false;
        ++m_pos;
    }
    else if (m_pos->type == TokenType::LSQR)
    {
        if (!parseList(ast))
            return false;
    }
    else if (m_pos->type == TokenType::LAMBDA)
    {
        if (!parseLambda(ast))
            return false;
    }
    else
        return false;

    while (m_pos != m_end)
    {
        if (m_pos->type == TokenType::LPAR)
        {
            ++m_pos;
            auto cpy = std::make_shared<ASTNode>(*ast);
            ast->value = OptrType::CALL;
            ast->alloc(2);
            ast->children[0] = cpy;
            if (!parseExprList(ast->children[1]))
                return false;
            if (m_pos == m_end || m_pos->type != TokenType::RPAR)
                return false;
            ++m_pos;
        }
        else if (m_pos->type == TokenType::LSQR)
        {
            ++m_pos;
            auto cpy = std::make_shared<ASTNode>(*ast);
            ast->value = OptrType::INDEX;
            ast->alloc(2);
            ast->children[0] = cpy;
            if (!parseExpr(ast->children[1]))
                return false;
            if (m_pos == m_end || m_pos->type != TokenType::RSQR)
                return false;
            ++m_pos;
            return true;
        }
        else
            return true;
    }
    return true;
}

bool Parser::parseList(std::shared_ptr<ASTNode> &ast)
{
    if (m_pos == m_end || m_pos->type != TokenType::LSQR)
        return false;
    ++m_pos;
    CHECK_END;
    if (parseExprList(ast) && m_pos->type == TokenType::RSQR)
    {
        ++m_pos;
        ast->value = OptrType::LIST;
        return true;
    }
    else
        return false;
}

bool Parser::parseParamList(std::shared_ptr<ASTNode> &ast)
{
    ast->value = OptrType::PARAM_LIST;
    ast->children.clear();
    while (m_pos != m_end && m_pos->type == TokenType::IDENT)
    {
        ast->children.push_back(std::make_shared<ASTNode>(m_pos->getIdent()));
        ++m_pos;
        if (m_pos == m_end || m_pos->type != TokenType::COMMA)
            return true;
        ++m_pos;
    }
    return true;
}

bool Parser::parseExprList(std::shared_ptr<ASTNode> &ast)
{
    ast->value = OptrType::EXPR_LIST;
    ast->children.clear();
    auto tmp = std::make_shared<ASTNode>();
    while (parseExpr(tmp))
    {
        ast->children.push_back(tmp);
        if (m_pos == m_end || m_pos->type != TokenType::COMMA)
            return true;
        ++m_pos;
        tmp = std::make_shared<ASTNode>();
    }
    return true;
}

bool Parser::parseLambda(std::shared_ptr<ASTNode> &ast)
{
    if (m_pos == m_end || m_pos->type != TokenType::LAMBDA)
        return false;
    ++m_pos;
    if (m_pos == m_end || m_pos->type != TokenType::LPAR)
        return false;
    ++m_pos;
    ast->value = OptrType::LAMBDA;
    ast->alloc(2);
    parseParamList(ast->children[0]);
    if (m_pos == m_end || m_pos->type != TokenType::RPAR)
        return false;
    ++m_pos;
    if (m_pos == m_end || m_pos->type != TokenType::LCUR)
        return false;
    ++m_pos;
    if (!parseExpr(ast->children[1]))
        return false;
    if (m_pos == m_end || m_pos->type != TokenType::RCUR)
        return false;
    ++m_pos;
    return true;
}

} // namespace eval