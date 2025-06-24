#include "moteur.h"
#include <Arduino.h>
#include <ESP32Servo.h>

namespace rlc {

Servo servo;
int servo_pin;

void Moteur::init(int pin) {
    servo_pin = pin;
    servo.setPeriodHertz(50); // standard servo frequency
    servo.attach(servo_pin);
}

void Moteur::ouvrir() {
    servo.write(90); // position ouverte
    delay(800);      // délai pour laisser le servo bouger
}

void Moteur::fermer() {
    servo.write(0);  // position fermée
    delay(800);      // délai pour laisser le servo bouger
}

}
