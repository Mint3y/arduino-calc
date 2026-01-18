#ifndef CALCULATOR_HPP
#define CALCULATOR_HPP

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>

// Max buffer size for a single expression
#ifndef MAX_EXPR_SIZE
    #define MAX_EXPR_SIZE 511
#endif


namespace calc {

size_t starts_with_numeric(const char* str, size_t length);
bool starts_with(const char* str, const char* prefix);
char filter_arithmetic_ops(char c);
char filter_braces(char c);
size_t find_brace(const char* str, size_t length);

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
        COS,
        UNKNOWN
    };

    // Returns the value given an input to the function
    static inline double FUNC_MAP(double input, FunctionType function) {
        switch (function) {
        case FunctionType::SQRT:
            return sqrt(input);

        case FunctionType::SIN:
            return sin(input);

        case FunctionType::COS:
            return cos(input);

        default:
            return 0.0;
        }
    }

    // Returns the length of the function name
    static size_t FUNC_LENGTH_MAP(FunctionType function) {
        switch (function) {
        case FunctionType::SQRT:
            return 4;

        case FunctionType::SIN:
        case FunctionType::COS:
            return 3;

        default:
            return 0;
        }
    }

    Type type;
    size_t length;
    union {
        double num;
        char op;
        FunctionType ft;
    } value;
};

// Calculator
// Stores and evaluates mathematical expressions
// e.g "7 * 3 + 8 / 2"
class Calculator {
public:
    // Makes the entire buffer lower case
    void to_lower();
    
    // Writes a string expression to the buffer.
    bool write(const char* buffer, const size_t length);
    
    // Writes a string expression into the buffer from Arduino serial
    bool write_from_serial();

    // Writes a string expression into the buffer from Arduino wire
    bool write_from_wire();

    // Displays the expression in the buffer to serial print
    void display_expression_serial();

    // Displays the expression in the buffer on a LCD
    void display_expression_lcd(LiquidCrystal& lcd);

    // Returns the amount of bytes the expression occupies in the buffer.
    size_t get_size() const noexcept;

    double evaluate() const;

private:
    Token eval(Token lhs) const;

    Token eval_operation(const Token& left, const Token& op, const Token& right) const;
    Token eval_function(const Token& tok) const;
    int priority(char op) const;
    bool is_closure(size_t index) const;

    Token parse_token(size_t index, bool op_prio) const;
    Token parse_numeric_token(size_t index, size_t length) const;
    Token parse_operation_token(size_t index) const;
    Token parse_brace_token(size_t index) const;
    Token parse_function_token(size_t index) const;

    char _buffer[MAX_EXPR_SIZE] = { 0 };
    size_t _size = 0;
};

};

#endif // CALCULATOR_HPP
