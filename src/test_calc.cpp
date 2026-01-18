#include <iostream>
#include "Calculator.hpp"

using namespace calc;

int main(int argc, char** argv) {
    // std::string expression = "5.13+3.2";
    std::string expression = "5+3*4*5";

    Calculator calculator;
    calculator.write(expression.c_str(), expression.length());
    double result = calculator.evaluate();

    std::cout << "Expression: " << expression << std::endl;
    std::cout << "Result: " << result << std::endl;

    return 0;
}
