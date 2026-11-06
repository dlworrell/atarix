// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0
//
// Minimal JSON reader for CPU test-vector files.
//
// Scope: the value shapes used by the SingleStepTests 65x02 and 65816 vector
// sets (objects, arrays, integers, strings, null, true/false). It is a strict
// recursive-descent parser over an in-memory buffer; malformed input raises
// std::runtime_error. It is test tooling, not an ATARIX runtime component.

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace atarix::json {

struct Value {
	enum class Kind { Null, Bool, Number, String, Array, Object };

	Kind kind = Kind::Null;
	bool boolean = false;
	int64_t number = 0;
	std::string string;
	std::vector<Value> array;
	std::map<std::string, Value, std::less<>> object;

	bool is_null() const { return kind == Kind::Null; }

	const Value &at(std::string_view key) const {
		if(kind != Kind::Object) throw std::runtime_error("json: not an object");
		const auto it = object.find(key);
		if(it == object.end()) throw std::runtime_error("json: missing key " + std::string(key));
		return it->second;
	}

	bool has(std::string_view key) const {
		return kind == Kind::Object && object.find(key) != object.end();
	}

	const Value &operator[](size_t index) const {
		if(kind != Kind::Array || index >= array.size()) throw std::runtime_error("json: bad array index");
		return array[index];
	}

	int64_t integer() const {
		if(kind != Kind::Number) throw std::runtime_error("json: not a number");
		return number;
	}

	const std::string &text() const {
		if(kind != Kind::String) throw std::runtime_error("json: not a string");
		return string;
	}
};

class Parser {
public:
	explicit Parser(std::string_view input) : in_(input) {}

	Value parse() {
		Value v = value();
		skip_space();
		if(pos_ != in_.size()) fail("trailing data");
		return v;
	}

private:
	std::string_view in_;
	size_t pos_ = 0;
	int depth_ = 0;
	static constexpr int MaxDepth = 64;

	[[noreturn]] void fail(const char *why) const {
		throw std::runtime_error(std::string("json: ") + why + " at offset " + std::to_string(pos_));
	}

	void skip_space() {
		while(pos_ < in_.size()) {
			const char c = in_[pos_];
			if(c == ' ' || c == '\n' || c == '\r' || c == '\t') ++pos_;
			else break;
		}
	}

	char peek() {
		skip_space();
		if(pos_ >= in_.size()) fail("unexpected end");
		return in_[pos_];
	}

	void expect(char c) {
		if(peek() != c) fail("unexpected character");
		++pos_;
	}

	bool literal(std::string_view word) {
		if(in_.substr(pos_, word.size()) == word) {
			pos_ += word.size();
			return true;
		}
		return false;
	}

	Value value() {
		if(++depth_ > MaxDepth) fail("nesting too deep");
		Value v;
		const char c = peek();
		if(c == '{') v = object();
		else if(c == '[') v = array();
		else if(c == '"') { v.kind = Value::Kind::String; v.string = string(); }
		else if(c == '-' || (c >= '0' && c <= '9')) v = number();
		else if(literal("null")) v.kind = Value::Kind::Null;
		else if(literal("true")) { v.kind = Value::Kind::Bool; v.boolean = true; }
		else if(literal("false")) { v.kind = Value::Kind::Bool; v.boolean = false; }
		else fail("unexpected token");
		--depth_;
		return v;
	}

	Value number() {
		Value v;
		v.kind = Value::Kind::Number;
		bool negative = false;
		if(in_[pos_] == '-') { negative = true; ++pos_; }
		if(pos_ >= in_.size() || in_[pos_] < '0' || in_[pos_] > '9') fail("bad number");
		int64_t n = 0;
		while(pos_ < in_.size() && in_[pos_] >= '0' && in_[pos_] <= '9') {
			if(n > (INT64_MAX - 9) / 10) fail("number overflow");
			n = n * 10 + (in_[pos_] - '0');
			++pos_;
		}
		if(pos_ < in_.size() && (in_[pos_] == '.' || in_[pos_] == 'e' || in_[pos_] == 'E')) {
			fail("non-integer numbers are not used by vector files");
		}
		v.number = negative ? -n : n;
		return v;
	}

	std::string string() {
		expect('"');
		std::string out;
		while(true) {
			if(pos_ >= in_.size()) fail("unterminated string");
			const char c = in_[pos_++];
			if(c == '"') break;
			if(c == '\\') {
				if(pos_ >= in_.size()) fail("bad escape");
				const char e = in_[pos_++];
				switch(e) {
					case '"': out.push_back('"'); break;
					case '\\': out.push_back('\\'); break;
					case '/': out.push_back('/'); break;
					case 'n': out.push_back('\n'); break;
					case 't': out.push_back('\t'); break;
					case 'r': out.push_back('\r'); break;
					case 'b': out.push_back('\b'); break;
					case 'f': out.push_back('\f'); break;
					default: fail("unsupported escape");
				}
			} else {
				out.push_back(c);
			}
		}
		return out;
	}

	Value array() {
		Value v;
		v.kind = Value::Kind::Array;
		expect('[');
		if(peek() == ']') { ++pos_; return v; }
		while(true) {
			v.array.push_back(value());
			const char c = peek();
			++pos_;
			if(c == ']') break;
			if(c != ',') fail("expected , or ]");
		}
		return v;
	}

	Value object() {
		Value v;
		v.kind = Value::Kind::Object;
		expect('{');
		if(peek() == '}') { ++pos_; return v; }
		while(true) {
			if(peek() != '"') fail("expected key");
			std::string key = string();
			expect(':');
			v.object.insert_or_assign(std::move(key), value());
			const char c = peek();
			++pos_;
			if(c == '}') break;
			if(c != ',') fail("expected , or }");
		}
		return v;
	}
};

inline Value parse(std::string_view input) {
	return Parser(input).parse();
}

}  // namespace atarix::json
