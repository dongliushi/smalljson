#include "smalljson.h"
#include <iostream>

namespace smalljson {
Value::Value(Array arr)
    : type_(ValueType::Array),
      value_data_(std::make_unique<Array>(std::move(arr))) {}
Value::Value(Object obj)
    : type_(ValueType::Object),
      value_data_(std::make_unique<Object>(std::move(obj))) {}

Value &Value::operator=(const Value &rhs) {
  type_ = rhs.type_;
  deepCopy(rhs.value_data_);
  return *this;
}

void Value::deepCopy(const Value::value_t &rhs) {
  if (auto pval = std::get_if<std::string>(&rhs)) {
    value_data_ = *pval;
  } else if (auto pval = std::get_if<object_ptr>(&rhs)) {
    value_data_ = std::make_unique<Object>(**pval);
  } else if (auto pval = std::get_if<array_ptr>(&rhs)) {
    value_data_ = std::make_unique<Array>(**pval);
  }
}

void Value::print() {
  if (auto pval = std::get_if<std::string>(&value_data_)) {
    std::cout << *pval << std::endl;
    value_data_ = *pval;
  } else if (auto pval = std::get_if<object_ptr>(&value_data_)) {
    std::get<object_ptr>(value_data_)->print();
  } else if (auto pval = std::get_if<array_ptr>(&value_data_)) {
    std::get<array_ptr>(value_data_)->print();
  }
}

void Object::print() {
  for (auto &x : object_data_) {
    x.second.print();
  }
}

void Array::print() {
  for (auto &x : array_data_) {
    x.print();
  }
}

} // namespace smalljson