#ifndef EVAL_PARSER_H_
#define EVAL_PARSER_H_

#include <evaluator/Tokenizer.h>
#include <evaluator/AST.h>

namespace eval
{

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
