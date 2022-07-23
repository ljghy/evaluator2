#include <evaluator/InternalFunc.h>
#include <cmath>

#define UNARY_FUNC_IMPL(name, impl)                                                                        \
    InternalFuncRet internal_##name(const std::vector<std::shared_ptr<ASTNode>> &params, Context &context) \
    {                                                                                                      \
        if (params.size() != 1)                                                                            \
            throw EvalExcept(EVAL_WRONG_NUMBER_OF_PARAMETERS);                                             \
        auto x = context.eval(params[0]);                                                                  \
        if (x.index() == 1)                                                                                \
            return impl(std::get<1>(x));                                                                   \
        if (x.index() == 2)                                                                                \
        {                                                                                                  \
            auto l = std::get<2>(x);                                                                       \
            for (auto &x : l)                                                                              \
                x = impl(x);                                                                               \
            return l;                                                                                      \
        }                                                                                                  \
        throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);                                                       \
        return decimal_t(0);                                                                               \
    }

#define CMP_OPTR_IMPL(name, optr)                                                                          \
    InternalFuncRet internal_##name(const std::vector<std::shared_ptr<ASTNode>> &params, Context &context) \
    {                                                                                                      \
        if (params.size() != 2)                                                                            \
            throw EvalExcept(EVAL_WRONG_NUMBER_OF_PARAMETERS);                                             \
        auto x = context.eval(params[0]);                                                                  \
        auto y = context.eval(params[1]);                                                                  \
        if (x.index() == 1 && y.index() == 1)                                                              \
            return decimal_t(std::get<1>(x) optr std::get<1>(y));                                          \
        throw EvalExcept(EVAL_WRONG_PARAMETER_TYPE);                                                       \
        return decimal_t(0);                                                                               \
    }

namespace eval
{
UNARY_FUNC_IMPL(sin, std::sin)
UNARY_FUNC_IMPL(cos, std::cos)
UNARY_FUNC_IMPL(tan, std::tan)

UNARY_FUNC_IMPL(asin, std::asin)
UNARY_FUNC_IMPL(acos, std::acos)
UNARY_FUNC_IMPL(atan, std::atan)

UNARY_FUNC_IMPL(exp, std::exp)
UNARY_FUNC_IMPL(ln, std::log)

UNARY_FUNC_IMPL(abs, std::abs)

UNARY_FUNC_IMPL(floor, std::floor)
UNARY_FUNC_IMPL(ceil, std::ceil)
UNARY_FUNC_IMPL(round, std::round)

UNARY_FUNC_IMPL(sqrt, std::sqrt)
UNARY_FUNC_IMPL(erf, std::erf)
UNARY_FUNC_IMPL(gamma, std::tgamma)

UNARY_FUNC_IMPL(not, !)

CMP_OPTR_IMPL(eq, ==)
CMP_OPTR_IMPL(neq, !=)
CMP_OPTR_IMPL(gt, >)
CMP_OPTR_IMPL(lt, <)
CMP_OPTR_IMPL(geq, >=)
CMP_OPTR_IMPL(leq, <=)
} // namespace eval