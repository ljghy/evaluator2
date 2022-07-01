#include <evaluator/Context.h>
#include <cmath>

#ifndef NDEBUG
#include <iostream>
#endif

namespace eval
{

void Context::init()
{
    m_globalVarMap.clear();
    setupInternalFunc();
}

DataType Context::exec(const std::string &input)
{
    Parser parser;
    auto ret = eval(parser.parse(tokenize(input)));
    if (ret.index() != 0)
        m_globalVarMap["ans"] = ret;
    return ret;
}

DataType Context::eval(std::shared_ptr<ASTNode> ast)
{
    assert(ast != nullptr);
    if (ast->isDecimal())
        return ast->getDecimal();
    if (ast->isIdent())
    {
        auto ite = m_globalVarMap.find(ast->getIdent());
        if (ite == m_globalVarMap.end())
            throw EvalExcept(EVAL_IDENTIFIER_UNDEFINED);
        return ite->second;
    }
    switch (ast->getOptr())
    {
    case OptrType::ASSIGN:
        m_globalVarMap[ast->children[0]->getIdent()] = eval(ast->children[1]);
        return VoidType{};
    case OptrType::ASSIGN_LAMBDA:
    {
        LambdaType lambda;
        lambda.params.reserve(ast->children[1]->children.size());
        for (auto &p : ast->children[1]->children)
            lambda.params.push_back(p->getIdent());
        lambda.expr = ast->children[2];
        m_globalVarMap[ast->children[0]->getIdent()] = lambda;
        return VoidType{};
    }
    case OptrType::NEG:
        return neg(eval(ast->children[0]));
    case OptrType::ADD:
    case OptrType::SUB:
    case OptrType::MUL:
    case OptrType::DIV:
    case OptrType::POW:
        return binOp(eval(ast->children[0]), eval(ast->children[1]), ast->getOptr());
    case OptrType::CALL:
    {
        auto lambda = eval(ast->children[0]);
        if (lambda.index() != 3)
            throw EvalExcept(EVAL_OBJECT_NOT_CALLABLE);
        auto &l = std::get<3>(lambda);
        if (l.isInternalFunc)
        {
            auto ret = l.internalFuncDef(ast->children[1]->children, *this);
            if (ret.type == InternalFuncRetType::DECIMAL)
                return ret.decimal;
            else if (ret.type == InternalFuncRetType::LIST)
                return ret.list;
            else
                return ret.lambda;
        }
        return call(l, ast->children[1]->children);
    }
    case OptrType::INDEX:
    {
        auto list = eval(ast->children[0]);
        if (list.index() != 2)
            throw EvalExcept(EVAL_OBJECT_NOT_LIST);

        auto idx = eval(ast->children[1]);
        if (idx.index() != 1)
            throw EvalExcept(EVAL_INDEX_NOT_DECIMAL);

        size_t i = static_cast<size_t>(std::round(std::get<1>(idx)));
        auto l = std::get<2>(list);
        if (i >= l.size())
            throw EvalExcept(EVAL_INDEX_OUT_OF_RANGE);
        return l[i];
    }
    case OptrType::LIST:
    {
        ListType list;
        list.reserve(ast->children.size());
        for (auto &c : ast->children)
        {
            auto val = eval(c);
            if (val.index() != 1)
                throw EvalExcept(EVAL_LIST_MEMBER_NOT_DECIMAL);
            list.push_back(std::get<1>(val));
        }
        return list;
    }
    case OptrType::LAMBDA:
    {
        LambdaType lambda;
        lambda.params.reserve(ast->children[0]->children.size());
        for (auto &p : ast->children[0]->children)
            lambda.params.push_back(p->getIdent());
        lambda.expr = ast->children[1];
        return lambda;
    }
    default:
        assert(0);
        return VoidType{};
    }
}

DataType Context::call(const LambdaType &lambda, const std::vector<std::shared_ptr<ASTNode>> &paramList)
{
    if (lambda.params.size() != paramList.size())
        throw EvalExcept(EVAL_WRONG_NUMBER_OF_PARAMETERS);

    VarMap local;
    for (size_t i = 0; i < lambda.params.size(); ++i)
        local[lambda.params[i]] = eval(paramList[i]);

    return eval(substitude(lambda.expr, local, {}));
}

std::shared_ptr<ASTNode> Context::substitude(const std::shared_ptr<ASTNode> &expr,
                                             const VarMap &varMap,
                                             std::unordered_set<std::string> masked)
{
    if (expr->isOptr() && expr->getOptr() == OptrType::LAMBDA)
        for (auto &c : expr->children[0]->children)
            masked.insert(c->getIdent());
    auto ret = std::make_shared<ASTNode>(*expr);
    for (auto &c : ret->children)
    {
        if (c->isIdent() && (masked.find(c->getIdent()) == masked.end()))
        {
            auto ite = varMap.find(c->getIdent());
            if (ite == varMap.end())
                continue;
            auto d = ite->second;
            assert(d.index() != 0);
            switch (d.index())
            {
            case 1:
                c = std::make_shared<ASTNode>(std::get<1>(d));
                break;
            case 2:
            {
                c = std::make_shared<ASTNode>(OptrType::LIST);
                auto &l = std::get<2>(d);
                c->alloc(l.size());
                for (size_t i = 0; i < l.size(); ++i)
                    c->children[i] = std::make_shared<ASTNode>(l[i]);
            }
            break;
            case 3:
            {
                c = std::make_shared<ASTNode>(OptrType::LAMBDA);
                auto &l = std::get<3>(d);
                c->alloc(2);
                c->children[0]->value = OptrType::PARAM_LIST;
                c->children[0]->alloc(l.params.size());
                for (size_t i = 0; i < l.params.size(); ++i)
                    c->children[0]->children[i]->value = l.params[i];
                c->children[1] = l.expr;
            }
            break;
            }
        }
        else
            c = substitude(c, varMap, masked);
    }
    return ret;
}

#include <evaluator/Operators.inl>
#include <evaluator/InternalFunc.inl>

} // namespace eval