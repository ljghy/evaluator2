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
        auto y = context.eval(params[1]);               \
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
        UNARY_FUNC_TEMPLATE(std::sin),
        "sin"};
    m_globalVarMap["cos"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::cos),
        "cos"};
    m_globalVarMap["tan"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::tan),
        "tan"};
    m_globalVarMap["asin"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::asin),
        "asin"};
    m_globalVarMap["acos"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::acos),
        "acos"};
    m_globalVarMap["atan"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::atan),
        "atan"};
    m_globalVarMap["exp"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::exp),
        "exp"};
    m_globalVarMap["ln"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::log),
        "ln"};
    m_globalVarMap["abs"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::abs),
        "abs"};
    m_globalVarMap["floor"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::floor),
        "floor"};
    m_globalVarMap["ceil"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::ceil),
        "ceil"};
    m_globalVarMap["round"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::round),
        "round"};
    m_globalVarMap["erf"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::erf),
        "erf"};
    m_globalVarMap["gamma"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::tgamma),
        "gamma"};
    m_globalVarMap["sqrt"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(std::sqrt),
        "sqrt"};
    m_globalVarMap["not"] = LambdaType{
        {"x"},
        nullptr,
        true,
        UNARY_FUNC_TEMPLATE(!),
        "not"};

    m_globalVarMap["eq"] = LambdaType{
        {"x", "y"},
        nullptr,
        true,
        CMP_OPTR_TEMPLATE(==),
        "eq"};
    m_globalVarMap["neq"] = LambdaType{
        {"x", "y"},
        nullptr,
        true,
        CMP_OPTR_TEMPLATE(!=),
        "neq"};
    m_globalVarMap["gt"] = LambdaType{
        {"x", "y"},
        nullptr,
        true,
        CMP_OPTR_TEMPLATE(>),
        "gt"};
    m_globalVarMap["lt"] = LambdaType{
        {"x", "y"},
        nullptr,
        true,
        CMP_OPTR_TEMPLATE(<),
        "lt"};
    m_globalVarMap["geq"] = LambdaType{
        {"x", "y"},
        nullptr,
        true,
        CMP_OPTR_TEMPLATE(>=),
        "geq"};
    m_globalVarMap["leq"] = LambdaType{
        {"x", "y"},
        nullptr,
        true,
        CMP_OPTR_TEMPLATE(<=),
        "leq"};

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

#endif