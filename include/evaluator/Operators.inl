#ifndef EVAL_OPERATORS_INL_
#define EVAL_OPERATORS_INL_

DataType Context::neg(const DataType &d)
{
    if (d.index() == 1)
        return -std::get<1>(d);
    if (d.index() == 2)
    {
        auto ret = std::get<2>(d);
        for (auto &x : ret)
            x = -x;
        return ret;
    }
    throw EvalExcept(EVAL_WRONG_OPERAND_TYPE);
}

ListType Context::listAdd(const ListType &l, decimal_t d)
{
    auto ret = l;
    for (auto &x : ret)
        x += d;
    return ret;
}
ListType Context::listAdd(const ListType &l1, const ListType &l2)
{
    if (l1.size() != l2.size())
        throw EvalExcept(EVAL_DIFFERENT_LIST_LENGTHS);
    auto ret = l1;
    for (size_t i = 0; i < ret.size(); ++i)
        ret[i] += l2[i];
    return ret;
}

ListType Context::listSub(const ListType &l, decimal_t d)
{
    auto ret = l;
    for (auto &x : ret)
        x -= d;
    return ret;
}
ListType Context::listSub(decimal_t d, const ListType &l)
{
    auto ret = l;
    for (auto &x : ret)
        x = d - x;
    return ret;
}
ListType Context::listSub(const ListType &l1, const ListType &l2)
{
    if (l1.size() != l2.size())
        throw EvalExcept(EVAL_DIFFERENT_LIST_LENGTHS);
    auto ret = l1;
    for (size_t i = 0; i < ret.size(); ++i)
        ret[i] -= l2[i];
    return ret;
}

ListType Context::listMul(const ListType &l, decimal_t d)
{
    auto ret = l;
    for (auto &x : ret)
        x *= d;
    return ret;
}
ListType Context::listMul(const ListType &l1, const ListType &l2)
{
    if (l1.size() != l2.size())
        throw EvalExcept(EVAL_DIFFERENT_LIST_LENGTHS);
    auto ret = l1;
    for (size_t i = 0; i < ret.size(); ++i)
        ret[i] *= l2[i];
    return ret;
}

ListType Context::listDiv(const ListType &l, decimal_t d)
{
    auto ret = l;
    auto id = 1 / d;
    for (auto &x : ret)
        x *= id;
    return ret;
}
ListType Context::listDiv(decimal_t d, const ListType &l)
{
    auto ret = l;
    for (auto &x : ret)
        x = d / x;
    return ret;
}
ListType Context::listDiv(const ListType &l1, const ListType &l2)
{
    if (l1.size() != l2.size())
        throw EvalExcept(EVAL_DIFFERENT_LIST_LENGTHS);
    auto ret = l1;
    for (size_t i = 0; i < ret.size(); ++i)
        ret[i] /= l2[i];
    return ret;
}

ListType Context::listPow(const ListType &l, decimal_t d)
{
    auto ret = l;
    for (auto &x : ret)
        x = std::pow(x, d);
    return ret;
}
ListType Context::listPow(decimal_t d, const ListType &l)
{
    auto ret = l;
    for (auto &x : ret)
        x = std::pow(d, x);
    return ret;
}
ListType Context::listPow(const ListType &l1, const ListType &l2)
{
    if (l1.size() != l2.size())
        throw EvalExcept(EVAL_DIFFERENT_LIST_LENGTHS);
    auto ret = l1;
    for (size_t i = 0; i < ret.size(); ++i)
        ret[i] = std::pow(ret[i], l2[i]);
    return ret;
}

DataType Context::binOp(const DataType &d1, const DataType &d2, OptrType op)
{
    switch (op)
    {
    case OptrType::ADD:
    {
        if (d1.index() == 1 && d2.index() == 1)
            return std::get<1>(d1) + std::get<1>(d2);
        if (d1.index() == 2 && d2.index() == 1)
            return listAdd(std::get<2>(d1), std::get<1>(d2));
        if (d1.index() == 1 && d2.index() == 2)
            return listAdd(std::get<2>(d2), std::get<1>(d1));
        if (d1.index() == 2 && d2.index() == 2)
            return listAdd(std::get<2>(d1), std::get<2>(d2));
        throw EvalExcept(EVAL_WRONG_OPERAND_TYPE);
    }
    break;
    case OptrType::SUB:
    {
        if (d1.index() == 1 && d2.index() == 1)
            return std::get<1>(d1) - std::get<1>(d2);
        if (d1.index() == 2 && d2.index() == 1)
            return listSub(std::get<2>(d1), std::get<1>(d2));
        if (d1.index() == 1 && d2.index() == 2)
            return listSub(std::get<1>(d1), std::get<2>(d2));
        if (d1.index() == 2 && d2.index() == 2)
            return listSub(std::get<2>(d1), std::get<2>(d2));
        throw EvalExcept(EVAL_WRONG_OPERAND_TYPE);
    }
    break;
    case OptrType::MUL:
    {
        if (d1.index() == 1 && d2.index() == 1)
            return std::get<1>(d1) * std::get<1>(d2);
        if (d1.index() == 2 && d2.index() == 1)
            return listMul(std::get<2>(d1), std::get<1>(d2));
        if (d1.index() == 1 && d2.index() == 2)
            return listMul(std::get<2>(d2), std::get<1>(d1));
        if (d1.index() == 2 && d2.index() == 2)
            return listMul(std::get<2>(d1), std::get<2>(d2));
        throw EvalExcept(EVAL_WRONG_OPERAND_TYPE);
    }
    break;
    case OptrType::DIV:
    {
        if (d1.index() == 1 && d2.index() == 1)
            return std::get<1>(d1) / std::get<1>(d2);
        if (d1.index() == 2 && d2.index() == 1)
            return listDiv(std::get<2>(d1), std::get<1>(d2));
        if (d1.index() == 1 && d2.index() == 2)
            return listDiv(std::get<1>(d1), std::get<2>(d2));
        if (d1.index() == 2 && d2.index() == 2)
            return listDiv(std::get<2>(d1), std::get<2>(d2));
        throw EvalExcept(EVAL_WRONG_OPERAND_TYPE);
    }
    break;
    case OptrType::POW:
    {
        if (d1.index() == 1 && d2.index() == 1)
            return std::pow(std::get<1>(d1), std::get<1>(d2));
        if (d1.index() == 2 && d2.index() == 1)
            return listPow(std::get<2>(d1), std::get<1>(d2));
        if (d1.index() == 1 && d2.index() == 2)
            return listPow(std::get<1>(d1), std::get<2>(d2));
        if (d1.index() == 2 && d2.index() == 2)
            return listPow(std::get<2>(d1), std::get<2>(d2));
        throw EvalExcept(EVAL_WRONG_OPERAND_TYPE);
    }
    break;
    default:
        assert(0);
    }
    return VoidType{};
}

#endif