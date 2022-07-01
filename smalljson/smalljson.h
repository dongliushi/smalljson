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
  ~Value() = default;
  Value(size_t num)
      : type_(ValueType::Number), value_data_(std::to_string(num)) {}
  Value(ssize_t num)
      : type_(ValueType::Number), value_data_(std::to_string(num)) {}
  Value(float num)
      : type_(ValueType::Number), value_data_(std::to_string(num)) {}
  Value(double num)
      : type_(ValueType::Number), value_data_(std::to_string(num)) {}
  Value(bool boolean)
      : type_(ValueType::Boolean), value_data_(boolean ? "true" : "false") {}
  Value(const std::string &str)
      : type_(ValueType::String), value_data_(std::move(str)) {}
  Value(const char *str) : type_(ValueType::String), value_data_(str) {}
  Value(const Value &rhs) : type_(rhs.type_) { deepCopy(rhs.value_data_); }
  Value(Value &&rhs) noexcept = default;
  Value(const Object &obj);
  Value(Object&& obj);
  Value(const Array& arr);
  Value(Array&& arr);
  Value &operator=(const Value &rhs);
  Value &operator=(Value &&rhs) = default;
  Value &operator[](size_t idx);
  Value &operator[](const std::string &key);

public:
  bool isNull() const noexcept { return type_ == ValueType::Null; }
  bool isNumber() const noexcept { return type_ == ValueType::Number; }
  bool isArray() const noexcept { return type_ == ValueType::Array; }
  bool isObject() const noexcept { return type_ == ValueType::Object; }
  bool isString() const noexcept { return type_ == ValueType::String; }
  bool isBoolean() const noexcept { return type_ == ValueType::Boolean; }
  ValueType type() const noexcept { return type_; }
  const std::string to_string() const;
  Array &to_array();
  const Array &to_array() const;
  Object &to_object();
  const Object &to_object() const;
  Value &at(size_t idx);
  Value &at(const std::string &key);
  const Value &at(size_t idx) const;
  const Value &at(const std::string &key) const;

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
  typedef object_t::iterator iterator;
  typedef object_t::const_iterator const_iterator;
  typedef object_t::reverse_iterator reverse_iterator;
  typedef object_t::const_reverse_iterator const_reverse_iterator;

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
  Value &operator[](std::string &&key);
  iterator begin() noexcept { return object_data_.begin(); }
  iterator end() noexcept { return object_data_.end(); }
  const_iterator begin() const noexcept { return object_data_.begin(); }
  const_iterator end() const noexcept { return object_data_.end(); }
  const_iterator cbegin() const noexcept { return object_data_.cbegin(); }
  const_iterator cend() const noexcept { return object_data_.cend(); }
  reverse_iterator rbegin() noexcept { return object_data_.rbegin(); }
  reverse_iterator rend() noexcept { return object_data_.rend(); }
  const_reverse_iterator rbegin() const noexcept {
    return object_data_.rbegin();
  }
  const_reverse_iterator rend() const noexcept { return object_data_.rend(); }
  const_reverse_iterator crbegin() const noexcept {
    return object_data_.crbegin();
  }
  const_reverse_iterator crend() const noexcept { return object_data_.crend(); }
  iterator find(const std::string &key) { return object_data_.find(key); }
  const_iterator find(const std::string &key) const {
    return object_data_.find(key);
  }
  object_t::mapped_type &at(const std::string &key) {
    return object_data_.at(key);
  }
  const object_t::mapped_type &at(const std::string &key) const {
    return object_data_.at(key);
  }
  object_t::size_type erase(const std::string &key) {
    return object_data_.erase(key);
  }
  iterator erase(iterator pos) { return object_data_.erase(pos); }
  const_iterator erase(const_iterator pos) { return object_data_.erase(pos); }
  iterator erase(const_iterator first, const_iterator last) {
    return object_data_.erase(first, last);
  }
  bool empty() const noexcept { return object_data_.empty(); }
  void clear() noexcept { object_data_.clear(); }

public:
  const std::string to_string() const;

private:
  object_t object_data_;
};

class Array {
public:
  typedef std::vector<Value> array_t;
  typedef array_t::iterator iterator;
  typedef array_t::const_iterator const_iterator;
  typedef array_t::reverse_iterator reverse_iterator;
  typedef array_t::const_reverse_iterator const_reverse_iterator;

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
  iterator begin() noexcept { return array_data_.begin(); }
  iterator end() noexcept { return array_data_.end(); }
  const_iterator begin() const noexcept { return array_data_.begin(); }
  const_iterator end() const noexcept { return array_data_.end(); }
  const_iterator cbegin() const noexcept { return array_data_.cbegin(); }
  const_iterator cend() const noexcept { return array_data_.cend(); }
  reverse_iterator rbegin() noexcept { return array_data_.rbegin(); }
  reverse_iterator rend() noexcept { return array_data_.rend(); }
  const_reverse_iterator rbegin() const noexcept {
    return array_data_.rbegin();
  }
  const_reverse_iterator rend() const noexcept { return array_data_.rend(); }
  const_reverse_iterator crbegin() const noexcept {
    return array_data_.crbegin();
  }
  const_reverse_iterator crend() const noexcept { return array_data_.crend(); }
  array_t::reference at(size_t idx) { return array_data_.at(idx); }
  array_t::const_reference at(size_t idx) const { return array_data_.at(idx); }
  iterator erase(iterator pos) { return array_data_.erase(pos); }
  const_iterator erase(const_iterator pos) { return array_data_.erase(pos); }
  iterator erase(const_iterator first, const_iterator last) {
    return array_data_.erase(first, last);
  }
  bool empty() const noexcept { return array_data_.empty(); }
  void clear() noexcept { array_data_.clear(); }

public:
  const std::string to_string() const;

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
} // namespace smalljson