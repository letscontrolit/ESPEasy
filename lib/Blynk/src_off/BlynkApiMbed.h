/**
 * @file       BlynkApiMbed.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 *
 */

#ifndef BlynkApiMbed_h
#define BlynkApiMbed_h

#include <Blynk/BlynkApi.h>
#include <mbed.h>

#ifdef BLYNK_NO_INFO

template<class Proto>
BLYNK_FORCE_INLINE
void BlynkApi<Proto>::sendInfo(void) {}

#else

template<class Proto>
BLYNK_FORCE_INLINE
void BlynkApi<Proto>::sendInfo(void)
{
    static const char profile[] BLYNK_PROGMEM = "blnkinf\0"
        BLYNK_PARAM_KV("ver"    , BLYNK_VERSION)
        BLYNK_PARAM_KV("h-beat" , BLYNK_TOSTRING(BLYNK_HEARTBEAT))
        BLYNK_PARAM_KV("buff-in", BLYNK_TOSTRING(BLYNK_MAX_READBYTES))
#ifdef BLYNK_INFO_DEVICE
        BLYNK_PARAM_KV("dev"    , BLYNK_INFO_DEVICE)
#endif
#ifdef BLYNK_INFO_CPU
        BLYNK_PARAM_KV("cpu"    , BLYNK_INFO_CPU)
#endif
#ifdef BLYNK_INFO_CONNECTION
        BLYNK_PARAM_KV("con"    , BLYNK_INFO_CONNECTION)
#endif
#ifdef BOARD_FIRMWARE_VERSION
        BLYNK_PARAM_KV("fw"     , BOARD_FIRMWARE_VERSION)
#endif
        BLYNK_PARAM_KV("build"  , __DATE__ " " __TIME__)
        "\0"
    ;
    const size_t profile_len = sizeof(profile)-8-2;

    char mem_dyn[64];
    BlynkParam profile_dyn(mem_dyn, 0, sizeof(mem_dyn));
#ifdef BOARD_TEMPLATE_ID
    profile_dyn.add_key("tmpl", BOARD_TEMPLATE_ID);
#endif

#ifdef BLYNK_HAS_PROGMEM
    char mem[profile_len];
    memcpy_P(mem, profile+8, profile_len);
    static_cast<Proto*>(this)->sendCmd(BLYNK_CMD_INTERNAL, 0, mem, profile_len, profile_dyn.getBuffer(void), profile_dyn.getLength(void));
#else
    static_cast<Proto*>(this)->sendCmd(BLYNK_CMD_INTERNAL, 0, profile+8, profile_len, profile_dyn.getBuffer(void), profile_dyn.getLength(void));
#endif
    return;
}

#endif


// Check if analog pins can be referenced by name on this device
#if defined(analogInputToDigitalPin)
    #define BLYNK_DECODE_PIN(it) (((it).asStr(void)[0] == 'A') ? analogInputToDigitalPin(atoi((it).asStr(void)+1)) : (it).asInt(void))
#else
    #define BLYNK_DECODE_PIN(it) ((it).asInt(void))

    #if defined(BLYNK_DEBUG_ALL)
        #pragma message "analogInputToDigitalPin not defined"
    #endif
#endif

template<class Proto>
BLYNK_FORCE_INLINE
void BlynkApi<Proto>::processCmd(const void* buff, size_t len)
{
    BlynkParam param((void*)buff, len);
    BlynkParam::iterator it = param.begin(void);
    if (it >= param.end(void))
        return;
    const char* cmd = it.asStr(void);
    uint16_t cmd16;
    memcpy(&cmd16, cmd, sizeof(cmd16));
    if (++it >= param.end(void))
        return;

    uint8_t pin = BLYNK_DECODE_PIN(it);

    switch(cmd16) {

#ifndef BLYNK_NO_BUILTIN

    case BLYNK_HW_PM: {
        while (it < param.end(void)) {
            pin = BLYNK_DECODE_PIN(it);
            ++it;
            if (!strcmp(it.asStr(void), "in")) {
                //pinMode(pin, INPUT);
            } else if (!strcmp(it.asStr(void), "out") || !strcmp(it.asStr(void), "pwm")) {
                //pinMode(pin, OUTPUT);
            } else {
#ifdef BLYNK_DEBUG
                BLYNK_LOG4(BLYNK_F("Invalid pin "), pin, BLYNK_F(" mode "), it.asStr(void));
#endif
            }
            ++it;
        }
    } break;
    case BLYNK_HW_DR: {
        DigitalIn p((PinName)pin);
        char mem[16];
        BlynkParam rsp(mem, 0, sizeof(mem));
        rsp.add("dw");
        rsp.add(pin);
        rsp.add(int(p));
        static_cast<Proto*>(this)->sendCmd(BLYNK_CMD_HARDWARE, 0, rsp.getBuffer(void), rsp.getLength(void)-1);
    } break;
    case BLYNK_HW_DW: {
        // Should be 1 parameter (value)
        if (++it >= param.end(void))
            return;

        //BLYNK_LOG("digitalWrite %d -> %d", pin, it.asInt(void));
        DigitalOut p((PinName)pin);
        p = it.asInt(void) ? 1 : 0;
    } break;
    case BLYNK_HW_AR: {
        AnalogIn p((PinName)pin);
        char mem[16];
        BlynkParam rsp(mem, 0, sizeof(mem));
        rsp.add("aw");
        rsp.add(pin);
        rsp.add(int(p.read(void) * 1024));
        static_cast<Proto*>(this)->sendCmd(BLYNK_CMD_HARDWARE, 0, rsp.getBuffer(void), rsp.getLength(void)-1);
    } break;
    case BLYNK_HW_AW: {
        // TODO: Not supported yet
    } break;

#endif

    case BLYNK_HW_VR: {
        BlynkReq req = { pin };
        WidgetReadHandler handler = GetReadHandler(pin);
        if (handler && (handler != BlynkWidgetRead)) {
            handler(req);
        } else {
            BlynkWidgetReadDefault(req);
        }
    } break;
    case BLYNK_HW_VW: {
        ++it;
        char* start = (char*)it.asStr(void);
        BlynkParam param2(start, len - (start - (char*)buff));
        BlynkReq req = { pin };
        WidgetWriteHandler handler = GetWriteHandler(pin);
        if (handler && (handler != BlynkWidgetWrite)) {
            handler(req, param2);
        } else {
            BlynkWidgetWriteDefault(req, param2);
        }
    } break;
    default:
        BLYNK_LOG2(BLYNK_F("Invalid HW cmd: "), cmd);
        static_cast<Proto*>(this)->sendCmd(BLYNK_CMD_RESPONSE, static_cast<Proto*>(this)->msgIdOutOverride, NULL, BLYNK_ILLEGAL_COMMAND);
    }
}

#endif
