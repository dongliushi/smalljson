#pragma once

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
  void print();

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
  Object(std::initializer_list<object_t::value_type> init_list)
      : object_data_(init_list) {}
  Object &operator=(const Object &rhs) = default;
  Object &operator=(Object &&rhs) = default;
  void print();

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
  Array(std::initializer_list<array_t::value_type> init_list)
      : array_data_(init_list) {}
  Array &operator=(const Array &rhs) = default;
  Array &operator=(Array &&rhs) = default;

  void print();

private:
  array_t array_data_;
};

} // namespace smalljson