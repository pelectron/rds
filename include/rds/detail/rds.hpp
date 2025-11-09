#ifndef RDS_DETAIL_RDS_HPP
#define RDS_DETAIL_RDS_HPP

#include "rds/common.hpp"
#include "rds/device.hpp"
#include "rds/value.hpp"

#include <functional>
#include <ryu/ryu.h>
#include <ryu/ryu_parse.h>
#include <stdexcept>

namespace rds {
struct Device;
namespace detail {

/**
 * A Deserializer turns file contents in form of a string into a map of
 * std::string to Value. There is one deserializer per file extension. See also
 * register_deserializer().
 */
using Deserializer = std::function<ValueMap(const std::string &s)>;

/**
 * A Serializer turns a device into a string. There is one serializer per file
 * extension. See also register_serializer().
 */
using Serializer = std::function<std::string(const ValueMap &map)>;
std::map<std::string, Serializer> &serializers();
std::map<std::string, Deserializer> &deserializers();
bool register_deserializer(const std::string &file_ext, Deserializer deser);

bool register_serializer(const std::string &file_ext, Serializer ser);

ValueMap deseralize_from_file(std::string_view file_path);

void seralize_to_file(std::string_view file_path, const ValueMap &map);

ValueMap to_map(const Device &d);

std::unique_ptr<Device> to_device(const ValueMap &map);

void do_derive(ValueMap &map, std::vector<ValueMap *> parents = {},
               std::string_view name = {});

ValueMap derive(ValueMap &&map);

std::unique_ptr<Device> sort(std::unique_ptr<Device> &&device);

std::unique_ptr<Device> validate(std::unique_ptr<Device> &&d);

/**
 * @brief parsers a Value from a string
 */
class Parser {
public:
  Parser() = default;

  /**
   * @brief parses a Value or throws a std::runtime_error.
   *
   * @param str the file the contents to parse
   * @param file_path an optional file path
   */
  rds::Value parse_or_throw(std::string_view str,
                            std::string_view file_path = {}) {
    if (file_path.empty()) {
      filename_.clear();
    } else {
      filename_ = std::string{file_path.data(), file_path.size()};
      filename_ += ':';
    }
    lex(str);
    return parse();
  }

  std::string_view input() const { return input_; }

  std::string_view filename() const {
    return filename_.empty()
               ? std::string_view{}
               : std::string_view{filename_.data(), filename_.size() - 1};
  }

  std::size_t line_count() const { return line_; }

  std::size_t column_count() const { return column_; }

private:
  template <typename... T> struct ol : T... {
    using T::operator()...;
  };

  enum class TokenType {
    o_brace,
    c_brace,
    o_bracket,
    c_bracket,
    colon,
    comma,
    identifier,
    string,
    signed_int,
    unsigned_int,
    floating_point,
    boolean
  };

  struct Location {
    std::size_t line;
    std::size_t column;
  };

  struct Source {
    Location begin;
    Location end;
  };

  struct Token {
    TokenType type;
    Source source;
    std::variant<std::monostate, char, bool, double, uint64_t, int64_t,
                 std::string_view>
        value;

    std::string value_as_string() const {
      return std::visit(
          ol{[](std::monostate) -> std::string { return "no value"; },
             [](char c) -> std::string { return std::string("'") + c + '\''; },
             [](const std::string_view &s) -> std::string {
               return '\'' +
                      (s.empty() ? "" : std::string(s.data(), s.size())) + '\'';
             },
             [](double v) {
               std::string res(std::size_t{25}, 0);
               res.resize(
                   static_cast<std::size_t>(d2s_buffered_n(v, res.data())));
               return res;
             },
             [](const auto &v) { return std::to_string(v); }},
          value);
    }
  };

  std::vector<Token> tokens_{};
  std::string input_{};
  std::string filename_{};
  const char *pos_ = nullptr;
  const char *end_ = nullptr;
  std::size_t tok_index_ = 0;
  std::size_t line_ = 0;
  std::size_t column_ = 0;

  static std::string_view to_string(TokenType t) {
    using enum TokenType;
    switch (t) {
    case c_brace:
      return "c_brace";
    case o_brace:
      return "o_brace";
    case o_bracket:
      return "o_bracket";
    case c_bracket:
      return "c_bracket";
    case colon:
      return "colon";
    case comma:
      return "comma";
    case identifier:
      return "identifier";
    case string:
      return "string";
    case signed_int:
      return "signed_int";
    case unsigned_int:
      return "unsigned_int";
    case floating_point:
      return "floating_point";
    case boolean:
      return "boolean";
    default:
      return {};
    }
  }

  /// returns true if c i a lower or uppercase letter, i.e. 'a' to 'z' or 'A' to
  /// 'Z'.
  bool is_letter(char c) {
    return (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z');
  }

  /// returns true if c is a number, i.e. '0' to '9'
  bool is_number(char c) { return (c >= '0' and c <= '9'); }
  bool is_hex_digit(char c) {
    return (c >= '0' and c <= '9') or (c >= 'a' and c <= 'f') or
           (c >= 'A' and c <= 'F');
  }
  uint64_t hex_value(char c) {
    return (c >= '0' and c <= '9')   ? static_cast<uint64_t>(c - '0')
           : (c >= 'a' and c <= 'f') ? static_cast<uint64_t>(c - 'a')
                                     : static_cast<uint64_t>(c - 'A');
  }

  bool is_identfier_char(char c) {
    return is_letter(c) or is_number(c) or c == '-' or c == '_';
  }

  std::variant<double, uint64_t, int64_t> consume_number() {
    assert(pos_);
    assert(end_);
    assert(pos_ != end_);

    if (*pos_ == '-') {
      ++pos_;
      ++column_;
      if (pos_ == end_) {
        throw std::runtime_error(std::format(
            "Error {}{}{}: expected integer after '-', got end of file",
            filename_, line_, column_ - 1));
      }

      if (not is_number(*pos_)) {
        throw std::runtime_error(
            std::format("Error {}{}{}: expected integer after '-', got '{}'",
                        filename_, line_, column_, *pos_));
      }

      auto value = consume_positive_number();
      std::visit(ol{[&value, this](uint64_t v) {
                      if (v > std::numeric_limits<uint64_t>::max() / 2) {
                        throw std::runtime_error(
                            std::format("Error {}{}{}: overflow in negative "
                                        "integer '-', -{} is too small",
                                        filename_, line_, column_, v));
                      }
                      value.emplace<int64_t>(-static_cast<int64_t>(v));
                    },
                    [&value](int64_t v) { value.emplace<int64_t>(-v); },
                    [&value](double v) { value.emplace<double>(-v); }},
                 value);

      return value;
    } else {
      return consume_positive_number();
    }
  }

  uint64_t consume_hex() {
    assert(pos_);
    assert(end_);
    assert(pos_ != end_);
    std::uint64_t value = 0;
    while (pos_ != end_) {
      const auto old = value;
      const char c = *pos_;
      if (c >= 'a' and c <= 'f') {
        value = 16 * value + (c - 'a');
      } else if (c >= 'A' and c < 'F') {
        value = 16 * value + (c - 'A');
      } else if (c >= '0' and c <= '9') {
        value = 16 * value + (c - '0');
      } else {
        return value;
      }
      if (old > value) {
        throw std::runtime_error(
            std::format("Error {}{}{}: overflow in binary "
                        "integer while parsing, went from {} to {}",
                        filename_, line_, column_, old, value));
      }
      ++pos_;
      ++column_;
    }
    return value;
  }

  uint64_t consume_binary() {
    assert(pos_);
    assert(end_);
    assert(pos_ != end_);
    std::uint64_t value = *pos_ - '0';
    while (pos_ != end_) {
      const auto old = value;
      if (*pos_ == '0') {
        value = 2u * value;
      } else if (*pos_ == '1') {
        value = 2u * value + 1u;
      } else {
        return value;
      }
      if (old > value) {
        throw std::runtime_error(
            std::format("Error {}{}{}: overflow in binary "
                        "integer while parsing, went from {} to {}",
                        filename_, line_, column_, old, value));
      }
      ++pos_;
      ++column_;
    }
    return value;
  }

  std::variant<double, uint64_t, int64_t> consume_positive_number() {
    assert(pos_);
    assert(end_);
    assert(pos_ != end_);
    assert(is_number(*pos_));
    const char *const begin = pos_;
    uint64_t int_value = *pos_ - '0';
    assert(int_value < 10);

    ++pos_;
    ++column_;
    if (pos_ == end_) {
      // integer value
      return int_value;
    }

    if (int_value == 0) {
      // first digit is 0
      if (*pos_ == 'x' or *pos_ == 'X') {
        ++pos_;
        ++column_;
        if (pos_ == end_) {
          throw std::runtime_error(
              std::format("Error {}{}{}: expected hex literal, got end of file",
                          filename_, line_, column_));
        }
        if (not is_hex_digit(*pos_)) {
          throw std::runtime_error(
              std::format("Error {}{}{}: expected hex digit, got '{}'",
                          filename_, line_, column_, *pos_));
        }
        auto val = consume_hex();
        return val;
      } else if (*pos_ == 'b' or *pos_ == 'B') {
        ++pos_;
        ++column_;
        if (pos_ == end_) {
          throw std::runtime_error(std::format(
              "Error {}{}{}: expected binary literal, got end of file",
              filename_, line_, column_));
        }
        if (not(*pos_ == '1' or *pos_ == '0')) {
          throw std::runtime_error(
              std::format("Error {}{}{}: expected binary digit, got '{}'",
                          filename_, line_, column_, *pos_));
        }
        auto val = consume_binary();
        return val;
      }
    }

    while (pos_ != end_ and is_number(*pos_)) {
      auto old = int_value;
      int_value = 10 * int_value + (*pos_ - '0');
      if (old > int_value) {
        // TODO: overflow
        throw std::runtime_error(std::format(
            "Error {}{}{}: {} is too big to fit into a 64 bit "
            "unsigned integer.",
            filename_, line_, column_, std::string_view{begin, pos_ + 1}));
      }
      ++pos_;
      ++column_;
    }

    if (pos_ == end_ or not(*pos_ == '.' or *pos_ == 'e' or *pos_ == 'E')) {
      return int_value;
    }

    if (*pos_ == '.') {
      ++pos_;
      ++column_;
      while (pos_ != end_ and is_number(*pos_)) {
        ++pos_;
        ++column_;
      }
    }

    if (pos_ != end_ and (*pos_ == 'e' or *pos_ == 'E')) {
      // *pos == e
      ++pos_;
      ++column_;
      if (pos_ == end_) {
        throw std::runtime_error(std::format(
            "Error {}{}{}: invalid floating point number, expected integer "
            "after "
            "'{}', got end of file",
            line_, column_, *(pos_ - 1), std::string_view{begin, pos_ + 1}));
      }

      if (*pos_ == '-') {
        ++pos_;
        ++column_;
        if (pos_ == end_) {
          throw std::runtime_error(std::format(
              "Error {}{}{}: {} is invalid floating point number, expected "
              "integer after '-', got end of file",
              line_, column_, *(pos_ - 1), std::string_view{begin, pos_ + 1}));
        }
      }

      int64_t exp = 0;
      while (pos_ != end_ and is_number(*pos_)) {
        auto old = exp;
        exp = 10 * exp + (*pos_ - '0');
        if (old > exp) {
          throw std::runtime_error(std::format(
              "Error {}{}{}: {} fractional is too big to fit into a 64 bit "
              "unsigned integer.",
              filename_, line_, column_, std::string_view{begin, pos_ + 1}));
        }
        ++pos_;
        ++column_;
      }
    }
    double d = 0;
    switch (s2d_n(begin, pos_ - begin, &d)) {
    case SUCCESS:
      return d;
    case INPUT_TOO_SHORT:
      throw std::runtime_error(std::format(
          "Error {}{}{}: cannot parse floating point value: '{}' is too short",
          filename_, line_, column_, std::string_view{begin, pos_}));
    case INPUT_TOO_LONG:
      throw std::runtime_error(std::format(
          "Error {}{}{}: cannot parse floating point value: '{}' is too long",
          filename_, line_, column_, std::string_view{begin, pos_}));
    case MALFORMED_INPUT:
      throw std::runtime_error(std::format(
          "Error {}{}{}: cannot parse floating point value: '{}' is malformed",
          filename_, line_, column_, std::string_view{begin, pos_}));
    }
  }

  std::string_view consume_identifier() {
    assert(pos_);
    assert(end_);
    assert(pos_ != end_);
    assert(is_letter(*pos_));
    const char *const begin = pos_;
    ++pos_;
    ++column_;
    while (pos_ != end_ and is_identfier_char(*pos_)) {
      ++pos_;
      ++column_;
    }
    return std::string_view{begin, static_cast<std::size_t>(pos_ - begin)};
  }

  std::string_view consume_string() {
    assert(pos_);
    assert(end_);
    assert(pos_ != end_);
    assert(*pos_ == '\'' or *pos_ == '"');
    const char *const begin = pos_;
    const char quote_char = *pos_;
    char prev_char = quote_char;
    ++pos_;
    ++column_;

    if (pos_ == end_) {
      throw std::runtime_error(
          std::format("Error {}{}{}: expected closing quote {} for string",
                      filename_, line_, column_, quote_char));
    }

    while (pos_ != end_) {
      const char current_char = *pos_;

      if (current_char == quote_char and prev_char != '\\') {
        // found end of string

        // the start of the string comes after the quote char
        const char *contents_begin = begin + 1;

        // the end of the string is the current postion
        const char *content_end = pos_;
        ++pos_;
        return {contents_begin,
                static_cast<std::size_t>(content_end - contents_begin)};
      }

      if (current_char == '\n') {
        // if a new line is encountered, increment the line counter and reset
        // column
        ++line_;
        column_ = 1;
      } else {
        // else just increase the column counter
        ++column_;
      }

      ++pos_;
      prev_char = current_char;
    }

    throw std::runtime_error(
        std::format("Error {}{}{}: expected closing quote {} for string",
                    filename_, line_, column_, quote_char));
  }

  void lex(std::string_view s) {
    if (s.empty())
      return;
    tokens_.clear();
    input_.insert(0, s.data(), s.size());
    pos_ = input_.data();
    end_ = input_.data() + input_.size();
    tok_index_ = 0;
    line_ = 1;
    column_ = 1;
    while (pos_ != end_) {
      const char current_c = *pos_;
      switch (current_c) {
      case '{':
        tokens_.push_back(
            Token{TokenType::o_brace,
                  Source{Location{line_, column_}, Location{line_, column_}},
                  current_c});
        ++pos_;
        ++column_;
        break;
      case '}':
        tokens_.push_back(
            Token{TokenType::c_brace,
                  Source{Location{line_, column_}, Location{line_, column_}},
                  current_c});
        ++pos_;
        ++column_;
        break;
      case '[':
        tokens_.push_back(
            Token{TokenType::o_bracket,
                  Source{Location{line_, column_}, Location{line_, column_}},
                  current_c});
        ++pos_;
        ++column_;
        break;
      case ']':
        tokens_.push_back(
            Token{TokenType::c_bracket,
                  Source{Location{line_, column_}, Location{line_, column_}},
                  current_c});
        ++pos_;
        ++column_;
        break;
      case ':':
        tokens_.push_back(
            Token{TokenType::colon,
                  Source{Location{line_, column_}, Location{line_, column_}},
                  current_c});
        ++pos_;
        ++column_;
        break;
      case ',':
        tokens_.push_back(
            Token{TokenType::comma,
                  Source{Location{line_, column_}, Location{line_, column_}},
                  current_c});
        ++pos_;
        ++column_;
        break;
      case '\'':
        [[fallthrough]];
      case '"': {
        const auto begin_line = line_;
        const auto begin_col = column_;
        std::string_view str = consume_string();
        tokens_.push_back(Token{.type = TokenType::string,
                                .source = {.begin = {begin_line, begin_col},
                                           .end = {line_, column_ - 1}},
                                .value = str});
      } break;
      case '-': {
        auto begin_col = column_;
        auto value = consume_number();
        std::visit(
            ol{[this, begin_col](double v) {
                 tokens_.push_back(Token{.type = TokenType::floating_point,
                                         .source = {.begin = {line_, begin_col},
                                                    .end = {line_, column_}},
                                         .value = v});
               },
               [this, begin_col](int64_t i) {
                 tokens_.push_back(Token{.type = TokenType::signed_int,
                                         .source = {.begin = {line_, begin_col},
                                                    .end = {line_, column_}},
                                         .value = i});
               },
               [this, begin_col](uint64_t u) {
                 tokens_.push_back(Token{.type = TokenType::unsigned_int,
                                         .source = {.begin = {line_, begin_col},
                                                    .end = {line_, column_}},
                                         .value = u});
               }},
            value);

      } break;
      case 't': {
        std::string_view rest{pos_, end_};
        if (rest.starts_with("true")) {
          tokens_.push_back(Token{.type = TokenType::unsigned_int,
                                  .source = {.begin = {line_, column_},
                                             .end = {line_, column_ + 4}},
                                  .value = true});
          column_ += 4;
          pos_ += 4;
        } else {
          // identifier
          const auto begin_col = column_;
          auto name = consume_identifier();
          tokens_.push_back(Token{
              .type = TokenType::identifier,
              .source = {.begin = {line_, begin_col}, .end = {line_, column_}},
              .value = name});
        }

      } break;
      case 'f': {
        std::string_view rest{pos_, end_};
        if (rest.starts_with("false")) {
          tokens_.push_back(Token{.type = TokenType::unsigned_int,
                                  .source = {.begin = {line_, column_},
                                             .end = {line_, column_ + 5}},
                                  .value = true});
          pos_ += 5;
          column_ += 5;
        } else {
          // identifier
          const auto begin_col = column_;
          auto name = consume_identifier();
          tokens_.push_back(Token{
              .type = TokenType::identifier,
              .source = {.begin = {line_, begin_col}, .end = {line_, column_}},
              .value = name});
        }

      } break;
      case '\n':
        ++line_;
        column_ = 1;
        ++pos_;

        break;
      case ' ':
        [[fallthrough]];
      case '\t':
      case '\v':
      case '\f':
        ++pos_;
        ++column_;

        break;
      default: {
        const auto begin_col = column_;
        if (is_letter(current_c)) {
          auto name = consume_identifier();
          tokens_.push_back(Token{
              .type = TokenType::identifier,
              .source = {.begin = {line_, begin_col}, .end = {line_, column_}},
              .value = name});

        } else if (is_number(current_c)) {
          auto value = consume_positive_number();
          std::visit(ol{[this, begin_col](double v) {
                          tokens_.push_back(
                              Token{.type = TokenType::floating_point,
                                    .source = {.begin = {line_, begin_col},
                                               .end = {line_, column_}},
                                    .value = v});
                        },
                        [this, begin_col](int64_t i) {
                          tokens_.push_back(
                              Token{.type = TokenType::signed_int,
                                    .source = {.begin = {line_, begin_col},
                                               .end = {line_, column_}},
                                    .value = i});
                        },
                        [this, begin_col](uint64_t u) {
                          tokens_.push_back(
                              Token{.type = TokenType::unsigned_int,
                                    .source = {.begin = {line_, begin_col},
                                               .end = {line_, column_}},
                                    .value = u});
                        }},
                     value);
        }
        break;
      }
      }
    }
    return;
  }

  rds::Value parse() {
    auto val = parse_impl();
    if (tok_index_ != tokens_.size())
      throw std::runtime_error(std::format(
          "Error: did not consume all tokens (size = {}), tok_index = {}",
          tokens_.size(), tok_index_));
    return val;
  }

  rds::Value parse_impl() {
    for (; tok_index_ < tokens_.size(); ++tok_index_) {
      using enum TokenType;
      const auto &tok = tokens_[tok_index_];
      switch (tok.type) {
      case o_brace:
        ++tok_index_;
        return parse_map();
      case o_bracket:
        ++tok_index_;
        return parse_array();
      case string: {
        ++tok_index_;
        auto v = std::get<std::string_view>(tok.value);
        return std::string(v.begin(), v.end());
      }
      case signed_int:
        ++tok_index_;
        return std::get<int64_t>(tok.value);
      case unsigned_int:
        ++tok_index_;
        return std::get<uint64_t>(tok.value);
      case floating_point:
        ++tok_index_;
        return std::get<double>(tok.value);
      case boolean:
        ++tok_index_;
        return std::get<bool>(tok.value);
      default:
        throw std::runtime_error(std::format(
            "\nError {}{}{}: invalid token type. Expected either "
            "opening brace, opening bracket, or a value. Got token "
            "with type {} and value {}",
            filename_, tok.source.begin.line, tok.source.begin.column,
            to_string(tok.type), tok.value_as_string()));
      }
    }
    return {};
  }

  const Token *peek(std::size_t i) {
    auto idx = tok_index_ + i;
    if (idx > tokens_.size()) {
      return nullptr;
    }
    return &tokens_[idx];
  }

  const Token &expect(TokenType type) {
    if (tok_index_ >= tokens_.size()) {
      throw std::runtime_error(
          std::format("Error {}{}{}: expected {}, got end of file", filename_,
                      line_, column_, to_string(type)));
    }
    const auto &tok = tokens_[tok_index_];
    if (tok.type == type)
      return tok;

    throw std::runtime_error(std::format(
        "Error {}{}{}: expected {}, got {} with value {}", filename_,
        tok.source.begin.line, tok.source.begin.column, to_string(type),
        to_string(tok.type), tok.value_as_string()));
  }

  rds::ValueMap parse_map() {
    rds::ValueMap map;
    if (tok_index_ >= tokens_.size()) {
      throw std::runtime_error(
          std::format("Error {}{}{}: expected '}}', got end of file", filename_,
                      line_, column_));
    }

    const auto &tok = tokens_[tok_index_];

    if (tok.type == TokenType::c_brace) {
      // TODO: maybe not increment tok_index
      ++tok_index_;
      return map;
    }

    for (; tok_index_ < tokens_.size(); ++tok_index_) {
      // identifier : value [, value ]...[,] '}'
      const auto &ident = expect(TokenType::identifier);
      ++tok_index_;
      [[maybe_unused]] const auto &colon = expect(TokenType::colon);
      ++tok_index_;
      if (tok_index_ >= tokens_.size()) {
        throw std::runtime_error(
            std::format("Error {}{}{}: expected value, got end of file",
                        filename_, line_, column_));
      }

      if (tok_index_ >= tokens_.size()) {
        throw std::runtime_error(
            std::format("Error {}{}{}: expected '}}', got end of file",
                        filename_, line_, column_));
      }
      auto name = std::get<std::string_view>(ident.value);
      auto [_, inserted] =
          map.insert({{name.begin(), name.end()}, parse_impl()});
      if (not inserted)
        throw std::runtime_error(std::format(
            "Error {}{}{}: duplicate key (={}) found in object", filename_,
            ident.source.begin.line, ident.source.begin.column, name));

      const auto &next = tokens_[tok_index_];

      if (next.type == TokenType::comma) {

        if (tok_index_ + 1 == tokens_.size()) {
          throw std::runtime_error(
              std::format("Error {}{}{}: expected closing brace or another key "
                          "value pair, got end of file",
                          filename_, line_, column_));
        }
        const auto &next = tokens_[tok_index_ + 1];
        if (next.type == TokenType::c_brace) {
          ++tok_index_;
          return map;
        }
      } else if (next.type == TokenType::c_brace) {
        ++tok_index_;
        return map;
      } else {
        throw std::runtime_error(std::format(
            "Error {}{}{}: expected '}}', got {} with value "
            "{}",
            filename_, next.source.begin.line, next.source.begin.column,
            to_string(next.type), next.value_as_string()));
      }
    }
    throw std::runtime_error(
        std::format("Error {}{}{}: expected '}}', got end of file", filename_,
                    line_, column_));
  }

  rds::ValueList parse_array() {
    rds::ValueList list;

    if (tok_index_ >= tokens_.size()) {
      throw std::runtime_error(
          std::format("Error {}{}{}: expected ']', got end of file", filename_,
                      line_, column_));
    }

    const auto &tok = tokens_[tok_index_];

    if (tok.type == TokenType::c_bracket) {
      // TODO: maybe not increment tok_index
      ++tok_index_;
      return list;
    }

    for (; tok_index_ < tokens_.size(); ++tok_index_) {
      // value[, value...] [,] ']'
      list.push_back(parse_impl());
      if (tok_index_ == tokens_.size())
        throw std::runtime_error(
            std::format("Error {}{}{}: expected ']', got end of file",
                        filename_, tokens_[tok_index_ - 1].source.end.line,
                        tokens_[tok_index_ - 1].source.end.column));
      const auto &next = tokens_[tok_index_];
      if (next.type == TokenType::comma) {
        if (tok_index_ + 1 == tokens_.size())
          throw std::runtime_error(std::format(
              "Error {}{}{}: expected either a value or ']', got end of file",
              filename_, next.source.end.line, next.source.end.column));
        const auto &end = tokens_[tok_index_ + 1];
        if (end.type == TokenType::c_bracket) {
          ++tok_index_;
          return list;
        }
      } else if (next.type == TokenType::c_bracket) {
        ++tok_index_;
        return list;
      } else {
        throw std::runtime_error(std::format(
            "Error {}{}{}: expected ']', got {} with value {}", filename_,
            next.source.begin.line, next.source.begin.column,
            to_string(next.type), next.value_as_string()));
      }
    }

    throw std::runtime_error(
        std::format("Error {}{}{}: expected ']', got end of file", filename_,
                    line_, column_));
  }

  Value parse(std::string_view contents, std::string_view file_path) {
    Parser p;
    return p.parse_or_throw(contents, file_path);
  }
};

Value parse(std::string_view contents, std::string_view file_path = {});

inline ValueMap rds_deserialize(const std::string &s) {
  Parser p;
  auto v = p.parse_or_throw(s);
  if (v.is_map())
    return v.as_map();
  throw std::runtime_error(std::format(
      "Error while deserializing. Expected top level to be a map, got {}",
      to_string(v.type())));
}

inline std::string rds_serialize(const ValueMap &map) {
  return to_string(map, 2);
}

inline std::string to_string(std::string_view s) {
  if (s.empty())
    return {};
  return s.data();
}

inline bool add_value(std::vector<EnumeratedValue> &values,
                      const EnumeratedValue &val, uint64_t num_bits) {
  if (val.name.empty())
    return false;

  if ((detail::make_mask(num_bits - 1, 0) & val.value) != val.value)
    return false;

  auto it = std::find_if(values.begin(), values.end(),
                         [&val](const EnumeratedValue &v) {
                           return val.value == v.value or val.name == v.name;
                         });

  if (it != values.end())
    return false;

  values.insert(std::find_if(values.begin(), values.end(),
                             [&val](const EnumeratedValue &v) {
                               return val.value < v.value;
                             }),
                val);
  return true;
}

inline bool add_values(std::vector<EnumeratedValue> &values,
                       const std::vector<EnumeratedValue> &vals,
                       uint64_t num_bits) {

  for (const auto &val : vals)
    if (((detail::make_mask(num_bits - 1, 0) & val.value) != val.value) or
        val.name.empty())
      return false;

  auto it = std::find_if(
      values.begin(), values.end(), [&vals](const EnumeratedValue &v) {
        return std::find_if(vals.begin(), vals.end(),
                            [&v](const EnumeratedValue &val) {
                              return val.value == v.value or val.name == v.name;
                            }) != vals.end();
      });

  if (it != values.end())
    return false;

  for (const auto &val : vals) {
    values.insert(std::find_if(values.begin(), values.end(),
                               [&val](const EnumeratedValue &v) {
                                 return val.value < v.value;
                               }),
                  val);
  }
  return true;
}

inline bool is_valid_field(uint64_t num_bits,
                           const std::vector<std::unique_ptr<Field>> &fields,
                           const Field &f) {
  if (f.name.empty() or f.msb < f.lsb or f.msb >= num_bits)
    return false;

  auto it = std::find_if(
      fields.begin(), fields.end(), [&f](const std::unique_ptr<Field> &f_) {
        return f_->name == f.name or (f.lsb >= f_->lsb and f.lsb <= f_->msb) or
               (f.msb >= f_->lsb and f.msb <= f_->msb) or
               (f.lsb <= f_->lsb and f.msb >= f_->msb);
      });
  return it == fields.end();
}

inline bool
is_valid_register(const std::vector<std::unique_ptr<Register>> &registers,
                  const Register &reg) {
  if (reg.name.empty() or reg.size == 0)
    return false;

  auto it = std::find_if(
      registers.begin(), registers.end(),
      [&reg](const std::unique_ptr<Register> &r) {
        return reg.name == r->name or
               (reg.addr <= r->addr and reg.addr + reg.size > r->addr) or
               (reg.addr >= r->addr and reg.addr < r->addr + r->size);
      });
  return it == registers.end();
}

inline bool is_valid_group(const std::vector<std::unique_ptr<Group>> &groups,
                           const Group &group) {

  if (group.name.empty() or group.size == 0)
    return false;

  auto it = std::find_if(
      groups.begin(), groups.end(), [&group](const std::unique_ptr<Group> &g) {
        return g->name == group.name or g->base_addr == group.base_addr or
               (group.base_addr < g->base_addr and
                group.base_addr + group.size > g->base_addr) or
               (group.base_addr > g->base_addr and
                g->base_addr + g->size > group.base_addr);
      });
  return it == groups.end();
}

std::vector<std::string> split_at(const auto &str, char delimiter);

constexpr decltype(auto) to_signed_if_unsigned(auto &&v) {
  using T = std::remove_cvref_t<decltype(v)>;
  if constexpr (not std::is_same_v<T, bool> and std::is_unsigned_v<T>) {
    return static_cast<std::make_signed_t<T>>(v);
  } else {
    return std::forward<decltype(v)>(v);
  }
}

static constexpr bool is_integer_(std::string_view sv) {
  if (sv.empty())
    return false;
  if (sv.starts_with("0x") or sv.starts_with("0b"))
    sv = sv.substr(2);

  for (const auto &ch : sv) {
    if ((ch >= '0' and ch <= '9') or (ch >= 'a' and ch <= 'z') or
        (ch >= 'A' and ch <= 'Z'))
      continue;
    else
      return false;
  }
  return true;
}

template <class T> constexpr T from_string(std::string_view sv) {
  int base = 10;
  if (sv.starts_with("0x")) {
    base = 16;
    sv = sv.substr(2);
  } else if (sv.starts_with("0b")) {
    base = 2;
    sv = sv.substr(2);
  }
  T result{};
  auto [ptr, ec] =
      std::from_chars(sv.data(), sv.data() + sv.size(), result, base);
  if (ec != std::errc()) {
    throw std::runtime_error("expected integer, got '" + std::string(sv) + "'");
  }
  return result;
}

template <typename Range, typename Visitor>
constexpr void neighbor_visit(Range &&range, Visitor &&vis) {
  auto first = range.begin();
  auto last = range.end();
  if (first == last)
    return;
  auto f1 = first;
  auto f2 = ++f1;
  if (f2 == last)
    return;
  while (f2 != last) {
    vis(*f1, *f2);
    f1 = f2++;
  }
}

template <class... Ts> struct overload : Ts... {
  using Ts::operator()...;
};

void derive(ValueMap &map, std::vector<ValueMap *> parents = {},
            std::string_view name = {});

} // namespace detail
} // namespace rds

#endif
