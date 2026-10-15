/*
 * Ejemplo basico de la libreria BT401_Rafa
 * Configura el modulo y muestra el estado por Serial.
 */

#include <BT401_Rafa.h>

HardwareSerial BTSerial(2);
BT401_Rafa     bt;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("BT401_Rafa - Ejemplo basico"));

    // Inicializar con pines (TX/RX del ESP32)
    if (!bt.begin(BTSerial, 115200, 17, 16)) {
        Serial.println(F("ERROR: no se pudo inicializar el BT401"));
        while (1);
    }
    Serial.println(F("BT401 inicializado"));

    // Configurar el modulo
    bt.setVolume(30);
    bt.setMode(BT401_Rafa::MODE_BLUETOOTH);
    bt.setEDRName("AUDIO_RAFA");
    bt.setBLEName("RAFA_BLE");
    bt.setSimplePassword(true);
    bt.setCallFunction(true);
    bt.setAudioFunction(true);
    bt.setBLE(true);
    bt.setEDR(true);
    bt.setAutoMode(BT401_Rafa::AUTO_BT);

    Serial.println(F("Configuracion enviada"));
}

unsigned long lastQuery = 0;

void loop() {
    // Procesar todos los mensajes del BT401
    bt.processIncoming();

    // Cada 2 segundos, consultar el estado
    if (millis() - lastQuery > 2000) {
        lastQuery = millis();
        bt.queryVolume();
        bt.queryMode();
        bt.queryBtStatus();
    }

    // Mostrar cuando llegue data nueva
    if (bt.hasNewData()) {
        BT401_Rafa::Status st = bt.getStatus();
        Serial.print(F("Modo=")); Serial.print(bt.modeName());
        Serial.print(F(" BT="));  Serial.print(bt.btStatusName());
        Serial.print(F(" Vol=")); Serial.print(st.btVolume);
        if (st.fileName[0]) {
            Serial.print(F(" File=")); Serial.print(st.fileName);
        }
        Serial.println();
        bt.clearNewData();
    }
}