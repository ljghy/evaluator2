#include <evaluator/Tokenizer.h>

#include <unordered_map>
#include <sstream>

namespace eval
{

static void skipSpace(std::string::const_iterator &ite,
                      const std::string::const_iterator &end)
{
    while (ite != end && std::isspace(*ite))
        ++ite;
}

static decimal_t parseDecimal(const std::string &src,
                              std::string::const_iterator &ite)
{
    size_t idx = 0;
    decimal_t d;

    try
    {
        d = strToDecimal<decimal_t>(src.substr(ite - src.begin()), &idx);
    }
    catch (const std::invalid_argument &)
    {
        return d;
    }
    catch (const std::out_of_range &)
    {
        throw EvalExcept(EVAL_DECIMAL_OUT_OF_RANGE);
    }
    ite += idx;
    return d;
}

static void parseSymbol(std::string::const_iterator &ite,
                        const std::string::const_iterator &end)
{
    if (!((*ite >= 'a' && *ite <= 'z') ||
          (*ite >= 'A' && *ite <= 'Z') ||
          (*ite == '_')))
        return;

    while (ite != end && ((*ite >= 'a' && *ite <= 'z') ||
                          (*ite >= 'A' && *ite <= 'Z') ||
                          (*ite >= '0' && *ite <= '9') ||
                          (*ite == '_')))
        ++ite;
}

static const std::unordered_map<char, TokenType> tkMap{
    {'+', TokenType::ADD},
    {'-', TokenType::SUB},
    {'*', TokenType::MUL},
    {'/', TokenType::DIV},
    {'^', TokenType::POW},

    {'(', TokenType::LPAR},
    {')', TokenType::RPAR},
    {'[', TokenType::LSQR},
    {']', TokenType::RSQR},
    {'{', TokenType::LCUR},
    {'}', TokenType::RCUR},

    {'@', TokenType::LAMBDA},
    {',', TokenType::COMMA},
    {'=', TokenType::ASSIGN},
};

TokenList tokenize(const std::string &src)
{
    TokenList ret{};
    auto ite = src.begin();
    const auto end = src.end();

    while (ite != end)
    {
        skipSpace(ite, end);
        if (ite == end)
            return ret;

        auto tkIte = tkMap.find(*ite);
        if (tkIte != tkMap.end())
        {
            ++ite;
            ret.push_back(tkIte->second);
            continue;
        }

        const auto beg = ite;

        decimal_t d = parseDecimal(src, ite);
        if (ite != beg)
        {
            ret.push_back(d);
            continue;
        }

        parseSymbol(ite, end);
        if (ite != beg)
        {
            ret.push_back(src.substr(beg - src.begin(), ite - beg));
            continue;
        }
        else
            throw EvalExcept(EVAL_PARSE_FAILED);
    }
    return ret;
}

} // namespace eval