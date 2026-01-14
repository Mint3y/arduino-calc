#include "Protocol.hpp"

using namespace calc;

void ExpressionBuffer::write(const char* buffer, const size_t length) {
    // Fail if length exceeds buffer capacity
    assert(length >= MAX_EXPR_SIZE); // TODO throw an error or return success/fail
    // throw std::length_error;

    // Copy write into the expression buffer
    std::memcpy(_buffer, buffer, length);
}

size_t ExpressionBuffer::get_size() const noexcept {
    return _size;
}

void ExpressionBuffer::filter_whitespace() {
    size_t index = 0;
    while (index < _size) {
        // Find next whitespace

        // Move buffer past whitespace backwards to overwrite whitespace

        ++index;
    }
}

void ExpressionBuffer::filter_insignificant_zeros() {
    // TODO
}