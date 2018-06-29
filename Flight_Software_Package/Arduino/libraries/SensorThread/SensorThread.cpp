#include "SensorThread.h"

void SensorThread::run() {
    callback();

    runned();
}

void GPSSensorThread::callback() {
    // read from sensor and store in buffer
}
