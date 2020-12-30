//
// Created by Brett Ferguson on 2020-12-30.
// Todo: Replace with ArduinoJson library

#ifndef ARDUINO_JSONENCODER2_H
#define ARDUINO_JSONENCODER2_H

#include "Arduino.h"

class JsonEncoder {
public:
    static const char* quote;
    static const char* openBrace;
    static const char* closeBrace;
    static const char* encodedQuote;
    static const char* encodedOpenBrace;
    static const char* encodedCloseBrace;
    static const char* valueSeparator;

    static String quoteString(String value);
    static String encodeQuoteString(String value);
    static String encodeValue(String key, String value);
    static String encodeValue(String key, int value);
    static String encodeValue(String key, float value);
    static String encodeValue(String key, bool value);
    static String wrapValue(String key, String value);
    static String wrapValue(String key, int value);
    static String wrapValue(String key, float value);
    static String wrapValue(String key, bool value);
    static String append(String valueToAppend);
};


#endif //ARDUINO_JSONENCODER2_H
