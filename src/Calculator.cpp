#include "Calculator.hpp"

using namespace calc;

std::unordered_map<Token::FunctionType, double (*)(double), Token::FunctionTypeHash> FUNC_MAP = {
    { Token::FunctionType::SQRT, std::sqrt },
    { Token::FunctionType::SIN,  std::sin  },
    { Token::FunctionType::COS,  std::cos  },
};
std::unordered_map<Token::FunctionType, size_t, Token::FunctionTypeHash> FUNC_LENGTH_MAP = {
    { Token::FunctionType::SQRT, 4 },
    { Token::FunctionType::SIN,  3 },
    { Token::FunctionType::COS,  3 },
};

size_t calc::starts_with_numeric(const char* str, size_t length) {
    bool found_decimal_point = false;

    // Iterate until a non numeric character or end of string is reached
    for (size_t i = 0; i < length; ++i) {
        // Special case: first character is a minus sign, ignore it and continue
        if (i == 0 && str[0] == '-') {
            continue;
        }

        // Non-numeric character has been reached, return the numeric size
        if (!std::isdigit(str[i])
        &&  (str[i] != '.' || found_decimal_point)) {
            // TODO: edge cases that std::atof supports
            return i;
        }

        // Found a decimal point in the number
        if (str[i] == '.') {
            found_decimal_point = true;
        }
    }

    return length;
}

char calc::filter_arithmetic_ops(char c) {
    // TODO: create some kind of list or map instead of this
    switch (c) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '^':
        return c;

    default:
        return 0;
    }
}

char calc::filter_braces(char c) {
    switch (c) {
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
        return c;

    default:
        return 0;
    }
}

void Calculator::write(const char* buffer, const size_t length) {
    // Fail if length exceeds buffer capacity
    assert(length < MAX_EXPR_SIZE); // TODO throw an error or return success/fail
    // throw std::length_error;

    // Copy write into the expression buffer
    std::memcpy(_buffer, buffer, length);

    // Update size
    _size = length;
}

size_t Calculator::get_size() const noexcept {
    return _size;
}

void Calculator::filter_whitespace() {
    size_t index = 0;
    while (index < _size) {
        // Find next whitespace

        // Move buffer past whitespace backwards to overwrite whitespace

        ++index;
    }
}

void Calculator::filter_insignificant_zeros() {
    // TODO
}

double Calculator::evaluate() const {
    // Evaluate the expression beginning with the first token
    Token left = parse_token(0);
    std::cout << "Left token length: " << left.length << std::endl;
    std::cout << "left token value: " << left.value.num << std::endl;
    Token result = eval(left);
    return result.value.num;
}

Token Calculator::eval(Token lhs) const {
    // Until the end of the expression or closing brace is reached...
    // while (lhs.length < _size
    //    &&  !is_closing_brace(lhs.length)) {


    // Until a closing condition is met...
    while (!is_closure(lhs.length)) {
        std::cout << "Loop start" << std::endl;
        // Parse the operation
        Token op = parse_token(lhs.length);
        // TODO error handling
    
        // Parse the right operand
        Token right = parse_token(lhs.length + op.length);
        // TODO error handling

        // Right operand is a function, evaluate it to a number
        if (right.type == Token::Type::FUNCTION) {
            std::cout << "Parsing right function" << std::endl;
            right = eval_function(right);
            std::cout << "Finished parsing right function" << std::endl;
        }
    
        // Check if the end of the expression has been reached
        // if (cursor >= _size) {
        //     // TODO just return a null token in parse token
        // }
    
        // Parse the following operation
        Token next_op = parse_token(lhs.length + op.length + right.length);
        // TODO something about braces
        // If closing brace, nothing needs to be done
        // If opening brace, it will be evaluated first
    
        // Current operation priority is lower than next, evaluate RHS first
        if (priority(op.value.op) < priority(next_op.value.op)) {
            std::cout << "Parsing RHS (higher priority)" << std::endl;
            // Evaluate higher priority op giving the right operand as LHS
            right.length = lhs.length + op.length + right.length;
            right = eval(right);
            std::cout << "Finished eval RHS" << std::endl;
        }
    
        // Evaluate current operation, extending LHS length
        lhs = eval_operation(lhs, op, right);
        std::cout << "Loop end, LHS len: " << lhs.length << std::endl;
    }

    // Reached a closing brace, extend LHS by 1 character
    ++lhs.length; // NOTE: When finalising the full expression this causes lhs.length = _size + 1

    return lhs;
}

Token Calculator::eval_operation(const Token& left, const Token& op, const Token& right) const {
    size_t result_end = left.length + op.length + right.length;
    Token result = { .type=Token::Type::NUMBER, .length=result_end };

    // Calculate the result of the operation
    // TODO: create a function map for this
    switch (op.value.op) {
    case '+':
        result.value.num = left.value.num + right.value.num;
        break;
        
    case '-':
        result.value.num = left.value.num - right.value.num;
        break;
        
    case '*':
        result.value.num = left.value.num * right.value.num;
        break;
        
    case '/':
        result.value.num = left.value.num / right.value.num;
        break;
        
    case '^':
        // result.value.num = left.value.num + right.value.num;
        break;
        
    default:
        result.value.num = NAN;
    }

    return result;
}

Token Calculator::eval_function(const Token& tok) const {
    return (Token){ .type=Token::Type::UNKNOWN, .length=0 };
    
}

int Calculator::priority(char op) const {
    switch (op) {
    case '+':
    case '-':
        return 1;
    case '*':
    case '/':
        return 2;
    case '^':
        return 3;
    // TODO: braces

    default:
        return 0;
    }
}

bool Calculator::is_closure(size_t index) const {
    return (index >= _size) || (_buffer[index] == ')');
}


Token Calculator::parse_token(size_t index) const {
    // Parse target index is out of bounds
    if (index >= _size) {
        return (Token){ .type=Token::Type::UNKNOWN, .length=0, .value={.num=0} };
    }

    // Check if token is numeric
    size_t numeric_length = starts_with_numeric(_buffer + index, _size - index);
    if (numeric_length > 0) {
        // std::cout << "Parsed numeric token at index " << index << std::endl;
        // std::cout << "with length " << numeric_length << std::endl;
        return parse_numeric_token(index, numeric_length);
    }

    // Check if token is an arithmetic operation
    if (filter_arithmetic_ops(_buffer[index]) != 0) {
        // std::cout << "Parsed operation token at index " << index << std::endl;
        return parse_operation_token(index);
    }

    std::cout << "Did not parse numeric or operation token" << std::endl;

    // Check if token is a brace
    if (filter_braces(_buffer[index]) != 0) {
        std::cout << "Parsed brace token at index " << index << std::endl;
        return parse_brace_token(index);
    }

    // Check if token is a function
    // auto func = parse_function(index);

    // WARNING: Unknown token
    return (Token){ .type=Token::Type::UNKNOWN, .length=0 };
}

Token Calculator::parse_numeric_token(size_t index, size_t length) const {
    // Create and return the numeric token at the given index
    Token tok = { .type=Token::Type::NUMBER, .length=length };
    tok.value.num = std::atof(_buffer + index);
    return tok;
}

Token Calculator::parse_operation_token(size_t index) const {
    // Create and return the operation token at the given index
    Token tok = { .type=Token::Type::OPERATION, .length=1 };
    tok.value.op = _buffer[index];
    return tok;
}

Token Calculator::parse_brace_token(size_t index) const {
    return (Token){ .type=Token::Type::UNKNOWN, .length=0 };
}

Token Calculator::parse_function_token(size_t index) const {
    return (Token){ .type=Token::Type::UNKNOWN, .length=0 };
}
