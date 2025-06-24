#pragma once

namespace rlc {

class Moteur {
public:
    void init(int pin);
    void ouvrir(); // Ex: tourner à 90°
    void fermer(); // Ex: revenir à 0°
};

}
