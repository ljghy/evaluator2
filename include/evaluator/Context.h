#ifndef EVAL_CONTEXT_H_
#define EVAL_CONTEXT_H_

#include <evaluator/Parser.h>
#include <unordered_set>
#include <unordered_map>
#include <functional>

namespace eval
{

struct VoidType
{
};

using ListType = std::vector<decimal_t>;

struct InternalFuncRet;

class Context;
struct LambdaType
{
    std::vector<std::string> params;
    std::shared_ptr<ASTNode> expr;
    bool isInternalFunc = false;
    std::function<InternalFuncRet(const std::vector<std::shared_ptr<ASTNode>> &, Context &)> internalFuncDef;
};

enum class InternalFuncRetType
{
    DECIMAL,
    LIST,
    LAMBDA
};
struct InternalFuncRet
{
    InternalFuncRetType type;
    decimal_t decimal;
    ListType list;
    LambdaType lambda;
    InternalFuncRet(decimal_t d) : type(InternalFuncRetType::DECIMAL), decimal(d) {}
    InternalFuncRet(const ListType &l) : type(InternalFuncRetType::LIST), list(l) {}
    InternalFuncRet(const LambdaType &l) : type(InternalFuncRetType::LAMBDA), lambda(l) {}
};

using DataType = std::variant<VoidType, decimal_t, ListType, LambdaType>;
using VarMap = std::unordered_map<std::string, DataType>;

class Context
{
public:
    void init();
    void setupInternalFunc();
    DataType exec(const std::string &);
    DataType eval(std::shared_ptr<ASTNode>);
    const VarMap &varMap() const { return m_globalVarMap; }

private:
    DataType call(const LambdaType &, const std::vector<std::shared_ptr<ASTNode>> &);

    static std::shared_ptr<ASTNode> substitude(const std::shared_ptr<ASTNode> &,
                                               const VarMap &,
                                               std::unordered_set<std::string>);

    static DataType neg(const DataType &);
    static DataType binOp(const DataType &, const DataType &, OptrType);

    static ListType listAdd(const ListType &, decimal_t);
    static ListType listAdd(const ListType &, const ListType &);

    static ListType listSub(const ListType &, decimal_t);
    static ListType listSub(decimal_t, const ListType &);
    static ListType listSub(const ListType &, const ListType &);

    static ListType listMul(const ListType &, decimal_t);
    static ListType listMul(const ListType &, const ListType &);

    static ListType listDiv(const ListType &, decimal_t);
    static ListType listDiv(decimal_t, const ListType &);
    static ListType listDiv(const ListType &, const ListType &);

    static ListType listPow(const ListType &, decimal_t);
    static ListType listPow(decimal_t, const ListType &);
    static ListType listPow(const ListType &, const ListType &);

private:
    VarMap m_globalVarMap;
};
} // namespace eval

#endif