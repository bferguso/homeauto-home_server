//
// Created by Brett Ferguson on 2020-12-29.
//

#ifndef ARDUINO_REMOTESERVERCALLBACK_H
#define ARDUINO_REMOTESERVERCALLBACK_H

#include <stdint.h>
#include <Arduino.h>

class RemoteServerCallback {
public:
    virtual String execute();
};

#endif //ARDUINO_REMOTESERVERCALLBACK_H
