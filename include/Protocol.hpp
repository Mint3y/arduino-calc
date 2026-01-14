#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <cstring>

namespace calc {

// Buffer size for a single expression
constexpr size_t MAX_EXPR_SIZE = 1024;

// ExpressionBuffer
// Static size Buffer class to store mathematical expressions
// e.g "7 * 3 + 8 / 2"
class ExpressionBuffer {
public:
    // Writes a string expression to the buffer.
    void write(const char* buffer, const size_t length);

    // Returns the amount of bytes the expression occupies in the buffer.
    size_t get_size() const noexcept;

    // Removes all whitespace in the buffer.
    void filter_whitespace();

    // Removes leading and trailing zeros from the buffer.
    void filter_insignificant_zeros();

private:
    char _buffer[MAX_EXPR_SIZE] = { 0 };
    size_t _size = 0;
};

};

#endif // PROTOCOL_HPP