#ifndef EVAL_INTERNAL_FUNC_INL_
#define EVAL_INTERNAL_FUNC_INL_

#define UNARY_FUNC_TEMPLATE(f)                                                                    \
    ([](const std::vector<std::shared_ptr<ASTNode>> &params, Context &context) -> InternalFuncRet \
     {                                                  \
        if (params.size() != 1)                        \
            throw EvalExcept(EVAL_WRONG_NUMBER_OF_PARAMETERS); \
        auto x = context.eval(params[0]);              \
        if (x.index() == 1)                            \
            return f(std::get<1>(x));                \
        if (x.index() == 2)                            \
        {                                              \
            auto l = std::get<2>(x);                   \
            for (auto &x : l)                          \
                x = f(x);                            \
            return l;                                  \
        }                                              \
        throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);     \
        return decimal_t(0); })

#define CMP_OPTR_TEMPLATE(optr)                                                                   \
    ([](const std::vector<std::shared_ptr<ASTNode>> &params, Context &context) -> InternalFuncRet \
     {                                                  \
        if (params.size() != 2)                        \
            throw EvalExcept(EVAL_WRONG_NUMBER_OF_PARAMETERS); \
        auto x = context.eval(params[0]);              \
        auto y = context.eval(params[1]);              \
        if (x.index() == 1 && y.index() == 1)                            \
            return decimal_t(std::get<1>(x) optr std::get<1>(y));                                       \
        throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);     \
        return decimal_t(0); })

void Context::setupInternalFunc()
{
    m_globalVarMap["e"] = std::exp(1);
    m_globalVarMap["pi"] = std::acos(-1);
    m_globalVarMap["ans"] = decimal_t(0);

    m_globalVarMap["sin"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::sin)};
    m_globalVarMap["cos"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::cos)};
    m_globalVarMap["tan"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::tan)};
    m_globalVarMap["asin"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::asin)};
    m_globalVarMap["acos"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::acos)};
    m_globalVarMap["atan"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::atan)};
    m_globalVarMap["exp"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::exp)};
    m_globalVarMap["ln"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::log)};
    m_globalVarMap["abs"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::abs)};
    m_globalVarMap["floor"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::floor)};
    m_globalVarMap["ceil"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::ceil)};
    m_globalVarMap["round"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::round)};
    m_globalVarMap["erf"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::erf)};
    m_globalVarMap["gamma"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::tgamma)};
    m_globalVarMap["not"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(!)};

    m_globalVarMap["eq"] = LambdaType{
        {"x", "y"},
        nullptr,
        true,
        CMP_OPTR_TEMPLATE(==)};
    m_globalVarMap["neq"] = LambdaType{
        {"x", "y"},
        nullptr,
        true,
        CMP_OPTR_TEMPLATE(!=)};
    m_globalVarMap["gt"] = LambdaType{
        {"x", "y"},
        nullptr,
        true,
        CMP_OPTR_TEMPLATE(>)};
    m_globalVarMap["lt"] = LambdaType{
        {"x", "y"},
        nullptr,
        true,
        CMP_OPTR_TEMPLATE(<)};
    m_globalVarMap["geq"] = LambdaType{
        {"x", "y"},
        nullptr,
        true,
        CMP_OPTR_TEMPLATE(>=)};
    m_globalVarMap["leq"] = LambdaType{
        {"x", "y"},
        nullptr,
        true,
        CMP_OPTR_TEMPLATE(<=)};

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
        }};

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
        }};

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
        }};

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
        }};
}

#endif