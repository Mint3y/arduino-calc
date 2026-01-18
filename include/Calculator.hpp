#ifndef CALCULATOR_HPP
#define CALCULATOR_HPP

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <unordered_map>

#include <cassert>
#include <iostream>

// Max buffer size for a single expression
#ifndef MAX_EXPR_SIZE
    #define MAX_EXPR_SIZE 1024
#endif


namespace calc {

size_t starts_with_numeric(const char* str, size_t length);
char filter_arithmetic_ops(char c);
char filter_braces(char c);

// TODO wrap everything in a class with getters, setters, is_number, is_operation
struct Token {
    enum class Type {
        NUMBER,
        OPERATION,
        BRACE,
        FUNCTION,
        UNKNOWN
    };
    enum class FunctionType {
        SQRT,
        SIN,
        COS
    };

    // Hash specialisation for the FunctionType ENUM CLASS written by everyone's
    // favourite ChatGPT because apparently the compiler doesn't implicitly convert
    // enum classes to a hashable value.
    struct FunctionTypeHash {
        size_t operator()(FunctionType ft) const noexcept {
            return static_cast<size_t>(ft);
        }
    };

    // TODO: what the fk
    static std::unordered_map<FunctionType, double (*)(double), FunctionTypeHash> FUNC_MAP;
    static std::unordered_map<FunctionType, size_t, FunctionTypeHash> FUNC_LENGTH_MAP;

    Type type;
    size_t length;
    union {
        double num;
        char op;
        // bool open_brace;
    } value;
};

// // Binary Tree Node implementation
// template<typename T>
// struct BinaryNode;

// template<typename T>
// struct BinaryNode {
//     T value;
//     BinaryNode<T> left;
//     BinaryNode<T> right;
// };

// Calculator
// Stores and evaluates mathematical expressions
// e.g "7 * 3 + 8 / 2"
class Calculator {
public:
    // Writes a string expression to the buffer.
    void write(const char* buffer, const size_t length);

    // Returns the amount of bytes the expression occupies in the buffer.
    size_t get_size() const noexcept;

    // Removes all whitespace in the buffer.
    void filter_whitespace();

    // Removes leading and trailing zeros from the buffer.
    void filter_insignificant_zeros();

    double evaluate() const;

private:
    Token eval(Token lhs) const;

    Token eval_operation(const Token& left, const Token& op, const Token& right) const;
    Token eval_function(const Token& tok) const;
    int priority(char op) const;
    bool is_closure(size_t index) const;

    Token parse_token(size_t index) const;
    Token parse_numeric_token(size_t index, size_t length) const;
    Token parse_operation_token(size_t index) const;
    Token parse_brace_token(size_t index) const;
    Token parse_function_token(size_t index) const;
    // std::tuple< parse_function(size_t index) const;

    char _buffer[MAX_EXPR_SIZE] = { 0 };
    size_t _size = 0;
};

};

#endif // CALCULATOR_HPP
