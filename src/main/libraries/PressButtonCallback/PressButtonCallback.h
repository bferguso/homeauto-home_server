//
//  Callback to write to a digital pin
//

#ifndef ARDUINO_PRESSBUTTONCALLBACK_H
#define ARDUINO_PRESSBUTTONCALLBACK_H
#include "RemoteServerCallback.h"

class PressButtonCallback : public RemoteServerCallback {
public:
    PressButtonCallback(int pinNumber, uint8_t newState);
    String execute();

private:
    int _pinNumber;
    uint8_t _newState;
};
#endif //ARDUINO_PRESSBUTTONCALLBACK_H
