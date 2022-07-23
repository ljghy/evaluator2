#include <evaluator/Context.h>
#include <evaluator/InternalFunc.h>

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
    m_AST = parser.parse(tokenize(input));
    auto ret = eval(m_AST);
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
    if (expr->isIdent() && (masked.find(expr->getIdent()) == masked.end()))
    {
        auto ite = varMap.find(expr->getIdent());
        if (ite == varMap.end())
            return std::make_shared<ASTNode>(*expr);
        auto d = ite->second;
        assert(d.index() != 0);
        switch (d.index())
        {
        case 1:
            return std::make_shared<ASTNode>(std::get<1>(d));
        case 2:
        {
            auto ret = std::make_shared<ASTNode>(OptrType::LIST);
            auto &l = std::get<2>(d);
            ret->alloc(l.size());
            for (size_t i = 0; i < l.size(); ++i)
                ret->children[i] = std::make_shared<ASTNode>(l[i]);
            return ret;
        }
        case 3:
        {
            auto &l = std::get<3>(d);
            if (l.isInternalFunc)
                return std::make_shared<ASTNode>(l.internalFuncName);
            auto ret = std::make_shared<ASTNode>(OptrType::LAMBDA);
            ret->alloc(2);
            ret->children[0]->value = OptrType::PARAM_LIST;
            ret->children[0]->alloc(l.params.size());
            for (size_t i = 0; i < l.params.size(); ++i)
                ret->children[0]->children[i]->value = l.params[i];
            ret->children[1] = l.expr;
            return ret;
        }
        }
        assert(0);
    }

    auto ret = std::make_shared<ASTNode>(*expr);
    for (auto &c : ret->children)
        c = substitude(c, varMap, masked);
    return ret;
}

#include <evaluator/Operators.inl>

#define PUSH_UNARY_FUNC(f)               \
    do                                   \
    {                                    \
        m_globalVarMap[#f] = LambdaType{ \
            {"x"},                       \
            nullptr,                     \
            true,                        \
            internal_##f,                \
            #f};                         \
    } while (0)

#define PUSH_BINARY_FUNC(f)              \
    do                                   \
    {                                    \
        m_globalVarMap[#f] = LambdaType{ \
            {"x", "y"},                  \
            nullptr,                     \
            true,                        \
            internal_##f,                \
            #f};                         \
    } while (0)

void Context::setupInternalFunc()
{
    m_globalVarMap["e"] = std::exp(1);
    m_globalVarMap["pi"] = std::acos(-1);
    m_globalVarMap["ans"] = decimal_t(0);

    PUSH_UNARY_FUNC(sin);
    PUSH_UNARY_FUNC(cos);
    PUSH_UNARY_FUNC(tan);

    PUSH_UNARY_FUNC(asin);
    PUSH_UNARY_FUNC(acos);
    PUSH_UNARY_FUNC(atan);

    PUSH_UNARY_FUNC(exp);
    PUSH_UNARY_FUNC(ln);

    PUSH_UNARY_FUNC(abs);

    PUSH_UNARY_FUNC(floor);
    PUSH_UNARY_FUNC(ceil);
    PUSH_UNARY_FUNC(round);

    PUSH_UNARY_FUNC(sqrt);
    PUSH_UNARY_FUNC(erf);
    PUSH_UNARY_FUNC(gamma);

    PUSH_UNARY_FUNC(not );

    PUSH_BINARY_FUNC(eq);
    PUSH_BINARY_FUNC(neq);
    PUSH_BINARY_FUNC(gt);
    PUSH_BINARY_FUNC(lt);
    PUSH_BINARY_FUNC(geq);
    PUSH_BINARY_FUNC(leq);

    m_globalVarMap["and"] = LambdaType{
        {"x", "y"},
        nullptr,
        true,
        [](const std::vector<std::shared_ptr<ASTNode>> &params, Context &context) -> InternalFuncRet
        {
            if (params.size() != 2)
                throw EvalExcept(EVAL_WRONG_NUMBER_OF_PARAMETERS);
            auto x = context.eval(params[0]);
            if (x.index() != 1)
                throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);
            if (!std::get<1>(x))
                return decimal_t(0);
            auto y = context.eval(params[1]);
            if (y.index() != 1)
                throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);
            return decimal_t(static_cast<bool>(std::get<1>(y)));
        },
        "and"};

    m_globalVarMap["or"] = LambdaType{
        {"x", "y"},
        nullptr,
        true,
        [](const std::vector<std::shared_ptr<ASTNode>> &params, Context &context) -> InternalFuncRet
        {
            if (params.size() != 2)
                throw EvalExcept(EVAL_WRONG_NUMBER_OF_PARAMETERS);
            auto x = context.eval(params[0]);
            if (x.index() != 1)
                throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);
            if (std::get<1>(x))
                return decimal_t(1);
            auto y = context.eval(params[1]);
            if (y.index() != 1)
                throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);
            return decimal_t(static_cast<bool>(std::get<1>(y)));
        },
        "or"};

    m_globalVarMap["if_else"] = LambdaType{
        {"cond", "true", "false"},
        nullptr,
        true,
        [](const std::vector<std::shared_ptr<ASTNode>> &params, Context &context) -> InternalFuncRet
        {
            if (params.size() != 3)
                throw EvalExcept(EVAL_WRONG_NUMBER_OF_PARAMETERS);
            auto cond = context.eval(params[0]);
            if (cond.index() != 1)
                throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);

            auto ret = context.eval(params[std::get<1>(cond) != decimal_t(0) ? 1 : 2]);

            switch (ret.index())
            {
            case 1:
                return std::get<1>(ret);
            case 2:
                return std::get<2>(ret);
            case 3:
                return std::get<3>(ret);
            default:
                assert(0);
                return decimal_t(0);
            }
        },
        "if_else"};

    m_globalVarMap["len"] = LambdaType{
        {"list"},
        nullptr,
        true,
        [](const std::vector<std::shared_ptr<ASTNode>> &params, Context &context) -> InternalFuncRet
        {
            if (params.size() != 1)
                throw EvalExcept(EVAL_WRONG_NUMBER_OF_PARAMETERS);
            auto l = context.eval(params[0]);
            if (l.index() != 2)
                throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);
            return static_cast<decimal_t>(std::get<2>(l).size());
        },
        "len"};
    m_globalVarMap["assign"] = LambdaType{
        {"list", "idx", "val"},
        nullptr,
        true,
        [](const std::vector<std::shared_ptr<ASTNode>> &params, Context &context) -> InternalFuncRet
        {
            if (params.size() != 3)
                throw EvalExcept(EVAL_WRONG_NUMBER_OF_PARAMETERS);
            auto list = context.eval(params[0]);
            if (list.index() != 2)
                throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);

            auto ret = std::get<2>(list);
            auto idx = context.eval(params[1]);
            if (idx.index() != 1)
                throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);
            size_t i = static_cast<size_t>(std::round(std::get<1>(idx)));
            if (i >= ret.size())
                throw EvalExcept(EVAL_INDEX_OUT_OF_RANGE);
            auto val = context.eval(params[2]);
            if (val.index() != 1)
                throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);
            ret[i] = std::get<1>(val);
            return ret;
        },
        "assign"};
    m_globalVarMap["append"] = LambdaType{
        {"list", "val"},
        nullptr,
        true,
        [](const std::vector<std::shared_ptr<ASTNode>> &params, Context &context) -> InternalFuncRet
        {
            if (params.size() != 2)
                throw EvalExcept(EVAL_WRONG_NUMBER_OF_PARAMETERS);
            auto list = context.eval(params[0]);
            if (list.index() != 2)
                throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);

            auto v1 = std::get<2>(list);

            auto v2 = context.eval(params[1]);
            if (v2.index() == 1)
            {
                v1.push_back(std::get<1>(v2));
                return v1;
            }
            if (v2.index() == 2)
            {
                auto v = std::get<2>(v2);
                v1.reserve(v1.size() + v.size());
                v1.insert(v1.end(), v.begin(), v.end());
                return v1;
            }

            throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);
            return v1;
        },
        "append"};
    m_globalVarMap["slice"] = LambdaType{
        {"list", "st", "ed"},
        nullptr,
        true,
        [](const std::vector<std::shared_ptr<ASTNode>> &params, Context &context) -> InternalFuncRet
        {
            if (params.size() != 3)
                throw EvalExcept(EVAL_WRONG_NUMBER_OF_PARAMETERS);
            auto list = context.eval(params[0]);
            if (list.index() != 2)
                throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);

            auto l = std::get<2>(list);

            auto st = context.eval(params[1]);
            if (st.index() != 1)
                throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);
            auto s = std::round(std::get<1>(st));
            if (s < 0 || s > l.size())
                throw EvalExcept(EVAL_INDEX_OUT_OF_RANGE);

            auto ed = context.eval(params[2]);
            if (ed.index() != 1)
                throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);
            auto e = std::round(std::get<1>(ed));
            if (e < s || e < 0 || e > l.size())
                throw EvalExcept(EVAL_INDEX_OUT_OF_RANGE);
            return std::vector<decimal_t>(l.begin() + static_cast<size_t>(s), l.begin() + static_cast<size_t>(e));
        },
        "slice"};
    m_globalVarMap["reverse"] = LambdaType{
        {"list"},
        nullptr,
        true,
        [](const std::vector<std::shared_ptr<ASTNode>> &params, Context &context) -> InternalFuncRet
        {
            if (params.size() != 1)
                throw EvalExcept(EVAL_WRONG_NUMBER_OF_PARAMETERS);
            auto list = context.eval(params[0]);
            if (list.index() != 2)
                throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);

            std::vector<decimal_t> l = std::get<2>(list);
            std::reverse(l.begin(), l.end());
            return l;
        },
        "reverse"};
}

} // namespace eval