#ifndef EVAL_INTERNAL_FUNC_H_
#define EVAL_INTERNAL_FUNC_H_

#include <evaluator/Context.h>

#define INTERNAL_FUNC_DECL(f) \
    InternalFuncRet           \
        internal_##f(const std::vector<std::shared_ptr<ASTNode>> &, Context &);

namespace eval
{

INTERNAL_FUNC_DECL(sin)
INTERNAL_FUNC_DECL(cos)
INTERNAL_FUNC_DECL(tan)

INTERNAL_FUNC_DECL(asin)
INTERNAL_FUNC_DECL(acos)
INTERNAL_FUNC_DECL(atan)

INTERNAL_FUNC_DECL(exp);
INTERNAL_FUNC_DECL(ln);

INTERNAL_FUNC_DECL(abs);

INTERNAL_FUNC_DECL(floor);
INTERNAL_FUNC_DECL(ceil);
INTERNAL_FUNC_DECL(round);

INTERNAL_FUNC_DECL(sqrt);
INTERNAL_FUNC_DECL(erf);
INTERNAL_FUNC_DECL(gamma);

INTERNAL_FUNC_DECL(not );

INTERNAL_FUNC_DECL(eq);
INTERNAL_FUNC_DECL(neq);
INTERNAL_FUNC_DECL(gt);
INTERNAL_FUNC_DECL(lt);
INTERNAL_FUNC_DECL(geq);
INTERNAL_FUNC_DECL(leq);

} // namespace eval

#endif