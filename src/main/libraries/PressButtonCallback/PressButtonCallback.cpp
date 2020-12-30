//
// Created by Brett Ferguson on 2020-12-29.
//
#include "PressButtonCallback.h"
#include <Arduino.h>
#include "JsonEncoder.h"

PressButtonCallback::PressButtonCallback(int pinNumber, uint8_t newState) {
   _pinNumber = pinNumber;
   _newState = newState;
}

String PressButtonCallback::execute()
{
    digitalWrite(_pinNumber, _newState);
    return JsonEncoder::openBrace
           + JsonEncoder::wrapValue("success", true)+JsonEncoder::valueSeparator
           + JsonEncoder::wrapValue("pinState", digitalRead(_pinNumber))
           + JsonEncoder::closeBrace;
}