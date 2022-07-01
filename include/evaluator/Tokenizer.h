#ifndef EVAL_TOKENIZER_H_
#define EVAL_TOKENIZER_H_

#include <evaluator/EvalDefs.h>

#include <cassert>
#include <string>
#include <variant>
#include <vector>

#ifndef NDEBUG
#include <unordered_map>
#endif

namespace eval
{
enum class TokenType
{
    DECIMAL,
    IDENT,

    ADD, // +
    SUB, // -
    MUL, // *
    DIV, // /
    POW, // ^

    LPAR, // (
    RPAR, // )
    LSQR, // [
    RSQR, // ]
    LCUR, // {
    RCUR, // }

    LAMBDA, // @
    COMMA,  // ,
    ASSIGN, // =
};

#ifndef NDEBUG

inline std::unordered_map<TokenType, std::string> tokenRepr{
    {TokenType::DECIMAL, "DECIMAL"},
    {TokenType::IDENT, "IDENT"},
    {TokenType::ADD, "ADD"},
    {TokenType::SUB, "SUB"},
    {TokenType::MUL, "MUL"},
    {TokenType::DIV, "DIV"},
    {TokenType::POW, "POW"},
    {TokenType::LPAR, "LPAR"},
    {TokenType::RPAR, "RPAR"},
    {TokenType::LSQR, "LSQR"},
    {TokenType::RSQR, "RSQR"},
    {TokenType::LCUR, "LCUR"},
    {TokenType::RCUR, "RCUR"},
    {TokenType::LAMBDA, "LAMBDA"},
    {TokenType::COMMA, "COMMA"},
    {TokenType::ASSIGN, "ASSIGN"},
};
#endif

struct Token
{
    TokenType type;
    std::variant<decimal_t, std::string> value;

    Token() = default;
    Token(const decimal_t &v) : type(TokenType::DECIMAL), value(v) {}
    Token(const std::string &v) : type(TokenType::IDENT), value(v) {}
    Token(const TokenType &ty) : type(ty) { assert((ty != TokenType::DECIMAL) && (ty != TokenType::IDENT)); }

    std::string getIdent() const
    {
        assert((type == TokenType::IDENT) && (value.index() == 1));
        return std::get<1>(value);
    }

    decimal_t getDecimal() const
    {
        assert((type == TokenType::DECIMAL));
        assert((value.index() == 0));
        return std::get<0>(value);
    }
};

template <typename Ty>
inline Ty strToDecimal(const std::string &, size_t *) {}
template <>
inline float strToDecimal<float>(const std::string &s, size_t *idx) { return std::stof(s, idx); }
template <>
inline double strToDecimal<double>(const std::string &s, size_t *idx) { return std::stod(s, idx); }
template <>
inline long double strToDecimal<long double>(const std::string &s, size_t *idx) { return std::stold(s, idx); }

using TokenList = std::vector<Token>;
TokenList tokenize(const std::string &src);

} // namespace eval

#endif