#include "Calculator.hpp"

using namespace calc;

size_t calc::starts_with_numeric(const char* str, size_t length) {
    bool found_decimal_point = false;

    // Iterate until a non numeric character or end of string is reached
    for (size_t i = 0; i < length; ++i) {
        // Special case: first character is a minus sign, ignore it and continue
        if (i == 0 && str[0] == '-') {
            continue;
        }

        // Non-numeric character has been reached, return the numeric size
        if (!isDigit(str[i])
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

bool calc::starts_with(const char* str, const char* prefix) {
    for (int i = 0; i < strlen(prefix); ++i) {
        if (str[i] != prefix[i]) {
            return false;
        }
    }

    return true;
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

size_t calc::find_brace(const char* str, size_t length) {
    for (int i = 0; i < length; ++i) {
        switch (str[i]) {
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
            return i;
        }
    }

    return length;
}

void Calculator::to_lower() {
    for (int i = 0; i < _size; ++i) {
        if (_buffer[i] >= 'A' && _buffer[i] <= 'Z') {
            _buffer[i] += 'a' - 'A';
        }
    }
}

bool Calculator::write(const char* buffer, const size_t length) {
    // Fail if length exceeds buffer capacity
    // assert(length < MAX_EXPR_SIZE); // TODO throw an error or return success/fail
    // throw std::length_error;
    if (length < MAX_EXPR_SIZE) {
        return false;
    }

    // Copy write into the expression buffer
    memcpy(_buffer, buffer, length);

    // Update size
    _size = length;
    _buffer[_size] = '\0';

    to_lower();
    return true;
}


bool Calculator::write_from_serial() {
    _size = Serial.readBytes(_buffer, MAX_EXPR_SIZE);
    _buffer[_size] = '\0';
    to_lower();
    return true;
}

bool Calculator::write_from_wire() {
    // Clear buffer
    _size = 0;

    // Read input from wire until all has been read or buffer is full
    while (Wire.available()
       &&  _size < MAX_EXPR_SIZE) {
        // Read next character into buffer and increment size
        _buffer[_size] = Wire.read();
        ++_size;
    }

    // Write null terminator at end of expression
    _buffer[_size] = '\0';
    to_lower();
    return true;
}

void Calculator::display_expression_serial() {
    Serial.print(_buffer);
}

void Calculator::display_expression_lcd(LiquidCrystal& lcd) {
    lcd.home();
    lcd.print(_buffer);
}

size_t Calculator::get_size() const noexcept {
    return _size;
}

double Calculator::evaluate() const {
    // Evaluate the expression beginning with the first token
    Token left = parse_token(0, false);
    Token result = eval(left);
    return result.value.num;
}

// TODO: add a parameter that takes in the type of closing brace to expect, or
// the end of the expression. If a different closing brace is met return an error
// Alternatively validate the string before doing this
Token Calculator::eval(Token lhs) const {
    // Evaluation begins with a brace
    if (lhs.type == Token::Type::BRACE) {
        // Parse the token to the right of the brace
        Token left = parse_token(lhs.length, false);

        // Nested eval begins from the end of this token
        left.length += lhs.length;
        lhs = eval(left);
    }
    else if (lhs.type == Token::Type::FUNCTION) {
        lhs = eval_function(lhs);
    }

    // For some magical reason NOTHING in this switch statement runs
    // switch (lhs.type) {
    // case Token::Type::BRACE:
    //     // Parse the token to the right of the brace
    //     Token left = parse_token(lhs.length + 1, false);

    //     // Nested eval begins from the end of this token
    //     lhs.length += left.length;
    //     lhs = eval(lhs);
    //     break;

    // case Token::Type::FUNCTION:
    //     Serial.println("Evaluating LHS function");
    //     lhs = eval_function(lhs);
    //     break;

    // default:
    //     Serial.println("?????????");
    // }

    // Until a closing condition is met...
    while (!is_closure(lhs.length)) {
        // Parse the operation
        Token op = parse_token(lhs.length, true);
        // TODO error handling
    
        // Parse the right operand
        Token right = parse_token(lhs.length + op.length, false);
        // TODO error handling

        // Right operand is a function, evaluate it to a number
        switch (right.type) {
        case Token::Type::BRACE:
            // Parse the token to the right of the right token
            right = parse_token(lhs.length + op.length + right.length, false);

            // Nested eval begins from the end of this token
            right.length = lhs.length + op.length + right.length + 1;
            right = eval(right);
            break;

        case Token::Type::FUNCTION:
            right.length = lhs.length + op.length + right.length;
            right = eval_function(right);
            break;
        }
    
        // Parse the following operation
        Token next_op = parse_token(lhs.length + op.length + right.length, true);
    
        // Current operation priority is lower than next, evaluate RHS first
        if (priority(op.value.op) < priority(next_op.value.op)) {
            // Evaluate higher priority op giving the right operand as LHS
            right.length = lhs.length + op.length + right.length;
            right = eval(right);
        }
    
        // Evaluate current operation, extending LHS length
        lhs = eval_operation(lhs, op, right);
    }

    // Reached a closing brace, extend LHS by 1 character
    ++lhs.length; // NOTE: When finalising the full expression this causes lhs.length = _size + 1

    return lhs;
}

Token Calculator::eval_operation(const Token& left, const Token& op, const Token& right) const {
    size_t result_end = left.length + op.length + right.length;
    Token result = { .type=Token::Type::NUMBER, .length=result_end };

    // Calculate the result of the operation
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
        // Division by 0
        if (right.value.num == 0.0) {            
            result.value.num = NAN;
        }
        else {
            result.value.num = left.value.num / right.value.num;            
        }
        break;
        
    case '^':
        result.value.num = pow(left.value.num, right.value.num);
        break;
        
    default:
        result.value.num = NAN;
    }

    return result;
}

Token Calculator::eval_function(const Token& tok) const {
    // Parse the first token (skip function open brace)
    Token body = parse_token(tok.length + 1, false);

    // Evaluate the function body
    body.length += tok.length + 1;
    body = eval(body);

    // Perform the function
    // TODO: Create a proper function map
    body.type = Token::Type::NUMBER;
    body.value.num = Token::FUNC_MAP(body.value.num, tok.value.ft);
    
    return body;
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

    default:
        return 0;
    }
}

bool Calculator::is_closure(size_t index) const {
    return (index >= _size) || (_buffer[index] == ')');
}


Token Calculator::parse_token(size_t index, bool op_prio) const {
    // Parse target index is out of bounds
    if (index >= _size) {
        return (Token){ .type=Token::Type::UNKNOWN, .length=0, .value={.num=0} };
    }

    // Expecting an operation, check for operations first (for negative numbers)
    if (op_prio) {
        if (filter_arithmetic_ops(_buffer[index]) != 0) {
            return parse_operation_token(index);
        }
    }

    // Check if token is numeric
    size_t numeric_length = starts_with_numeric(_buffer + index, _size - index);
    if (numeric_length > 0) {
        return parse_numeric_token(index, numeric_length);
    }

    // Check if token is an arithmetic operation
    if (filter_arithmetic_ops(_buffer[index]) != 0) {
        return parse_operation_token(index);
    }

    // Check if token is a brace
    if (filter_braces(_buffer[index]) != 0) {
        return parse_brace_token(index);
    }

    Serial.println("Parsing a function token");

    // Assume token is a function
    return parse_function_token(index);
}

Token Calculator::parse_numeric_token(size_t index, size_t length) const {
    // Create and return the numeric token at the given index
    Token tok = { .type=Token::Type::NUMBER, .length=length };
    tok.value.num = atof(_buffer + index);
    return tok;
}

Token Calculator::parse_operation_token(size_t index) const {
    // Create and return the operation token at the given index
    Token tok = { .type=Token::Type::OPERATION, .length=1 };
    tok.value.op = _buffer[index];
    return tok;
}

Token Calculator::parse_brace_token(size_t index) const {
    return (Token){ .type=Token::Type::BRACE, .length=1 };
}

Token Calculator::parse_function_token(size_t index) const {
    // Create and return the function token at the given index
    Token tok = { .type=Token::Type::FUNCTION, .length=0 };
    if (starts_with(_buffer + index, "sin")) {
        tok.value.ft = Token::FunctionType::SIN;
    }
    else if (starts_with(_buffer + index, "cos")) {
        tok.value.ft = Token::FunctionType::COS;
    }
    else if (starts_with(_buffer + index, "sqrt")) {
        tok.value.ft = Token::FunctionType::SQRT;
    }
    else {
        tok.value.ft = Token::FunctionType::UNKNOWN;
    }

    // Update token length
    tok.length = Token::FUNC_LENGTH_MAP(tok.value.ft);
    if (tok.value.ft == Token::FunctionType::UNKNOWN) {
        Serial.print("Parsed unknown function at index: ");
        Serial.println(index);
        // Find the first open brace and set the length to it
        tok.length = find_brace(_buffer + index, _size - index);
    }

    return tok;
}
