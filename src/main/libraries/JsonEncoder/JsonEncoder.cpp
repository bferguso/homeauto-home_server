//
// Created by Brett Ferguson on 2020-12-30.
//
#include "JsonEncoder.h"

const char *JsonEncoder::quote = "\"";
const char *JsonEncoder::openBrace = "{";
const char *JsonEncoder::closeBrace = "}";
const char *JsonEncoder::valueSeparator = ",";

const char *JsonEncoder::encodedQuote = "%22";
const char *JsonEncoder::encodedOpenBrace = "%7b";
const char *JsonEncoder::encodedCloseBrace = "%7d";

String JsonEncoder::append(String valueToAppend)
{
    return valueSeparator+valueToAppend;
}

String JsonEncoder::quoteString(String value)
{
    return quote+value+quote;
}

String JsonEncoder::encodeQuoteString(String value)
{
    return encodedQuote+value+encodedQuote;
}

String JsonEncoder::encodeValue(String key, String value)
{
    return encodeQuoteString(key)+":"+encodeQuoteString(value);
}

String JsonEncoder::encodeValue(String key, int value)
{
    return encodeQuoteString(key)+":"+value;
}

String JsonEncoder::encodeValue(String key, float value)
{
    return encodeQuoteString(key)+":"+value;
}

String JsonEncoder::encodeValue(String key, bool value)
{
    return encodeQuoteString(key)+":"+value;
}

String JsonEncoder::wrapValue(String key, String value)
{
    return quoteString(key)+":"+quoteString(value);
}

String JsonEncoder::wrapValue(String key, int value)
{
    return quoteString(key)+":"+value;
}

String JsonEncoder::wrapValue(String key, float value)
{
    return quoteString(key)+":"+value;
}

String JsonEncoder::wrapValue(String key, bool value)
{
    return quoteString(key)+":"+value;
}
