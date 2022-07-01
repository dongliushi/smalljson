#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace smalljson {
class Array;
class Object;

class Value {
public:
  typedef std::unique_ptr<Object> object_ptr;
  typedef std::unique_ptr<Array> array_ptr;
  typedef std::variant<std::string, array_ptr, object_ptr> value_t;

  enum class ValueType : unsigned {
    Array,
    Object,
    Null,
    Boolean,
    String,
    Number,
  };

public:
  Value() : type_(ValueType::Null) {}
  Value(size_t num)
      : type_(ValueType::Number), value_data_(std::to_string(num)) {}
  Value(ssize_t num)
      : type_(ValueType::Number), value_data_(std::to_string(num)) {}
  Value(float num)
      : type_(ValueType::Number), value_data_(std::to_string(num)) {}
  Value(double num)
      : type_(ValueType::Number), value_data_(std::to_string(num)) {}
  Value(const std::string &str)
      : type_(ValueType::String), value_data_(std::move(str)) {}
  Value(const char *str) : type_(ValueType::String), value_data_(str) {}
  Value(const Value &rhs) : type_(rhs.type_) { deepCopy(rhs.value_data_); }
  Value(Value &&rhs) noexcept = default;
  Value(Object obj);
  Value(Array arr);
  Value &operator=(const Value &rhs);
  Value &operator=(Value &&rhs) = default;
  Value &operator[](size_t idx);
  Value &operator[](const std::string &key);

public:
  bool isNull();
  bool isNumber();
  bool isArray();
  bool isObject();
  bool isString();
  bool isBoolean();
  std::string to_string();
  Array &to_array();
  Object &to_object();
  template <typename... Args>
  Value(ValueType type, Args &&...args)
      : type_(type), value_data_(std::forward<Args>(args)...) {
    static_assert(std::is_constructible<value_t, Args...>::value,
                  "template args error");
  }

private:
  void deepCopy(const value_t &rhs);
  ValueType type_;
  value_t value_data_;
};

class Object {
public:
  typedef std::map<std::string, Value> object_t;

public:
  Object() = default;
  Object(const Object &rhs) = default;
  Object(Object &&rhs) noexcept = default;
  Object(const object_t &object_data) : object_data_(object_data) {}
  Object(object_t &&object_data) : object_data_(std::move(object_data)) {}
  Object(std::initializer_list<object_t::value_type> init_list)
      : object_data_(init_list) {}
  Object &operator=(const Object &rhs) = default;
  Object &operator=(Object &&rhs) = default;
  Value &operator[](const std::string &key);

public:
  std::string to_string();

private:
  object_t object_data_;
};

class Array {
public:
  typedef std::vector<Value> array_t;

public:
  Array() = default;
  Array(const Array &rhs) = default;
  Array(Array &&rhs) noexcept = default;
  Array(const array_t &array_data) : array_data_(array_data) {}
  Array(array_t &&array_data) : array_data_(std::move(array_data)) {}
  Array(std::initializer_list<array_t::value_type> init_list)
      : array_data_(init_list) {}
  Array &operator=(const Array &rhs) = default;
  Array &operator=(Array &&rhs) = default;
  Value &operator[](size_t idx);

public:
  std::string to_string();

private:
  array_t array_data_;
};

class Parser {
public:
  typedef std::string::const_iterator iterator_t;
  static Value parse(const std::string &json_data) {
    return Parser(json_data).parseStart();
  }

private:
  Parser(const std::string &json_data)
      : cur_(json_data.begin()), end_(json_data.end()) {}
  Value parseStart();
  Value parseObject();
  Value parseArray();
  Value parseValue();
  Value parseString();
  Value parseBoolean();
  Value parseNumber();
  Value parseNull();
  std::string parseRawString();
  void skipWhiteSpace();
  void skipDigit();

private:
  iterator_t cur_, end_;
};

class Exception : public std::exception {
public:
  enum class ParseError {
    NOT_JSON,
    ROOT_NOT_ONE,
    MISS_COLON,
    MISS_VALUE,
    LACK_COMMA_OR_BRACE,
    LACK_COMMA_OR_BRACKET,
    BAD_KEY,
    BAD_VALUE,
    JSON_LENGTH,
    BAD_ESCAPE,
    BAD_BOOLEAN,
    BAD_NULL,
    BAD_NUMBER,
    BAD_TYPE
  };
  explicit Exception(ParseError err) : err_(err) {}
  const char *what() const noexcept { return errorToStr(); }

private:
  const char *errorToStr() const;
  ParseError err_;
};

std::string escapeJson(const std::string &str);

std::string unescapeJson(const std::string &str);
} // namespace smalljson