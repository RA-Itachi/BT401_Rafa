# BT401_Rafa

Librería Arduino completa para el módulo Bluetooth **BT401** (Tenstar / Qingyue Electronics).

## Características

- Control completo del BT401 vía comandos AT
- **Envío NO bloqueante** de comandos
- **Parser asíncrono** que se llama desde el `loop()`
- Estado completo del módulo en una estructura `Status`
- Soporte de todos los modos: Bluetooth, U-Disk, TF-Card, AUX, SoundCard
- Reproducción de música con control completo (play/pause/next/prev/folder)
- Bluetooth: nombre EDR/BLE, password, MAC, llamadas
- Queries de todo el estado (volumen, modo, pista, tiempo, nombre, etc.)
- **Sin dependencias externas** (solo `Arduino.h`)

## Instalación

1. Copiar la carpeta `BT401_Rafa` a `~/Arduino/libraries/`
2. Reiniciar el IDE de Arduino
3. `#include <BT401_Rafa.h>`

## Uso básico

\`\`\`cpp
#include <BT401_Rafa.h>

HardwareSerial BTSerial(2);
BT401_Rafa     bt;

void setup() {
    Serial.begin(115200);

    // Inicializar con pines (TX/RX del ESP32 -> RX/TX del BT401)
    bt.begin(BTSerial, 115200, 17, 16);

    // Configurar el módulo
    bt.setVolume(30);            // volumen máximo
    bt.setMode(BT401_Rafa::MODE_BLUETOOTH);
    bt.setEDRName("AUDIO_RAFA");
    bt.setBLEName("RAFA_BLE");
    bt.setSimplePassword(true);  // sin contraseña
    bt.setCallFunction(true);
    bt.setAudioFunction(true);
    bt.setBLE(true);
    bt.setEDR(true);
    bt.setAutoMode(BT401_Rafa::AUTO_BT);
}

void loop() {
    // ⚠️ CRÍTICO: llamar siempre, no bloqueante
    bt.processIncoming();

    // Consultar el estado parseado
    BT401_Rafa::Status st = bt.getStatus();

    if (bt.hasNewData()) {
        Serial.printf("Modo=%s BT=%s Vol=%d\n",
                      bt.modeName(),
                      bt.btStatusName(),
                      st.btVolume);
        bt.clearNewData();
    }
}
\`\`\`

## Control de reproducción

\`\`\`cpp
bt.playPause();
bt.next();
bt.last();
bt.playControl(BT401_Rafa::AA_STOP);
bt.setPlayMode(BT401_Rafa::PLAY_REPEAT_ALL);
bt.playFileNum(5);
\`\`\`

## Queries

\`\`\`cpp
bt.queryVolume();
bt.queryMode();
bt.queryBtStatus();
bt.queryTrackNumber();
bt.queryFileName();
// Los resultados llegan al parser y se guardan en getStatus()
\`\`\`

## Licencia

MIT