/*
 * ============================================================================
 *   BT401_Rafa - Implementacion
 * ============================================================================
 *   Ver BT401_Rafa.h para documentacion completa.
 * ============================================================================
 */

#include "BT401_Rafa.h"

// ============================================================================
//   CONSTRUCTOR
// ============================================================================
BT401_Rafa::BT401_Rafa()
{
    _s = nullptr;
}

// ============================================================================
//   INICIALIZACION
// ============================================================================
bool BT401_Rafa::begin(Stream &s)
{
    _s = &s;
    if (_s == nullptr) return false;

    _rxBuffer.reserve(256);
    _lastLine.reserve(128);

    flushInput();
    return true;
}

bool BT401_Rafa::begin(HardwareSerial &s, uint32_t baud,
                        int8_t rxPin, int8_t txPin)
{
    if (rxPin >= 0 && txPin >= 0) {
        s.begin(baud, SERIAL_8N1, rxPin, txPin);
    } else {
        s.begin(baud);
    }
    return begin((Stream &)s);
}

// ============================================================================
//   UTILIDADES
// ============================================================================
void BT401_Rafa::flushInput()
{
    if (_s == nullptr) return;
    while (_s->available()) _s->read();
}

void BT401_Rafa::copyString(const String &src, char *dst, size_t dstSize)
{
    if (dstSize == 0) return;
    size_t len = src.length();
    if (len >= dstSize) len = dstSize - 1;
    memcpy(dst, src.c_str(), len);
    dst[len] = 0;
}

// ============================================================================
//   ENVIO DE COMANDOS
// ============================================================================
void BT401_Rafa::sendAT(const char *cmd)
{
    if (_s == nullptr) return;
    _s->print("AT+");
    _s->print(cmd);
    _s->print("\r\n");
}

void BT401_Rafa::sendAT(const char *cmd, const char *payload)
{
    if (_s == nullptr) return;
    _s->print("AT+");
    _s->print(cmd);
    if (payload != nullptr) _s->print(payload);
    _s->print("\r\n");
}

void BT401_Rafa::sendRawCommand(const char *cmd)
{
    if (_s == nullptr) return;
    _s->print(cmd);
    _s->print("\r\n");
}

// ============================================================================
//   COMANDOS GENERALES
// ============================================================================
void BT401_Rafa::setVolume(uint8_t vol)
{
    if (vol > BT401_VOLUME_MAX) vol = BT401_VOLUME_MAX;
    char buf[4];
    if (vol < 10) snprintf(buf, sizeof(buf), "0%u", vol);
    else          snprintf(buf, sizeof(buf), "%u",  vol);
    sendAT("CA", buf);
}

void BT401_Rafa::playPause()   { sendAT("CB"); }
void BT401_Rafa::next()        { sendAT("CC"); }
void BT401_Rafa::last()        { sendAT("CD"); }
void BT401_Rafa::volumeUp()    { sendAT("CE"); }
void BT401_Rafa::volumeDown()  { sendAT("CF"); }

void BT401_Rafa::setMode(Mode mode)
{
    if (mode == MODE_NEXT) { sendAT("CM00"); return; }
    if (mode > MODE_OFF) return;
    char buf[4];
    snprintf(buf, sizeof(buf), "0%u", (uint8_t)mode);
    sendAT("CM", buf);
}

void BT401_Rafa::nextMode()
{
    sendAT("CM00");
}

void BT401_Rafa::setMute(MuteMode mode)
{
    if (mode > MUTE_TOGGLE) return;
    char buf[4];
    snprintf(buf, sizeof(buf), "0%u", (uint8_t)mode);
    sendAT("CU", buf);
}

void BT401_Rafa::setDAC(bool on)
{
    sendAT("CS", on ? "01" : "00");
}

void BT401_Rafa::setBaudRate(BaudCode code)
{
    if (code < BAUD_9600 || code > BAUD_256000) return;
    char buf[4];
    snprintf(buf, sizeof(buf), "0%u", (uint8_t)code);
    sendAT("CT", buf);
}

void BT401_Rafa::setAutoMode(AutoMode mode)
{
    if (mode > AUTO_DEVICE) return;
    char buf[4];
    snprintf(buf, sizeof(buf), "0%u", (uint8_t)mode);
    sendAT("CP", buf);
}

void BT401_Rafa::setAutoReport(bool on)         { sendAT("CR", on ? "01" : "00"); }
void BT401_Rafa::setSingleTrigger(bool on)      { sendAT("CJ", on ? "01" : "00"); }
void BT401_Rafa::setPromptSound(bool on)        { sendAT("CN", on ? "01" : "00"); }
void BT401_Rafa::setAutoSwitchToBT(bool on)     { sendAT("CK", on ? "01" : "00"); }
void BT401_Rafa::setBluetoothBackground(bool on){ sendAT("CG", on ? "01" : "00"); }

void BT401_Rafa::reset()        { sendAT("CZ"); }
void BT401_Rafa::factoryReset() { sendAT("CW"); }

// ============================================================================
//   COMANDOS DE MUSICA
// ============================================================================
void BT401_Rafa::playControl(PlayControl cmd)
{
    if ((uint8_t)cmd > AA_DELETE) return;
    char buf[4];
    snprintf(buf, sizeof(buf), "0%u", (uint8_t)cmd);
    sendAT("AA", buf);
}

void BT401_Rafa::setPlayMode(PlayMode mode)
{
    if ((uint8_t)mode > PLAY_REPEAT_FOLDER) return;
    char buf[4];
    snprintf(buf, sizeof(buf), "0%u", (uint8_t)mode);
    sendAT("AC", buf);
}

void BT401_Rafa::playFileNum(uint32_t number)
{
    char buf[12];
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)number);
    sendAT("AB", buf);
}

void BT401_Rafa::playFolderLoop(const char *path)  { sendAT("AF", path); }
void BT401_Rafa::playSpecFile(const char *path)    { sendAT("AJ", path); }
void BT401_Rafa::delCurFile()                      { sendAT("AA08"); }

// ============================================================================
//   COMANDOS BLUETOOTH
// ============================================================================
void BT401_Rafa::setEDRName(const char *name)      { sendAT("BD", name); }
void BT401_Rafa::setEDRPassword(const char *pass)  { sendAT("BE", pass); }
void BT401_Rafa::setBLEName(const char *name)      { sendAT("BM", name); }
void BT401_Rafa::setEDRMac(const char *mac12hex)   { sendAT("BS", mac12hex); }

void BT401_Rafa::setSimplePassword(bool on) { sendAT("B1", on ? "01" : "00"); }
void BT401_Rafa::setCallFunction(bool on)   { sendAT("B2", on ? "01" : "00"); }
void BT401_Rafa::setAudioFunction(bool on)  { sendAT("B3", on ? "01" : "00"); }
void BT401_Rafa::setBLE(bool on)            { sendAT("B4", on ? "01" : "00"); }
void BT401_Rafa::setEDR(bool on)            { sendAT("B5", on ? "01" : "00"); }

void BT401_Rafa::callOut(const char *phoneNumber) { sendAT("BT", phoneNumber); }

void BT401_Rafa::callControl(CallControl cmd)
{
    if ((uint8_t)cmd > BA_PAUSE_AUDIO) return;
    char buf[4];
    snprintf(buf, sizeof(buf), "0%u", (uint8_t)cmd);
    sendAT("BA", buf);
}

// ============================================================================
//   QUERIES
// ============================================================================
void BT401_Rafa::queryVolume()         { sendAT("QA"); }
void BT401_Rafa::queryBaudRate()       { sendAT("QT"); }
void BT401_Rafa::queryMode()           { sendAT("QM"); }
void BT401_Rafa::queryPromptSound()    { sendAT("QN"); }
void BT401_Rafa::queryAutoSwitch()     { sendAT("QK"); }
void BT401_Rafa::queryBtStatus()       { sendAT("TS"); }
void BT401_Rafa::queryTrackNumber()    { sendAT("M1"); }
void BT401_Rafa::queryTrackTotal()     { sendAT("M2"); }
void BT401_Rafa::queryTrackTotalSec()  { sendAT("MT"); }
void BT401_Rafa::queryTrackCurSec()    { sendAT("MK"); }
void BT401_Rafa::queryPlayStatus()     { sendAT("MP"); }
void BT401_Rafa::queryCurrentDevice()  { sendAT("MD"); }
void BT401_Rafa::queryFileName()       { sendAT("MF"); }
void BT401_Rafa::queryIncomingNumber() { sendAT("TT"); }

// ============================================================================
//   PARSER ASINCRONO
// ============================================================================
void BT401_Rafa::processIncoming()
{
    if (_s == nullptr) return;

    while (_s->available()) {
        char c = (char)_s->read();

        if (c == '\n') {
            _rxBuffer.trim();
            if (_rxBuffer.length() > 0) {
                _lastLine = _rxBuffer;
                parseLine(_rxBuffer);
            }
            _rxBuffer = "";
        } else if (c != '\r') {
            _rxBuffer += c;
            if (_rxBuffer.length() > BT401_LINE_MAX_LEN) {
                _rxBuffer = "";
            }
        }
    }
}

/*
 * Interpreta una linea y actualiza _status.
 * Formatos (datasheet v1.6):
 *   OK
 *   ER+n
 *   QA+nn         volumen 0..30
 *   QT+nn         baud rate code
 *   QM+nn         modo
 *   QN+nn         prompt sound 0/1
 *   QK+nn         auto-switch BT 0/1
 *   TS+nn         estado BT 0..4
 *   TD+nombre     nombre EDR
 *   TA+12hex      MAC EDR
 *   TM+nombre     nombre BLE
 *   TB+12hex      MAC BLE
 *   M1+8hex       pista actual
 *   M2+8hex       total pistas
 *   MC+nn         modo reproduccion
 *   MD+nn         dispositivo actual
 *   MP+nn         estado reproduccion
 *   MT+8hex       duracion total (seg)
 *   MK+8hex       tiempo actual (seg)
 *   MF+nombre     nombre archivo
 *   MV+nn         dispositivo online
 *   MO+8hex       pista terminada
 *   MU+nn         insercion/extraccion
 *   TT+numero     numero entrante
 */
void BT401_Rafa::parseLine(const String &line)
{
    if (line.length() == 0) return;

    // Ignorar ACK y errores basicos
    if (line == "OK") return;
    if (line.startsWith("ER+")) return;

    _status.newDataFlag = true;

    // ---------- Queries generales ----------
    if (line.startsWith("QA+")) {
        _status.btVolume = line.substring(3).toInt();
        return;
    }
    if (line.startsWith("QM+")) {
        _status.mode = line.substring(3).toInt();
        return;
    }
    if (line.startsWith("QT+")) return;   // baud rate
    if (line.startsWith("QN+")) return;   // prompt sound
    if (line.startsWith("QK+")) return;   // auto-switch

    // ---------- Bluetooth ----------
    if (line.startsWith("TS+")) {
        _status.btStatus = line.substring(3).toInt();
        return;
    }
    if (line.startsWith("TD+")) {
        copyString(line.substring(3), _status.edrName, sizeof(_status.edrName));
        return;
    }
    if (line.startsWith("TA+")) {
        copyString(line.substring(3), _status.edrMac, sizeof(_status.edrMac));
        return;
    }
    if (line.startsWith("TM+")) {
        copyString(line.substring(3), _status.bleName, sizeof(_status.bleName));
        return;
    }
    if (line.startsWith("TB+")) {
        copyString(line.substring(3), _status.bleMac, sizeof(_status.bleMac));
        return;
    }
    if (line.startsWith("TT+")) {
        copyString(line.substring(3), _status.phoneNumber, sizeof(_status.phoneNumber));
        return;
    }
    if (line.startsWith("TL+")) return;   // BLE status

    // ---------- Musica ----------
    if (line.startsWith("M1+")) {
        _status.trackNum = strtoul(line.substring(3).c_str(), NULL, 16);
        return;
    }
    if (line.startsWith("M2+")) {
        _status.trackTotal = strtoul(line.substring(3).c_str(), NULL, 16);
        return;
    }
    if (line.startsWith("MC+")) {
        _status.playMode = line.substring(3).toInt();
        return;
    }
    if (line.startsWith("MD+")) {
        _status.currentDevice = line.substring(3).toInt();
        return;
    }
    if (line.startsWith("MP+")) {
        _status.playStatus = line.substring(3).toInt();
        return;
    }
    if (line.startsWith("MT+")) {
        _status.trackTotalSec = strtoul(line.substring(3).c_str(), NULL, 16);
        return;
    }
    if (line.startsWith("MK+")) {
        _status.trackCurSec = strtoul(line.substring(3).c_str(), NULL, 16);
        return;
    }
    if (line.startsWith("MF+")) {
        copyString(line.substring(3), _status.fileName, sizeof(_status.fileName));
        return;
    }
    if (line.startsWith("MV+")) return;   // dispositivo online
    if (line.startsWith("MO+")) return;   // pista terminada
    if (line.startsWith("MU+")) return;   // insercion/extraccion

    // Ignorar lineas desconocidas silenciosamente
}

// ============================================================================
//   HELPERS
// ============================================================================
const char *BT401_Rafa::modeName() const
{
    switch (_status.mode) {
        case MODE_IDLE:      return "IDLE";
        case MODE_BLUETOOTH: return "BT";
        case MODE_UDISK:     return "UDISK";
        case MODE_TFCARD:    return "TFCARD";
        case MODE_MUSIC:     return "MUSIC";
        case MODE_AUX:       return "AUX";
        case MODE_SOUNDCARD: return "SOUNDCARD";
        case MODE_OFF:       return "OFF";
        default:             return "?";
    }
}

const char *BT401_Rafa::btStatusName() const
{
    switch (_status.btStatus) {
        case BT_STANDBY: return "STANDBY";
        case BT_IDLE:    return "IDLE";
        case BT_PLAYING: return "PLAYING";
        case BT_CALLING: return "CALLING";
        case BT_ONPHONE: return "ONPHONE";
        default:         return "ERROR";
    }
}