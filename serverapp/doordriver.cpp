//
// This file is part of DoorOpener.
// Copyright (c) 2014-2015 Jacob Dawid <jacob@omg-it.works>
//
// DoorOpener is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// DoorOpener is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public
// License along with DoorOpener.
// If not, see <http://www.gnu.org/licenses/>.
//

// Own includes
#include "doordriver.h"

// Wiring PI
#include <wiringPi.h>

// Qt includes
#include <QFile>

#define GPIO_DOOR(x) (wpiPinToGpio(x))
#define GPIO_RING(x) (wpiPinToGpio(x))

DoorDriver& DoorDriver::instance() {
    static DoorDriver doorService;
    return doorService;
}

DoorDriver::DoorDriver(QObject *parent) :
    QObject(parent) {

    wiringPiSetupGpio();

    pinMode(GPIO_DOOR(37), OUTPUT);
    pinMode(GPIO_RING(38), INPUT);
    pullUpDnControl(GPIO_RING(38), PUD_DOWN);

    _openDoorHoldTimer = new QTimer(this);
    _openDoorHoldTimer->setSingleShot(true);
    connect(_openDoorHoldTimer, SIGNAL(timeout()),
            this, SLOT(close()));

    _doorSemaphore = new QSemaphore(1);

    _ringPollTimer = new QTimer(this);
    _ringPollTimer->setInterval(50);
    connect(_ringPollTimer, SIGNAL(timeout()),
            this, SLOT(ringPoll()));
    _ringPollTimer->start();
}

DoorDriver::~DoorDriver() {
}

void DoorDriver::open(int holdDuration) {
    _doorSemaphore->acquire();
    if(!_openDoorHoldTimer->isActive()) {
        digitalWrite(GPIO_DOOR(37), HIGH);

        if(digitalRead(GPIO_DOOR(37))) {
            emit opened();
            system("espeak \"Door is opening.\" -p 99");
            _openDoorHoldTimer->setInterval(holdDuration);
            _openDoorHoldTimer->start();
        }
    }
    _doorSemaphore->release();
}

void DoorDriver::simulateRing() {
    emit ring();
}

void DoorDriver::close() {
    _doorSemaphore->acquire();
    digitalWrite(GPIO_DOOR(37), LOW);

    if(!digitalRead(GPIO_DOOR(37))) {
        emit closed();
        _openDoorHoldTimer->stop();
    }
    _doorSemaphore->release();
}

void DoorDriver::ringPoll() {
    if(!digitalRead(GPIO_RING(38))) {
      emit ring();
    }
}
