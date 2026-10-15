/*
 * Ejemplo Monitor - muestra por Serial TODO lo que llega del BT401
 * Util para depurar y ver el trafico completo.
 */

#include <BT401_Rafa.h>

HardwareSerial BTSerial(2);
BT401_Rafa     bt;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("=== BT401_Rafa Monitor ==="));

    bt.begin(BTSerial, 115200, 17, 16);
    bt.setVolume(30);

    // No configuramos mas: dejamos que el modulo hable por si mismo
    Serial.println(F("Escuchando al BT401..."));
}

void loop() {
    bt.processIncoming();
}