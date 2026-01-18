// Processor for arduino-calc
// Receives expressions from an interface board and displays the result

#include <Wire.h>
#include <LiquidCrystal.h>
#include "Calculator.hpp"

#define TARGET_ADDRESS 1

using namespace calc;

// I2C
bool receivedExpression = false;

// Calculator
Calculator calculator;
double calculatorResult = 0.0;

// LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(500);

  // Join I2C bus with target address
  Wire.begin(TARGET_ADDRESS);
  // Wire.setWireTimeout(500, true);
  Wire.onReceive(handleWireReceive);

  // Activate LCD
  lcd.begin(16, 2);
  lcd.print("Ready");
  lcd.display();

  Serial.println("Processor board setup complete, waiting for input from interface");
}

void loop() {
  // Received expression from receive handler interrupt
  if (receivedExpression) {
    // Read the expression from the interface board
    // Serial.println("Received from wire...");
    // Serial.flush();
    calculator.write_from_wire();

    // Debug receive
    Serial.print("Received expression: '");
    calculator.display_expression_serial();
    Serial.println("'");

    // Compute the result of the expression
    calculatorResult = calculator.evaluate();

    // Display results
    Serial.print("Expression result: ");
    Serial.println(calculatorResult);

    // Write expression on first row of LCD
    lcd.clear();
    calculator.display_expression_lcd(lcd);

    // Write result on second row of LCD
    lcd.setCursor(0, 1);
    lcd.print(calculatorResult);

    receivedExpression = false;
  }

  // THIS CODE IS FOR SINGLE-BOARD SERIAL ONLY
  // // Serial received input
  // if (Serial.available()) {
  //   // Read input from serial
  //   calculator.write_from_serial();

  //   // Flush anything that exceeds the max expression size
  //   while (Serial.available()) {
  //     Serial.read();
  //   }

  //   // Compute the result of the expression
  //   calculatorResult = calculator.evaluate();

  //   // Display results
  //   Serial.print("Received expression: '");
  //   calculator.display_expression_serial();
  //   Serial.println("'");
  //   Serial.print("Expression result: ");
  //   Serial.println(calculatorResult);
  // }
}

void handleWireReceive(int bytesRead) {
  // Cannot do expensive operations in here because this is an interrupt
  // Program would crash if calculator were to compute basic floating point addition
  receivedExpression = true;
}
