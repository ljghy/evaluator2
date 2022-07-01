#ifndef EVAL_DEFS_H_
#define EVAL_DEFS_H_

#include <cstddef>
#include <string>

namespace eval
{
using decimal_t = double;

enum EvalErrCode
{
    EVAL_DECIMAL_OUT_OF_RANGE = 0,
    EVAL_PARSE_FAILED,
    EVAL_IDENTIFIER_UNDEFINED,
    EVAL_OBJECT_NOT_CALLABLE,
    EVAL_OBJECT_NOT_LIST,
    EVAL_INDEX_NOT_DECIMAL,
    EVAL_INDEX_OUT_OF_RANGE,
    EVAL_LIST_MEMBER_NOT_DECIMAL,
    EVAL_WRONG_NUMBER_OF_PARAMETERS,
    EVAL_WRONG_OPERAND_TYPE,
    EVAL_DIFFERENT_LIST_LENGTHS,
    EVAL_WRONG_PARAMETER_TYPE,
};

inline const std::string EvalErrMsg[]{
    "syntax error: decimal out of range",
    "syntax error: parse failed",
    "runtime error: identifier undefined",
    "runtime error: object not callable",
    "runtime error: object not list",
    "runtime error: index not decimal",
    "runtime error: index out of range",
    "runtime error: list member not decimal",
    "runtime error: wrong number of parameters",
    "runtime error: wrong operand type",
    "runtime error: different list lengths",
    "runtime error: wrong parameter type",
};

class EvalExcept
{
private:
    std::string m_whatStr;
    EvalErrCode m_code;

public:
    EvalExcept(const EvalErrCode &err)
        : m_code(err)
    {
        m_whatStr = EvalErrMsg[static_cast<size_t>(err)];
    }

    const char *what() const throw() { return m_whatStr.c_str(); }
    EvalErrCode code() const throw() { return m_code; }
};

} // namespace eval

#endif