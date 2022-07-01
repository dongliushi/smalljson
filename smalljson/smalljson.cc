#include "smalljson.h"
#include <cassert>
#include <iostream>

namespace smalljson {
std::string escapeJson(const std::string &str);

std::string unescapeJson(const std::string &str);

Value::Value(const Array &arr)
    : type_(ValueType::Array), value_data_(std::make_unique<Array>(arr)) {}

Value::Value(Array &&arr)
    : type_(ValueType::Array),
      value_data_(std::make_unique<Array>(std::move(arr))) {}

Value::Value(const Object &obj)
    : type_(ValueType::Object), value_data_(std::make_unique<Object>(obj)) {}

Value::Value(Object &&obj)
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

Value &Value::operator[](size_t idx) { return to_array()[idx]; }

Value &Value::operator[](const std::string &key) { return to_object()[key]; }

const std::string Value::to_string() const {
  switch (type_) {
  case ValueType::Null:
    return "null";
  case ValueType::Boolean:
  case ValueType::Number:
    return std::get<std::string>(value_data_);
  case ValueType::String:
    return "\"" + std::get<std::string>(value_data_) + "\"";
  case ValueType::Object:
    return std::get<object_ptr>(value_data_)->to_string();
  case ValueType::Array:
    return std::get<array_ptr>(value_data_)->to_string();
  default:
    throw Exception(Exception::ParseError::NOT_JSON);
  }
}

Array &Value::to_array() {
  if (isArray()) {
    return *std::get<array_ptr>(value_data_);
  }
  throw Exception(Exception::ParseError::BAD_TYPE);
}

Object &Value::to_object() {
  if (isObject()) {
    return *std::get<object_ptr>(value_data_);
  }
  throw Exception(Exception::ParseError::BAD_TYPE);
}

const Array &Value::to_array() const {
  if (isArray()) {
    return *std::get<array_ptr>(value_data_);
  }
  throw Exception(Exception::ParseError::BAD_TYPE);
}

const Object &Value::to_object() const {
  if (isObject()) {
    return *std::get<object_ptr>(value_data_);
  }
  throw Exception(Exception::ParseError::BAD_TYPE);
}

Value &Value::at(size_t idx) { return to_array().at(idx); }

Value &Value::at(const std::string &key) { return to_object().at(key); }

const Value &Value::at(size_t idx) const { return to_array().at(idx); }

const Value &Value::at(const std::string &key) const {
  return to_object().at(key);
}

const std::string Object::to_string() const {
  std::string str = "{";
  for (auto &[key, value] : object_data_) {
    str += "\"" + escapeJson(key) + "\":" + value.to_string() + ",";
  }
  str.pop_back();
  str += "}";
  return str;
}

Value &Object::operator[](const std::string &key) { return object_data_[key]; }

Value &Object::operator[](std::string &&key) {
  return object_data_[std::move(key)];
}

const std::string Array::to_string() const {
  std::string str = "[";
  for (auto &value : array_data_) {
    str += value.to_string() + ",";
  }
  str.pop_back();
  str += "]";
  return str;
}

Value &Array::operator[](size_t idx) { return array_data_[idx]; }

Value Parser::parseStart() {
  skipWhiteSpace();
  Value ret;
  switch (*cur_) {
  case '{':
    ret = parseObject();
    break;
  case '[':
    ret = parseArray();
    break;
  default:
    throw Exception(Exception::ParseError::NOT_JSON);
    break;
  }
  skipWhiteSpace();
  if (*cur_ != '\0')
    throw Exception(Exception::ParseError::ROOT_NOT_ONE);
  return ret;
}

void Parser::skipWhiteSpace() {
  while (cur_ != end_ &&
         (*cur_ == ' ' || *cur_ == '\n' || *cur_ == '\r' || *cur_ == '\t'))
    cur_++;
}

void Parser::skipDigit() {
  while (cur_ != end_ && std::isdigit(*cur_))
    cur_++;
}

Value Parser::parseObject() {
  assert(*cur_ == '{');
  cur_++;
  skipWhiteSpace();
  if (*cur_ == '}') {
    cur_++;
    return Object();
  }
  Object::object_t object_data;
  while (true) {
    skipWhiteSpace();
    std::string key = parseRawString();
    skipWhiteSpace();
    if (*cur_ != ':')
      throw Exception(Exception::ParseError::MISS_COLON);
    cur_++;
    skipWhiteSpace();
    Value value = parseValue();
    skipWhiteSpace();
    object_data.emplace(unescapeJson(key), std::move(value));
    if (*cur_ == ',') {
      cur_++;
    } else if (*cur_ == '}') {
      cur_++;
      break;
    } else {
      throw Exception(Exception::ParseError::LACK_COMMA_OR_BRACE);
    }
  }
  return Object(std::move(object_data));
}

Value Parser::parseArray() {
  assert(*cur_ == '[');
  cur_++;
  skipWhiteSpace();
  if (*cur_ == ']') {
    cur_++;
    return Array();
  }
  Array::array_t array_data;
  while (true) {
    skipWhiteSpace();
    Value value = parseValue();
    skipWhiteSpace();
    array_data.emplace_back(std::move(value));
    if (*cur_ == ',') {
      cur_++;
    } else if (*cur_ == ']') {
      cur_++;
      break;
    } else {
      throw Exception(Exception::ParseError::LACK_COMMA_OR_BRACKET);
    }
  }
  return Array(std::move(array_data));
}

Value Parser::parseValue() {
  switch (*cur_) {
  case 't':
  case 'f':
    return parseBoolean();
  case 'n':
    return parseNull();
  case '"':
    return parseString();
  case '[':
    return parseArray();
  case '{':
    return parseObject();
  default:
    break;
  }
  if (*cur_ == '-' || std::isdigit(*cur_)) {
    return parseNumber();
  }
  throw Exception(Exception::ParseError::BAD_VALUE);
  return nullptr;
}

std::string Parser::parseRawString() {
  if (*cur_ != '"') {
    throw Exception(Exception::ParseError::BAD_KEY);
  }
  cur_++;
  iterator_t old_cur = cur_;
  while (cur_ != end_) {
    if (*cur_ == '\\') {
      cur_++;
      if (cur_ == end_)
        throw Exception(Exception::ParseError::BAD_ESCAPE);
      switch (*cur_) {
      case '"':
      case '\\':
      case '/':
      case 't':
      case 'r':
      case 'n':
      case 'u':
      case 'b':
      case 'f':
        cur_++;
        break;
      default:
        throw Exception(Exception::ParseError::BAD_ESCAPE);
        break;
      }
    } else if (*cur_ == '"') {
      cur_++;
      break;
    } else {
      cur_++;
    }
  }
  if (cur_ == end_) {
    throw Exception(Exception::ParseError::JSON_LENGTH);
  }
  return std::string(old_cur, cur_ - 1);
}

Value Parser::parseString() {
  std::string str = parseRawString();
  return Value(Value::ValueType::String, std::move(str));
}

Value Parser::parseBoolean() {
  assert(*cur_ == 't' || *cur_ == 'f');
  std::string str;
  switch (*cur_) {
  case 't':
    str.assign(cur_, cur_ + 4 <= end_ ? cur_ + 4 : end_);
    if (str == "true") {
      cur_ += 4;
      return Value(Value::ValueType::Boolean, std::move(str));
    }
    break;
  case 'f':
    str.assign(cur_, cur_ + 5 <= end_ ? cur_ + 5 : end_);
    if (str == "false") {
      cur_ += 5;
      return Value(Value::ValueType::Boolean, std::move(str));
    }
    break;
  default:
    break;
  }
  throw Exception(Exception::ParseError::BAD_BOOLEAN);
  return nullptr;
}

Value Parser::parseNull() {
  assert(*cur_ == 'n');
  std::string str(cur_, cur_ + 4 <= end_ ? cur_ + 4 : end_);
  if (str == "null") {
    cur_ += 4;
    return Value();
  }
  throw Exception(Exception::ParseError::BAD_NULL);
  return nullptr;
}

Value Parser::parseNumber() {
  iterator_t old_cur = cur_;
  if (*cur_ == '-') {
    cur_++;
  }
  if (!std::isdigit(*cur_)) {
    throw Exception(Exception::ParseError::BAD_NUMBER);
  }
  if (cur_ != end_ && *cur_ == '0' && cur_ + 1 != end_ &&
      std::isdigit(*(cur_ + 1))) {
    throw Exception(Exception::ParseError::BAD_NUMBER);
  }
  skipDigit();
  switch (*cur_) {
  case '.':
    cur_++;
    skipDigit();
    break;
  case 'e':
  case 'E':
    cur_++;
    if (cur_ == end_) {
      throw Exception(Exception::ParseError::BAD_NUMBER);
    }
    if (*cur_ == '+' || *cur_ == '-') {
      cur_++;
    }
    if (!std::isdigit(*cur_)) {
      throw Exception(Exception::ParseError::BAD_NUMBER);
    }
    skipDigit();
  default:
    break;
  }
  return Value(Value::ValueType::Number, std::string(old_cur, cur_));
}

std::string escapeJson(const std::string &str) {
  std::string escapeStr(str);
  std::string replaceStr[7] = {R"(\n)", R"(\t)", R"(\\)", R"(\b)",
                               R"(\r)", R"(\")", R"(\f)"};
  for (size_t idx = 0; idx < escapeStr.size(); idx++) {
    size_t replaceIdx = 0;
    switch (escapeStr[idx]) {
    case '\n':
      replaceIdx = 0;
      break;
    case '\t':
      replaceIdx = 1;
      break;
    case '\\':
      replaceIdx = 2;
      break;
    case '\b':
      replaceIdx = 3;
      break;
    case '\r':
      replaceIdx = 4;
      break;
    case '\"':
      replaceIdx = 5;
      break;
    case '\f':
      replaceIdx = 6;
    default:
      continue;
    }
    escapeStr.replace(idx, 1, replaceStr[replaceIdx]);
    idx++;
  }
  return escapeStr;
}

std::string unescapeJson(const std::string &str) {
  std::string unescapeStr(str);
  std::string replaceStr[7] = {"\n", "\t", "\\", "\b", "\r", "\"", "\f"};
  for (size_t idx = 0; idx < unescapeStr.size(); idx++) {
    if (unescapeStr[idx] != '\\')
      continue;
    size_t replaceIdx = 0;
    switch (unescapeStr[idx + 1]) {
    case 'n':
      replaceIdx = 0;
      break;
    case 't':
      replaceIdx = 1;
      break;
    case '\\':
      replaceIdx = 2;
      break;
    case 'b':
      replaceIdx = 3;
      break;
    case 'r':
      replaceIdx = 4;
      break;
    case '"':
      replaceIdx = 5;
      break;
    case 'f':
      replaceIdx = 6;
    default:
      break;
    }
    unescapeStr.replace(idx, 2, replaceStr[replaceIdx]);
  }
  return unescapeStr;
}

const char *Exception::errorToStr() const {
  switch (err_) {
  case ParseError::NOT_JSON:
    return "format not json";
  case ParseError::ROOT_NOT_ONE:
    return "root not one";
  case ParseError::MISS_COLON:
    return "miss colon";
  case ParseError::MISS_VALUE:
    return "miss value";
  case ParseError::LACK_COMMA_OR_BRACE:
    return "lack ',' or '}'";
  case ParseError::LACK_COMMA_OR_BRACKET:
    return "lack ',' or ']'";
  case ParseError::BAD_KEY:
    return "bad key";
  case ParseError::BAD_VALUE:
    return "bad value";
  case ParseError::JSON_LENGTH:
    return "json format ";
  case ParseError::BAD_ESCAPE:
    return "bad escape";
  case ParseError::BAD_BOOLEAN:
    return "bad boolean";
  case ParseError::BAD_NULL:
    return "bad null";
  case ParseError::BAD_NUMBER:
    return "bad number";
  case ParseError::BAD_TYPE:
    return "bad type";
  default:
    return "other error";
  }
}

} // namespace smalljson