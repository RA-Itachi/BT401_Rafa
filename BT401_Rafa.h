/*
 * ============================================================================
 *   BT401_Rafa - Libreria completa para el modulo Bluetooth BT401
 * ============================================================================
 *
 *   Descripcion:
 *     Control completo del modulo BT401 via comandos AT: modos de trabajo,
 *     reproduccion de musica (TF/U-Disk), funciones Bluetooth, llamadas,
 *     queries de estado, y parsing asincrono de todos los mensajes que el
 *     modulo envia de forma automatica.
 *
 *   Caracteristicas:
 *     - Envio de comandos AT de forma NO bloqueante
 *     - Parser asincrono que se llama desde el loop()
 *     - Estado completo del modulo en una estructura Status
 *     - Control de todos los modos: Bluetooth, U-Disk, TF-Card, AUX, etc.
 *     - Soporte completo de metadatos (pista, duracion, nombre, etc.)
 *     - Sin dependencias externas (solo Arduino.h)
 *
 *   Basada en:
 *     Datasheet BT401 v1.6 (Qingyue Electronics)
 *     Libreria DFRobot_BT401 original (Eddard Liu, 2020)
 *
 *   Licencia: MIT
 *   Version:  1.0.0
 * ============================================================================
 */

#ifndef BT401_RAFA_H
#define BT401_RAFA_H

#include <Arduino.h>

// ============================================================================
//   CONFIGURACION
// ============================================================================
#define BT401_BAUD_DEFAULT        115200
#define BT401_ACK_TIMEOUT_MS      1000
#define BT401_LINE_MAX_LEN        128
#define BT401_VOLUME_MAX          30
#define BT401_VOLUME_MIN          0

// ============================================================================
//   CLASE PRINCIPAL
// ============================================================================
class BT401_Rafa
{
public:
    // ========================================================================
    //   ENUMS
    // ========================================================================

    /**
     * @brief Modos de trabajo (AT+CM)
     */
    enum Mode : uint8_t {
        MODE_IDLE       = 0x00,
        MODE_BLUETOOTH  = 0x01,
        MODE_UDISK      = 0x02,
        MODE_TFCARD     = 0x03,
        MODE_MUSIC      = 0x04,   // auto-detecta U-Disk o TF
        MODE_AUX        = 0x05,
        MODE_SOUNDCARD  = 0x07,
        MODE_OFF        = 0x08,
        MODE_NEXT       = 0xFF    // AT+CM00 -> siguiente modo
    };

    /**
     * @brief Modos de reproduccion (AT+AC)
     */
    enum PlayMode : uint8_t {
        PLAY_REPEAT_ALL     = 0,   // AT+AC00
        PLAY_REPEAT_DEVICE  = 1,   // AT+AC01
        PLAY_REPEAT_SINGLE  = 2,   // AT+AC02
        PLAY_RANDOM_DEVICE  = 3,   // AT+AC03
        PLAY_REPEAT_FOLDER  = 4    // AT+AC04
    };

    /**
     * @brief Comandos de reproduccion (AT+AA)
     */
    enum PlayControl : uint8_t {
        AA_STOP         = 0,
        AA_PLAY         = 1,
        AA_PAUSE        = 2,
        AA_TOGGLE       = 3,
        AA_FAST_FWD     = 4,
        AA_FAST_REW     = 5,
        AA_NEXT_FOLDER  = 6,
        AA_PREV_FOLDER  = 7,
        AA_DELETE       = 8
    };

    /**
     * @brief Comandos de llamada (AT+BA)
     */
    enum CallControl : uint8_t {
        BA_REDIAL       = 0,
        BA_DISCONNECT   = 1,
        BA_REJECT       = 2,
        BA_HANGUP       = 3,
        BA_ANSWER       = 4,
        BA_SCAN         = 5,
        BA_DISCOVER_ON  = 6,
        BA_DISCOVER_OFF = 7,
        BA_PLAY_AUDIO   = 8,
        BA_PAUSE_AUDIO  = 9
    };

    /**
     * @brief Estados del Bluetooth (AT+TS)
     */
    enum BtStatus : uint8_t {
        BT_STANDBY = 0,   // esperando emparejamiento
        BT_IDLE    = 1,   // conectado pero sin reproducir
        BT_PLAYING = 2,   // reproduciendo
        BT_CALLING = 3,   // llamada entrante/saliente
        BT_ONPHONE = 4,   // en llamada
        BT_ERROR   = 0xFF
    };

    /**
     * @brief Modo de auto-entrada al encender (AT+CP)
     */
    enum AutoMode : uint8_t {
        AUTO_BT      = 0,   // AT+CP00 -> entra en Bluetooth
        AUTO_WAIT    = 1,   // AT+CP01 -> espera comandos
        AUTO_DEVICE  = 2    // AT+CP02 -> reproduce si hay dispositivo
    };

    /**
     * @brief Baud rates soportados (AT+CT)
     */
    enum BaudCode : uint8_t {
        BAUD_9600    = 1,
        BAUD_19200   = 2,
        BAUD_38400   = 3,
        BAUD_57600   = 4,
        BAUD_115200  = 5,
        BAUD_256000  = 6
    };

    /**
     * @brief Mute / DAC (AT+CU / AT+CS)
     */
    enum MuteMode : uint8_t {
        MUTE_OFF     = 0,   // AT+CU00 -> unmute
        MUTE_ON      = 1,   // AT+CU01 -> mute
        MUTE_TOGGLE  = 2    // AT+CU02 -> toggle
    };

    // ========================================================================
    //   ESTRUCTURA DE ESTADO
    // ========================================================================
    /**
     * @brief Estado completo del modulo BT401, actualizado por el parser.
     */
    struct Status {
        // --- Modo y estado ---
        uint8_t  mode          = 0;      // QM+xx
        uint8_t  btStatus      = 0;      // TS+xx
        uint8_t  btVolume      = 0;      // QA+xx (0..30)
        uint8_t  playMode      = 0;      // MC+xx
        uint8_t  playStatus    = 0;      // MP+xx: 0=stop, 1=play, 2=pause
        uint8_t  currentDevice = 0;      // MD+xx: 1=U-Disk, 4=TF

        // --- Pistas ---
        uint32_t trackNum      = 0;      // M1+xxxxxxxx (hex)
        uint32_t trackTotal    = 0;      // M2+xxxxxxxx (hex)
        uint32_t trackTotalSec = 0;      // MT+xxxxxxxx (hex seg)
        uint32_t trackCurSec   = 0;      // MK+xxxxxxxx (hex seg)

        // --- Nombre de archivo ---
        char     fileName[64]  = {0};    // MF+xxxxxxxx

        // --- Bluetooth info ---
        char     edrName[32]   = {0};    // TD+nombre
        char     edrMac[16]    = {0};    // TA+12hex
        char     bleName[32]   = {0};    // TM+nombre
        char     bleMac[16]    = {0};    // TB+12hex
        char     phoneNumber[20] = {0};  // TT+numero

        // --- Flags ---
        bool     newDataFlag   = false;  // true si llego data nueva
    };

public:
    // ========================================================================
    //   CONSTRUCTOR Y SETUP
    // ========================================================================
    BT401_Rafa();

    /**
     * @brief Inicializa con un Stream (Serial1, Serial2, etc.)
     * @param s Referencia al puerto serie
     * @return true si OK
     */
    bool begin(Stream &s);

    /**
     * @brief Inicializa un HardwareSerial con pines y baud rate.
     * @param s       HardwareSerial
     * @param baud    Baud rate (115200 recomendado)
     * @param rxPin   Pin RX (opcional, -1 si no se usa)
     * @param txPin   Pin TX (opcional, -1 si no se usa)
     * @return true si OK
     */
    bool begin(HardwareSerial &s, uint32_t baud,
               int8_t rxPin = -1, int8_t txPin = -1);

    // ========================================================================
    //   PARSER ASINCRONO (llamar en el loop)
    // ========================================================================
    /**
     * @brief Procesa todos los bytes disponibles en el puerto serie.
     *        NO ES BLOQUEANTE. Llamar frecuentemente desde loop().
     */
    void processIncoming();

    /**
     * @brief Devuelve el estado actual parseado
     */
    Status getStatus() const { return _status; }

    /**
     * @brief Indica si llego nueva data desde la ultima consulta
     */
    bool hasNewData() const { return _status.newDataFlag; }

    /**
     * @brief Limpia el flag de nueva data
     */
    void clearNewData() { _status.newDataFlag = false; }

    /**
     * @brief Devuelve la ultima linea completa recibida (debug)
     */
    String getLastLine() const { return _lastLine; }

    // ========================================================================
    //   COMANDOS GENERALES (AT+C?)
    // ========================================================================

    /** @brief Envia un comando AT en crudo (avanzado) */
    void sendRawCommand(const char *cmd);

    /** @brief AT+CAxx - Fija el volumen (0..30) */
    void setVolume(uint8_t vol);

    /** @brief AT+CB - Play/Pause */
    void playPause();

    /** @brief AT+CC - Siguiente pista */
    void next();

    /** @brief AT+CD - Pista anterior */
    void last();

    /** @brief AT+CE - Subir volumen */
    void volumeUp();

    /** @brief AT+CF - Bajar volumen */
    void volumeDown();

    /** @brief AT+CMxx - Cambiar modo */
    void setMode(Mode mode);

    /** @brief AT+CM00 - Siguiente modo */
    void nextMode();

    /** @brief AT+CU00/01/02 - Mute */
    void setMute(MuteMode mode);

    /** @brief AT+CS00/01 - DAC off/on */
    void setDAC(bool on);

    /** @brief AT+CT0x - Baud rate (efectivo tras reset) */
    void setBaudRate(BaudCode code);

    /** @brief AT+CP0x - Modo auto-entrada al encender */
    void setAutoMode(AutoMode mode);

    /** @brief AT+CR00/01 - Auto-feedback on/off */
    void setAutoReport(bool on);

    /** @brief AT+CJ00/01 - Reproduccion por trigger o loop */
    void setSingleTrigger(bool on);

    /** @brief AT+CN00/01 - Prompt sounds on/off */
    void setPromptSound(bool on);

    /** @brief AT+CK00/01 - Auto-switch a BT desde TF */
    void setAutoSwitchToBT(bool on);

    /** @brief AT+CG00/01 - Bluetooth en background */
    void setBluetoothBackground(bool on);

    /** @brief AT+CZ - Reset del chip (efectivo inmediato) */
    void reset();

    /** @brief AT+CW - Restaurar configuracion de fabrica */
    void factoryReset();

    // ========================================================================
    //   COMANDOS DE MUSICA (AT+A?)
    // ========================================================================

    /** @brief AT+AA0x - Control de reproduccion TF/U-Disk */
    void playControl(PlayControl cmd);

    /** @brief AT+AC0x - Modo de reproduccion */
    void setPlayMode(PlayMode mode);

    /** @brief AT+ABxxx - Reproducir numero de pista fisico */
    void playFileNum(uint32_t number);

    /** @brief AT+AF/ruta/*.??? - Loop de carpeta */
    void playFolderLoop(const char *path);

    /** @brief AT+AJ/ruta/archivo.??? - Reproducir archivo especifico */
    void playSpecFile(const char *path);

    /** @brief AT+AA08 - Borrar archivo actual */
    void delCurFile();

    // ========================================================================
    //   COMANDOS BLUETOOTH (AT+B?)
    // ========================================================================

    /** @brief AT+BDnombre - Fijar nombre EDR (Bluetooth audio) */
    void setEDRName(const char *name);

    /** @brief AT+BEnnnn - Fijar contrasena EDR */
    void setEDRPassword(const char *pass);

    /** @brief AT+BMnombre - Fijar nombre BLE */
    void setBLEName(const char *name);

    /** @brief AT+BSxxxxxxxxxxxx - Fijar MAC EDR (12 hex chars) */
    void setEDRMac(const char *mac12hex);

    /** @brief AT+B100/01 - Password simple on/off */
    void setSimplePassword(bool on);

    /** @brief AT+B200/01 - Funcion de llamada on/off */
    void setCallFunction(bool on);

    /** @brief AT+B300/01 - Audio Bluetooth (A2DP) on/off */
    void setAudioFunction(bool on);

    /** @brief AT+B400/01 - BLE on/off */
    void setBLE(bool on);

    /** @brief AT+B500/01 - EDR on/off */
    void setEDR(bool on);

    /** @brief AT+BTnnnn - Marcar numero */
    void callOut(const char *phoneNumber);

    /** @brief AT+BAxx - Control de llamadas */
    void callControl(CallControl cmd);

    // ========================================================================
    //   QUERIES (AT+Q? y AT+M?, AT+T?)
    // ========================================================================

    /** @brief AT+QA - Query volumen (respuesta asincrona) */
    void queryVolume();

    /** @brief AT+QT - Query baud rate */
    void queryBaudRate();

    /** @brief AT+QM - Query modo */
    void queryMode();

    /** @brief AT+QN - Query prompt sounds */
    void queryPromptSound();

    /** @brief AT+QK - Query auto-switch BT */
    void queryAutoSwitch();

    /** @brief AT+TS - Query estado Bluetooth */
    void queryBtStatus();

    /** @brief AT+M1 - Query pista actual */
    void queryTrackNumber();

    /** @brief AT+M2 - Query total de pistas */
    void queryTrackTotal();

    /** @brief AT+MT - Query duracion total (segundos) */
    void queryTrackTotalSec();

    /** @brief AT+MK - Query tiempo actual (segundos) */
    void queryTrackCurSec();

    /** @brief AT+MP - Query estado de reproduccion */
    void queryPlayStatus();

    /** @brief AT+MD - Query dispositivo actual */
    void queryCurrentDevice();

    /** @brief AT+MF - Query nombre del archivo actual */
    void queryFileName();

    /** @brief AT+TT - Query numero entrante */
    void queryIncomingNumber();

    // ========================================================================
    //   HELPERS
    // ========================================================================

    /**
     * @brief Devuelve el nombre del modo actual como string
     */
    const char *modeName() const;

    /**
     * @brief Devuelve el nombre del estado BT actual como string
     */
    const char *btStatusName() const;

private:
    // ========================================================================
    //   INTERNOS
    // ========================================================================

    /**
     * @brief Envia "AT+<cmd>\r\n" al modulo
     */
    void sendAT(const char *cmd);

    /**
     * @brief Envia "AT+<cmd><payload>\r\n" al modulo
     */
    void sendAT(const char *cmd, const char *payload);

    /**
     * @brief Parsea una linea recibida y actualiza _status
     */
    void parseLine(const String &line);

    /**
     * @brief Limpia el buffer de entrada
     */
    void flushInput();

    /**
     * @brief Copia un String a un buffer char[]
     */
    static void copyString(const String &src, char *dst, size_t dstSize);

private:
    Stream        *_s;
    Status         _status;
    String         _rxBuffer;
    String         _lastLine;
};

#endif // BT401_RAFA_H