#ifndef JSON_PARSER_HPP_
#define JSON_PARSER_HPP_

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <memory>
#include <stdexcept>
#include <cassert>
#include <cstdint>
#include <cmath>
#include <sstream>

enum JsonParseErrCode
{
    JSON_EMPTY_BUFFER = 0,
    JSON_ROOT_NOT_SINGLE_VALUE,
    JSON_TOO_DEEP,

    JSON_INVALID_VALUE,

    JSON_INVALID_LITERAL,

    JSON_INVALID_NUMBER,
    JSON_NUMBER_OUT_OF_RANGE,

    JSON_INVALID_STRING_CHAR,
    JSON_INVALID_STRING_ESCAPE_SEQUENCE,
    JSON_INVALID_UNICODE_SURROGATE,
    JSON_UNEXPECTED_END_OF_STRING,
    JSON_INVALID_UNICODE_HEX,
    JSON_INVALID_UTF8_CHAR,

    JSON_MISS_COMMA_OR_SQUARE_BRACKET,
    JSON_TRAILING_COMMA,
    JSON_MISS_COMMA_OR_CURLY_BRACKET,
    JSON_INVALID_KEY_VALUE_PAIR,
    JSON_MISS_VALUE,
};

inline const std::string JsonParseErrMsg[]{
    "Empty buffer",
    "Root not single value",
    "Too deep",

    "Invalid value",

    "Invalid literal",

    "Invalid number",
    "Number out of range",

    "Invalid string char",
    "Invalid string escape sequence",
    "Invalid unicode surrogate",
    "Unexpected end of string",
    "Invalid unicode hex",
    "Invalid utf-8 char",

    "Miss comma or square bracket",
    "Trailing comma",
    "Miss comma or curly bracket",
    "Invalid key-value pair",
    "Miss value",
};

class JsonParseExcept
{
private:
    std::string m_whatStr;
    JsonParseErrCode m_code;
    size_t m_pos;
    size_t m_line; // start from 1
    size_t m_col;  // start from 1

public:
    JsonParseExcept(const JsonParseErrCode &err,
                    size_t pos, size_t line, size_t col)
        : m_code(err), m_pos(pos), m_line(line), m_col(col)
    {
        m_whatStr = JsonParseErrMsg[static_cast<size_t>(err)] +
                    " at line " + std::to_string(m_line) + ':' + std::to_string(m_col);
    }

    const char *what() const throw() { return m_whatStr.c_str(); }
    JsonParseErrCode code() const throw() { return m_code; }
    size_t pos() const throw() { return m_pos; }
    size_t line() const throw() { return m_line; }
    size_t col() const throw() { return m_col; }
};

class JsonNode;

struct JsonEmpty_t
{
};
constexpr struct JsonNull_t
{
} JsonNull;

using JsonBool_t = bool;

#ifndef JSON_PARSER_USE_FLOAT
using JsonNum_t = double;
#else
using JsonNum_t = float;
#endif

using JsonStr_t = std::string;
using JsonArr_t = std::vector<std::shared_ptr<JsonNode>>;
using JsonObj_t = std::unordered_multimap<JsonStr_t, std::shared_ptr<JsonNode>>;

inline const std::string JsonTypeStr[]{
    "empty",
    "null",
    "bool",
    "number",
    "string",
    "array",
    "object"};

inline constexpr uint16_t JSON_MAX_DEPTH = 20;
inline constexpr uint16_t JSON_DECIMAL_TO_STRING_PRECISION = 12;

class JsonNode
{
private:
    std::variant<JsonEmpty_t, JsonNull_t, JsonBool_t, JsonNum_t, JsonStr_t, JsonArr_t, JsonObj_t> m_value;
    enum
    {
        JsonEmptyType = 0,
        JsonNullType,
        JsonBoolType,
        JsonNumType,
        JsonStrType,
        JsonArrType,
        JsonObjType
    };

    static std::string _toHex4(uint64_t u)
    {
        char h[4];
        for (int8_t i = 3; i >= 0; --i)
        {
            h[i] = u & 0xF;
            h[i] = h[i] < 10 ? (h[i] + '0') : (h[i] - 10 + 'a');
            u >>= 4;
        }
        return "\\u" + std::string(h, 4);
    }

    static std::string _decodeUTF8(std::string::const_iterator &ite,
                                   const std::string &src)
    {
        uint64_t u = 0;
        if (!(*ite & 0x20))
        {
            u = ((*ite & 0x1F) << 6) | (*(ite + 1) & 0x3F);
            ++ite;
        }
        else if (!(*ite & 0x10))
        {
            u = ((*ite & 0xF) << 12) | ((*(ite + 1) & 0x3F) << 6) | (*(ite + 2) & 0x3F);
            ite += 2;
        }
        else
        {
            u = ((*ite & 0x7) << 18) | ((*(ite + 1) & 0x3F) << 12) | ((*(ite + 2) & 0x3F) << 6) | (*(ite + 3) & 0x3F);
            ite += 3;
        }

        if (u >= 0x10000)
        {
            uint64_t l = 0xDC00 + (u & 0x3FF);
            uint64_t u0 = ((u - 0x10000) >> 10) + 0xD800;
            if (u0 <= 0xDBFF)
            {
                return _toHex4(u0) + _toHex4(l);
            }
        }
        return _toHex4(u);
    }

    static std::string _toJsonString(const std::string &src, bool decodeUTF8)
    {
        std::stringstream ss;
        for (auto ite = src.begin(); ite != src.end(); ++ite)
        {
            switch (*ite)
            {
            case '"':
            case '\\':
            case '/':
                ss << '\\' << *ite;
                break;
            case '\b':
                ss << "\\b";
                break;
            case '\f':
                ss << "\\f";
                break;
            case '\n':
                ss << "\\n";
                break;
            case '\r':
                ss << "\\r";
                break;
            case '\t':
                ss << "\\t";
                break;
            default:
            {
                if (decodeUTF8 && *ite & 0x80)
                    ss << _decodeUTF8(ite, src);
                else
                    ss << *ite;
            }
            break;
            }
        }
        return ss.str();
    }

    std::string _toString(uint16_t depth, bool decodeUTF8) const
    {
        if (depth >= JSON_MAX_DEPTH)
        {
            throw std::runtime_error("Json string conversion failure: json too deep");
            return "...";
        }
        switch (m_value.index())
        {
        case JsonEmptyType:
            return "";
        case JsonNullType:
            return "null";
        case JsonBoolType:
            return getBool() ? "true" : "false";
        case JsonNumType:
        {
            std::stringstream ss;
            ss.precision(JSON_DECIMAL_TO_STRING_PRECISION);
            ss << getNum();
            return ss.str();
        }
        case JsonStrType:
            return '"' + _toJsonString(getStr(), decodeUTF8) + '"';
        case JsonArrType:
        {
            JsonArr_t arr = getArr();
            if (arr.size() == 0)
                return "[]";
            std::stringstream ss;
            ss << '[';
            for (size_t i = 0; i != arr.size() - 1; ++i)
                ss << arr[i]->_toString(depth + 1, decodeUTF8) << ", ";
            ss << arr.back()->_toString(depth + 1, decodeUTF8) << ']';
            return ss.str();
        }
        default: // JsonObjType
        {
            JsonObj_t obj = getObj();
            if (obj.size() == 0)
                return "{}";
            size_t i = 0;
            std::stringstream ss;
            ss << '{';
            for (auto &each : obj)
            {
                ss << '"' << _toJsonString(each.first, decodeUTF8) << "\": " << each.second->_toString(depth + 1, decodeUTF8);
                ++i;
                if (i < obj.size())
                    ss << ", ";
            }
            ss << '}';
            return ss.str();
        }
        }
    }

    std::string _toStringFormatted(uint16_t depth, uint8_t indent, bool decodeUTF8) const
    {
        if (depth >= JSON_MAX_DEPTH)
        {
            throw std::runtime_error("Json string conversion failure: json too deep");
            return "...";
        }
        switch (m_value.index())
        {
        case JsonEmptyType:
            return "";
        case JsonNullType:
            return "null";
        case JsonBoolType:
            return getBool() ? "true" : "false";
        case JsonNumType:
        {
            std::stringstream ss;
            ss.precision(JSON_DECIMAL_TO_STRING_PRECISION);
            ss << getNum();
            return ss.str();
        }
        case JsonStrType:
            return '"' + _toJsonString(getStr(), decodeUTF8) + '"';
        case JsonArrType:
        {
            JsonArr_t arr = getArr();
            if (arr.size() == 0)
                return "[]";
            std::stringstream ss;
            std::string spaces(depth * indent, ' '), spaces2(indent, ' ');
            ss << "[\n";
            for (size_t i = 0; i != arr.size() - 1; ++i)
                ss << spaces << spaces2 << arr[i]->_toStringFormatted(depth + 1, indent, decodeUTF8) << ",\n";
            ss << spaces << spaces2 << arr.back()->_toStringFormatted(depth + 1, indent, decodeUTF8) << '\n'
               << spaces << ']';
            return ss.str();
        }
        default: // JsonObjType
        {
            JsonObj_t obj = getObj();
            if (obj.size() == 0)
                return "{}";
            size_t i = 0;
            std::stringstream ss;
            std::string spaces(depth * indent, ' '), spaces2(indent, ' ');
            ss << "{\n";
            for (auto &each : obj)
            {
                ss << spaces << spaces2 << '"' << _toJsonString(each.first, decodeUTF8)
                   << "\": " << each.second->_toStringFormatted(depth + 1, indent, decodeUTF8);
                ++i;
                if (i < obj.size())
                    ss << ',';
                ss << '\n';
            }
            ss << spaces << '}';
            return ss.str();
        }
        }
    }

public:
    JsonNode() : m_value(JsonEmpty_t()) {}

    template <typename ValType>
    JsonNode(const ValType &val) : m_value(val) {}

    JsonNode(const char *val) : m_value(JsonStr_t(val)) {}

    JsonNode(int8_t val) : m_value(static_cast<JsonNum_t>(val)) {}
    JsonNode(int16_t val) : m_value(static_cast<JsonNum_t>(val)) {}
    JsonNode(int32_t val) : m_value(static_cast<JsonNum_t>(val)) {}
    JsonNode(int64_t val) : m_value(static_cast<JsonNum_t>(val)) {}
    JsonNode(uint8_t val) : m_value(static_cast<JsonNum_t>(val)) {}
    JsonNode(uint16_t val) : m_value(static_cast<JsonNum_t>(val)) {}
    JsonNode(uint32_t val) : m_value(static_cast<JsonNum_t>(val)) {}
    JsonNode(uint64_t val) : m_value(static_cast<JsonNum_t>(val)) {}

    JsonNode(const std::initializer_list<std::shared_ptr<JsonNode>> &val) : m_value(JsonArr_t(val)) {}
    JsonNode(const std::initializer_list<std::pair<const JsonStr_t, std::shared_ptr<JsonNode>>> &val) : m_value(JsonObj_t(val)) {}

    JsonNode(const JsonNode &) = default;
    JsonNode(JsonNode &&other) : m_value(other.m_value) {}

    JsonNode &operator=(const JsonNode &) = default;
    JsonNode &operator=(JsonNode &&) = default;

    bool isEmpty() const { return m_value.index() == JsonEmptyType; }
    bool isNull() const { return m_value.index() == JsonNullType; }
    bool isBool() const { return m_value.index() == JsonBoolType; }
    bool isNum() const { return m_value.index() == JsonNumType; }
    bool isStr() const { return m_value.index() == JsonStrType; }
    bool isArr() const { return m_value.index() == JsonArrType; }
    bool isObj() const { return m_value.index() == JsonObjType; }

    bool operator==(bool b) const { return getBool() == b; }
    bool operator!=(bool b) const { return getBool() != b; }

    bool operator==(JsonNum_t n) const { return getNum() == n; }
    bool operator!=(JsonNum_t n) const { return getNum() != n; }
    bool operator>(JsonNum_t n) const { return getNum() > n; }
    bool operator<(JsonNum_t n) const { return getNum() < n; }
    bool operator>=(JsonNum_t n) const { return getNum() >= n; }
    bool operator<=(JsonNum_t n) const { return getNum() <= n; }

    bool operator==(const JsonStr_t &s) const { return getStr() == s; }
    bool operator!=(const JsonStr_t &s) const { return getStr() != s; }
    bool operator==(const char *s) const { return getStr() == JsonStr_t(s); }
    bool operator!=(const char *s) const { return getStr() != JsonStr_t(s); }

    JsonBool_t getBool() const
    {
        if (!isBool())
            throw std::runtime_error("Bad json access: expect bool but is " + JsonTypeStr[m_value.index()]);
        return std::get<JsonBoolType>(m_value);
    }
    JsonBool_t &getBool()
    {
        if (!isBool())
            throw std::runtime_error("Bad json access: expect bool but is " + JsonTypeStr[m_value.index()]);
        return std::get<JsonBoolType>(m_value);
    }

    JsonNum_t getNum() const
    {
        if (!isNum())
            throw std::runtime_error("Bad json access: expect number but is " + JsonTypeStr[m_value.index()]);
        return std::get<JsonNumType>(m_value);
    }
    JsonNum_t &getNum()
    {
        if (!isNum())
            throw std::runtime_error("Bad json access: expect number but is " + JsonTypeStr[m_value.index()]);
        return std::get<JsonNumType>(m_value);
    }
    template <typename IntType = int>
    IntType getInt() const
    {
        if (!isNum())
            throw std::runtime_error("Bad json access: expect number but is " + JsonTypeStr[m_value.index()]);
        return static_cast<IntType>(std::round(std::get<JsonNumType>(m_value)));
    }

    JsonStr_t getStr() const
    {
        if (!isStr())
            throw std::runtime_error("Bad json access: expect string but is " + JsonTypeStr[m_value.index()]);
        return std::get<JsonStrType>(m_value);
    }
    JsonStr_t &getStr()
    {
        if (!isStr())
            throw std::runtime_error("Bad json access: expect string but is " + JsonTypeStr[m_value.index()]);
        return std::get<JsonStrType>(m_value);
    }

    JsonArr_t getArr() const
    {
        if (!isArr())
            throw std::runtime_error("Bad json access: expect array but is " + JsonTypeStr[m_value.index()]);
        return std::get<JsonArrType>(m_value);
    }
    JsonArr_t &getArr()
    {
        if (!isArr())
            throw std::runtime_error("Bad json access: expect array but is " + JsonTypeStr[m_value.index()]);
        return std::get<JsonArrType>(m_value);
    }

    JsonObj_t getObj() const
    {
        if (!isObj())
            throw std::runtime_error("Bad json access: expect object but is " + JsonTypeStr[m_value.index()]);
        return std::get<JsonObjType>(m_value);
    }
    JsonObj_t &getObj()
    {
        if (!isObj())
            throw std::runtime_error("Bad json access: expect object but is " + JsonTypeStr[m_value.index()]);
        return std::get<JsonObjType>(m_value);
    }

    const JsonNode &operator[](size_t idx) const
    {
        const JsonArr_t &arr = getArr();
        if (idx >= arr.size())
            throw std::runtime_error("Bad json access: index out of range");
        return *(arr[idx]);
    }
    JsonNode &operator[](size_t idx)
    {
        JsonArr_t &arr = getArr();
        if (idx >= arr.size())
            throw std::runtime_error("Bad json access: index out of range");
        return *(arr[idx]);
    }

    const JsonNode &operator[](const JsonStr_t &key) const
    {
        const JsonObj_t &obj = getObj();
        auto ite = obj.find(key);
        if (ite == obj.end())
            throw std::runtime_error("Bad json access: key not found");
        return *(ite->second);
    }
    JsonNode &operator[](const JsonStr_t &key)
    {
        JsonObj_t &obj = getObj();
        auto ite = obj.find(key);
        if (ite != obj.end())
            return *(ite->second);
        return *(obj.insert(std::make_pair(key, std::make_shared<JsonNode>(JsonNull)))->second);
    }
    bool find(const JsonStr_t &key) const
    {
        const JsonObj_t &obj = getObj();
        return (obj.find(key) != obj.end());
    }

    std::string toString(bool decodeUTF8 = true) const
    {
        return _toString(0, decodeUTF8);
    }

    std::string toStringFormatted(uint8_t indent = 4u, bool decodeUTF8 = true) const
    {
        return _toStringFormatted(0, indent, decodeUTF8);
    }

    JsonNode copy() const
    {
        switch (m_value.index())
        {
        case JsonArrType:
        {
            JsonArr_t arr = getArr(), ret;
            ret.reserve(arr.size());
            for (auto &elem : arr)
                ret.push_back(std::make_shared<JsonNode>(elem->copy()));
            return ret;
        }
        case JsonObjType:
        {
            JsonObj_t obj = getObj(), ret;
            ret.reserve(obj.size());
            for (auto &p : obj)
                ret.insert({p.first, std::make_shared<JsonNode>(p.second->copy())});
            return ret;
        }
        default:
            return *this;
        }
    }
};

template <typename ValType>
static inline std::shared_ptr<JsonNode> jsptr(const ValType &val)
{
    return std::make_shared<JsonNode>(val);
}
static inline std::shared_ptr<JsonNode> jsptr(const char *val)
{
    return std::make_shared<JsonNode>(JsonStr_t(val));
}
static inline std::shared_ptr<JsonNode> jsptr(const std::initializer_list<std::shared_ptr<JsonNode>> &val)
{
    return std::make_shared<JsonNode>(JsonArr_t(val));
}
static inline std::shared_ptr<JsonNode> jsptr(const std::initializer_list<std::pair<const JsonStr_t, std::shared_ptr<JsonNode>>> &val)
{
    return std::make_shared<JsonNode>(JsonObj_t(val));
}

class JsonParser
{
private:
    std::string m_buffer;
    std::string::const_iterator m_pos;
    size_t m_line;
    int64_t m_lineLen;

private:
    JsonNode _parseValue(uint16_t);

    JsonNode _parseLiteral();

    JsonStr_t _parseString();

    JsonNode _parseNumber();

    JsonNode _parseArray(uint16_t);

    JsonNode _parseObject(uint16_t);

    void _skipSpace();

    void _parseEscapeSequence(std::string &);
    void _parseUTF8(std::string &);
    uint64_t _parseHex4();
    void _encodeUTF8(std::string &, uint64_t u);

    std::pair<JsonStr_t, std::shared_ptr<JsonNode>>
        _parseKeyValue(uint16_t);

    size_t _pos() const { return static_cast<size_t>(m_pos - m_buffer.begin()); }
    size_t _col() const { return _pos() - m_lineLen; }

public:
    JsonParser() = default;
    JsonParser(const JsonParser &) = delete;
    JsonParser &operator=(const JsonParser &) = delete;

    JsonNode parse(const std::string_view);
};

inline JsonNode JsonParser::parse(const std::string_view buffer_view)
{
    m_buffer = std::string(buffer_view.data(), buffer_view.size());
    m_pos = m_buffer.begin();
    m_line = 1;
    m_lineLen = -1;

    JsonNode ret = _parseValue(0);
    if (ret.isEmpty())
        throw JsonParseExcept(JSON_EMPTY_BUFFER, _pos(), m_line, _col());
    if (m_pos != m_buffer.end())
        throw JsonParseExcept(JSON_ROOT_NOT_SINGLE_VALUE, _pos(), m_line, _col());
    return ret;
}

inline JsonNode JsonParser::_parseValue(uint16_t depth)
{
    if (depth >= JSON_MAX_DEPTH)
        throw JsonParseExcept(JSON_TOO_DEEP, _pos(), m_line, _col());
    _skipSpace();
    JsonNode ret = JsonEmpty_t();
    if (m_pos == m_buffer.end())
        return ret;

    switch (*m_pos)
    {
    case 'n':
    case 't':
    case 'f':
        ret = _parseLiteral();
        break;
    case '"':
        ret = _parseString();
        break;
    case '[':
        ret = _parseArray(depth);
        break;
    case '{':
        ret = _parseObject(depth);
        break;
    default:
        if (std::isdigit(*m_pos) || (*m_pos == '-'))
            ret = _parseNumber();
        else
            throw JsonParseExcept(JSON_INVALID_VALUE, _pos(), m_line, _col());
        break;
    }
    _skipSpace();
    return ret;
}

inline std::pair<JsonStr_t, std::shared_ptr<JsonNode>>
JsonParser::_parseKeyValue(uint16_t depth)
{
    _skipSpace();
    if (m_pos == m_buffer.end() || *m_pos != '"')
    {
        if (*m_pos == '}')
            return {"", std::make_shared<JsonNode>(JsonEmpty_t())};
        throw JsonParseExcept(JSON_INVALID_KEY_VALUE_PAIR, _pos(), m_line, _col());
    }

    JsonStr_t key = _parseString();
    _skipSpace();
    if (m_pos == m_buffer.end() || *m_pos != ':')
    {
        throw JsonParseExcept(JSON_INVALID_KEY_VALUE_PAIR, _pos(), m_line, _col());
    }
    ++m_pos;
    JsonNode node = _parseValue(depth + 1);
    return {key, std::make_shared<JsonNode>(node)};
}

inline JsonNode JsonParser::_parseObject(uint16_t depth)
{
    assert((m_pos != m_buffer.end()) && (*m_pos == '{'));
    ++m_pos;
    JsonObj_t obj;

    while (m_pos != m_buffer.end())
    {
        auto keyValue = _parseKeyValue(depth);
        if (m_pos == m_buffer.end())
        {
            throw JsonParseExcept(JSON_MISS_COMMA_OR_CURLY_BRACKET, _pos(), m_line, _col());
        }
        if (keyValue.second->isEmpty())
        {
            if (*m_pos == '}' && obj.empty())
            {
                ++m_pos;
                return obj;
            }
            if (*m_pos == '}')
                throw JsonParseExcept(JSON_TRAILING_COMMA, _pos(), m_line, _col());
            throw JsonParseExcept(JSON_MISS_VALUE, _pos(), m_line, _col());
        }
        obj.insert(keyValue);
        if (*m_pos == '}')
        {
            ++m_pos;
            return obj;
        }
        if (*m_pos == ',')
            ++m_pos;
        else
        {
            throw JsonParseExcept(JSON_MISS_COMMA_OR_CURLY_BRACKET, _pos(), m_line, _col());
        }
    }
    throw JsonParseExcept(JSON_MISS_COMMA_OR_CURLY_BRACKET, _pos(), m_line, _col());
}

inline JsonNode JsonParser::_parseArray(uint16_t depth)
{
    assert((m_pos != m_buffer.end()) && (*m_pos == '['));
    ++m_pos;
    JsonArr_t arr;

    while (m_pos != m_buffer.end())
    {
        _skipSpace();
        if (m_pos == m_buffer.end())
            throw JsonParseExcept(JSON_MISS_COMMA_OR_SQUARE_BRACKET, _pos(), m_line, _col());
        if (*m_pos == ']')
        {
            ++m_pos;
            if (!arr.empty())
                throw JsonParseExcept(JSON_TRAILING_COMMA, _pos(), m_line, _col());
            return arr;
        }
        JsonNode node = _parseValue(depth + 1);
        if (m_pos == m_buffer.end())
            throw JsonParseExcept(JSON_MISS_COMMA_OR_SQUARE_BRACKET, _pos(), m_line, _col());

        arr.push_back(std::make_shared<JsonNode>(node));
        if (*m_pos == ']')
        {
            ++m_pos;
            return arr;
        }
        if (*m_pos == ',')
            ++m_pos;
        else
        {
            throw JsonParseExcept(JSON_MISS_COMMA_OR_SQUARE_BRACKET, _pos(), m_line, _col());
        }
    }
    throw JsonParseExcept(JSON_MISS_COMMA_OR_SQUARE_BRACKET, _pos(), m_line, _col());
}

inline void JsonParser::_encodeUTF8(std::string &dest, uint64_t u)
{
    if (u <= 0x7F)
        dest.push_back(u & 0xFF);
    else if (u <= 0x7FF)
    {
        dest.push_back(0xC0 | ((u >> 6) & 0xFF));
        dest.push_back(0x80 | (u & 0x3F));
    }
    else if (u <= 0xFFFF)
    {
        dest.push_back(0xE0 | ((u >> 12) & 0xFF));
        dest.push_back(0x80 | ((u >> 6) & 0x3F));
        dest.push_back(0x80 | (u & 0x3F));
    }
    else
    {
        dest.push_back(0xF0 | ((u >> 18) & 0xFF));
        dest.push_back(0x80 | ((u >> 12) & 0x3F));
        dest.push_back(0x80 | ((u >> 6) & 0x3F));
        dest.push_back(0x80 | (u & 0x3F));
    }
}

inline uint64_t JsonParser::_parseHex4()
{
    assert(m_pos != m_buffer.end());
    uint64_t u = 0;
    auto p = m_pos;
    for (; m_pos != m_buffer.end() && m_pos < p + 4; ++m_pos)
    {
        if (m_pos == m_buffer.end())
        {
            throw JsonParseExcept(JSON_UNEXPECTED_END_OF_STRING, _pos(), m_line, _col());
        }
        u <<= 4;
        if (*m_pos >= '0' && *m_pos <= '9')
            u |= *m_pos - '0';
        else if (*m_pos >= 'A' && *m_pos <= 'F')
            u |= *m_pos - 'A' + 10;
        else if (*m_pos >= 'a' && *m_pos <= 'f')
            u |= *m_pos - 'a' + 10;
        else
        {
            throw JsonParseExcept(JSON_INVALID_UNICODE_HEX, _pos(), m_line, _col());
        }
    }
    return u;
}

inline void JsonParser::_parseEscapeSequence(std::string &dest)
{
    assert((m_pos != m_buffer.end()) && (*m_pos == '\\'));
    ++m_pos;
    if (m_pos == m_buffer.end())
    {
        throw JsonParseExcept(JSON_UNEXPECTED_END_OF_STRING, _pos(), m_line, _col());
    }
    switch (*m_pos)
    {
    case '"':
    case '\\':
    case '/':
        dest.push_back(*m_pos);
        ++m_pos;
        break;
    case 'b':
        dest.push_back('\b');
        ++m_pos;
        break;
    case 'f':
        dest.push_back('\f');
        ++m_pos;
        break;
    case 'n':
        dest.push_back('\n');
        ++m_pos;
        break;
    case 'r':
        dest.push_back('\r');
        ++m_pos;
        break;
    case 't':
        dest.push_back('\t');
        ++m_pos;
        break;
    case 'u':
    {
        ++m_pos;
        uint64_t u = _parseHex4();
        if (u >= 0xD800 && u <= 0xDBFF)
        {
            if (*m_pos != '\\' ||
                *(m_pos + 1) != 'u')
            {
                throw JsonParseExcept(JSON_INVALID_UNICODE_SURROGATE, _pos(), m_line, _col());
            }
            m_pos += 2;
            uint64_t l = _parseHex4();
            if (l < 0xDC00 || l > 0xDFFF)
            {
                throw JsonParseExcept(JSON_INVALID_UNICODE_SURROGATE, _pos(), m_line, _col());
            }
            u = (((u - 0xD800) << 10) | (l - 0xDC00)) +
                0x10000;
        }
        else if (u > 0xDBFF && u <= 0xDFFF)
        {
            throw JsonParseExcept(JSON_INVALID_UNICODE_SURROGATE, _pos(), m_line, _col());
        }
        _encodeUTF8(dest, u);
    }
    break;
    default:
        throw JsonParseExcept(JSON_INVALID_STRING_ESCAPE_SEQUENCE, _pos(), m_line, _col());
        ;
    }
}

inline void JsonParser::_parseUTF8(std::string &dest)
{
    assert(m_pos != m_buffer.end());
    uint8_t u = static_cast<uint8_t>(*m_pos), byteCount = 0;
    if ((u <= 0x7F))
        byteCount = 1;
    else if ((u >= 0xC0) && (u <= 0xDF))
        byteCount = 2;
    else if ((u >= 0xE0) && (u <= 0xEF))
        byteCount = 3;
    else if ((u >= 0xF0) && (u <= 0xF7))
        byteCount = 4;
    else
        throw JsonParseExcept(JSON_INVALID_UTF8_CHAR, _pos(), m_line, _col());

    dest.push_back(*m_pos);
    ++m_pos;
    for (auto p0 = m_pos; m_pos != m_buffer.end() && m_pos < p0 + byteCount - 1; ++m_pos)
    {
        u = static_cast<uint8_t>(*m_pos);
        if ((u >= 0x80) && (u <= 0xBF))
            dest.push_back(*m_pos);
        else
            throw JsonParseExcept(JSON_INVALID_UTF8_CHAR, _pos(), m_line, _col());
    }
}

inline JsonStr_t JsonParser::_parseString()
{
    assert((m_pos != m_buffer.end()) && (*m_pos == '"'));
    ++m_pos;
    JsonStr_t str;
    while (m_pos != m_buffer.end() && (*m_pos != '"'))
    {
        switch (*m_pos)
        {
        case '\\':
            _parseEscapeSequence(str);
            break;
        default:
            if (static_cast<uint8_t>(*m_pos) < 0x20)
            {
                throw JsonParseExcept(JSON_INVALID_STRING_CHAR, _pos(), m_line, _col());
            }
            _parseUTF8(str);
        }
    }
    ++m_pos;
    return str;
}

inline JsonNode JsonParser::_parseNumber()
{
    assert(m_pos != m_buffer.end());

    auto p = m_pos;
    p += (*p == '-');
    if (p == m_buffer.end())
        throw JsonParseExcept{JSON_INVALID_NUMBER, _pos(), m_line, _col()};
    if (*p == '0')
    {
        ++p;
    }
    else
    {
        if (!std::isdigit(*p) || *p == '0')
            throw JsonParseExcept{JSON_INVALID_NUMBER, _pos(), m_line, _col()};
        for (; p != m_buffer.end() && std::isdigit(*p); ++p)
            ;
    }
    if (p != m_buffer.end() && *p == '.')
    {
        ++p;
        if (p == m_buffer.end() || !std::isdigit(*p))
            throw JsonParseExcept{JSON_INVALID_NUMBER, _pos(), m_line, _col()};
        for (; p != m_buffer.end() && std::isdigit(*p); ++p)
            ;
    }
    if (p != m_buffer.end() && (*p == 'e' || *p == 'E'))
    {
        ++p;
        if (p == m_buffer.end())
            throw JsonParseExcept{JSON_INVALID_NUMBER, _pos(), m_line, _col()};
        if (*p == '+' || *p == '-')
            ++p;
        if (p == m_buffer.end())
            throw JsonParseExcept{JSON_INVALID_NUMBER, _pos(), m_line, _col()};
        for (; p != m_buffer.end() && std::isdigit(*p); ++p)
            ;
    }

    JsonNum_t ret;
    size_t idx = 0;
    const std::string &numStr = m_buffer.substr(_pos(), p - m_pos);
    try
    {
#ifndef JSON_PARSER_USE_FLOAT
        ret = std::stod(numStr, &idx);
#else
        ret = std::stof(numStr, &idx);
#endif
    }
    catch (const std::invalid_argument &)
    {
        throw JsonParseExcept(JSON_INVALID_NUMBER, _pos(), m_line, _col());
    }
    catch (const std::out_of_range &)
    {
        throw JsonParseExcept(JSON_NUMBER_OUT_OF_RANGE, _pos(), m_line, _col());
    }
    m_pos += idx;
    return ret;
}

inline JsonNode JsonParser::_parseLiteral()
{
    assert((m_pos != m_buffer.end()) && (*m_pos == 'n' || *m_pos == 't' || *m_pos == 'f'));
    if (m_buffer.end() - m_pos < 4)
    {
        throw JsonParseExcept(JSON_INVALID_LITERAL, _pos(), m_line, _col());
    }
    if (*m_pos == 'n' && *(m_pos + 1) == 'u' && *(m_pos + 2) == 'l' && *(m_pos + 3) == 'l')
    {
        m_pos += 4;
        return JsonNull;
    }
    if (*m_pos == 't' && *(m_pos + 1) == 'r' && *(m_pos + 2) == 'u' && *(m_pos + 3) == 'e')
    {
        m_pos += 4;
        return JsonBool_t(true);
    }
    if (m_buffer.end() - m_pos < 5)
    {
        throw JsonParseExcept(JSON_INVALID_LITERAL, _pos(), m_line, _col());
    }
    if (*m_pos == 'f' && *(m_pos + 1) == 'a' && *(m_pos + 2) == 'l' && *(m_pos + 3) == 's' && *(m_pos + 4) == 'e')
    {
        m_pos += 5;
        return JsonBool_t(false);
    }
    throw JsonParseExcept(JSON_INVALID_LITERAL, _pos(), m_line, _col());
}

inline void JsonParser::_skipSpace()
{
    while (m_pos != m_buffer.end() && ((*m_pos == ' ') ||
                                       (*m_pos == '\t') ||
                                       (*m_pos == '\n') ||
                                       (*m_pos == '\r')))
    {
        if (*m_pos == '\r' || *m_pos == '\n')
        {
            ++m_line;
            if ((*m_pos == '\r') && (m_pos + 1 != m_buffer.end()) && *(m_pos + 1) == '\n') // windows new line
                ++m_pos;
            m_lineLen = _pos();
        }
        ++m_pos;
    }
}

#endif