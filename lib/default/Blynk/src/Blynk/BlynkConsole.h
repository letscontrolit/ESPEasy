/**
 * @file       BlynkConsole.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2020 Volodymyr Shymanskyy
 * @date       Oct 2020
 * @brief      Console Utility
 *
 */

#ifndef BlynkConsole_h
#define BlynkConsole_h

#define BLYNK_CONSOLE_MAX_COMMANDS 64
#define BLYNK_CONSOLE_INPUT_BUFFER 256
#define BLYNK_CONSOLE_USE_STREAM
#define BLYNK_CONSOLE_USE_LAMBDAS

#ifdef BLYNK_CONSOLE_USE_LAMBDAS
#include <functional>
#endif

#ifdef BLYNK_CONSOLE_USE_STREAM
#include <stdarg.h>
#endif

class BlynkConsole
{
private:

#ifdef BLYNK_CONSOLE_USE_LAMBDAS
    typedef std::function<void(void)> HandlerSimp;
    typedef std::function<void(int argc, const char** argv)> HandlerArgs;
#else
    typedef void (*HandlerSimp)();
    typedef void (*HandlerArgs)(int argc, const char** argv);
#endif
    enum HandlerType {
        SIMPLE,
        WITH_ARGS
    };

    class CmdHandler {
    public:
        const char* cmd;
        HandlerType type;
        union {
            HandlerSimp* f_simp;
            HandlerArgs* f_args;
        };
        CmdHandler() = default;
        CmdHandler(const char* s, HandlerSimp* f)
            : cmd(s), type(SIMPLE), f_simp(f)
        {}
        CmdHandler(const char* s, HandlerArgs* f)
            : cmd(s), type(WITH_ARGS), f_args(f)
        {}
    };

public:
    
    enum ProcessResult {
        PROCESSED,
        SKIPPED,
        EXECUTED,
        NOT_FOUND,
    };

    BlynkConsole() {
        reset_buff();

#if defined(BLYNK_CONSOLE_USE_STREAM) && defined(BLYNK_CONSOLE_USE_LAMBDAS)
        HandlerSimp help = [=]() {
            stream->print("Available commands: ");
            for (size_t i=0; i<commandsQty; i++) {
                CmdHandler& handler = commands[i];
                stream->print(handler.cmd);
                if (i < commandsQty-1) { stream->print(", "); }
            }
            stream->println();
        };
        
        addCommand("help", help);
        addCommand("?", help);
#endif

    }

#ifdef BLYNK_CONSOLE_USE_STREAM
    template <typename T>
    void print(T val) {
        stream->print(val);
    }

    void printf(char *fmt, ... ) {
        char buf[256];
        va_list args;
        va_start (args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end (args);
        stream->print(buf);
    }
#endif

    void addCommand(const char* cmd, HandlerSimp h) {
        if (commandsQty >= BLYNK_CONSOLE_MAX_COMMANDS) return;
        commands[commandsQty++] = CmdHandler(cmd, new HandlerSimp(h));
    }

    void addCommand(const char* cmd, HandlerArgs h) {
        if (commandsQty >= BLYNK_CONSOLE_MAX_COMMANDS) return;
        commands[commandsQty++] = CmdHandler(cmd, new HandlerArgs(h));
    }

    ProcessResult process(char c) {
        if (cmdPtr >= cmdBuff+sizeof(cmdBuff)) {
            reset_buff();
        }
      
        *(cmdPtr++) = c;
        if (c == '\n' || c == '\r') {
            ProcessResult ret = runCommand(cmdBuff);
            reset_buff();
            return ret;
        }
        return PROCESSED;
    }

    ProcessResult runCommand(char* cmd) {
        char* argv[8] = { 0, };
        int argc = split_argv(cmd, argv);
        if (argc <= 0) {
            return SKIPPED;
        }
#ifdef BLYNK_CONSOLE_USE_STREAM
        if (stream) stream->println();
#endif
        for (size_t i=0; i<commandsQty; i++) {
            CmdHandler& handler = commands[i];
            if (!strncasecmp(argv[0], handler.cmd, strlen(handler.cmd)+1)) {
                switch (handler.type) {
                case SIMPLE:
                    (*(handler.f_simp))();
                    break;
                case WITH_ARGS:
                    (*(handler.f_args))(argc-1, (const char**)(argv+1));
                    break;
                }
                return EXECUTED;
            }
        }
        return NOT_FOUND;
    }
    
#ifdef BLYNK_CONSOLE_USE_STREAM
    void init(Stream& s) {
        stream = &s;
    }
    
    void run() {
        while (stream && stream->available()) {
            char c = stream->read();
            switch (process(c)) {
            case SKIPPED: break;
            case PROCESSED:
                stream->print(c);
                break;
            case NOT_FOUND:
                stream->println("Command not found.");
            case EXECUTED:
                stream->print(">");
                break;
            }
        }
    }
#endif

private:
    CmdHandler commands[BLYNK_CONSOLE_MAX_COMMANDS];
    unsigned   commandsQty = 0;

    char* cmdPtr;
    char  cmdBuff[BLYNK_CONSOLE_INPUT_BUFFER];

#ifdef BLYNK_CONSOLE_USE_STREAM
    Stream* stream = nullptr;
#endif

    void reset_buff() {
        memset(cmdBuff, 0, sizeof(cmdBuff));
        cmdPtr = cmdBuff;
    }

    static
    void unescape(char* buff)
    {
        char* outp = buff;
        while (*buff) {
            if (*buff == '\\') {
                switch (*(buff+1)) {
                case '0':  *outp++ = '\0'; break;
                case 'b':  *outp++ = '\b'; break;
                case 'n':  *outp++ = '\n'; break;
                case 'r':  *outp++ = '\r'; break;
                case 't':  *outp++ = '\t'; break;
                case 'x': {
                    char hex[3] = { *(buff+2), *(buff+3), '\0' };
                    *outp = strtol(hex, NULL, 16);
                    buff += 2; outp += 1;
                    break;
                }
                // Otherwise just pass the letter
                // Also handles '\\'
                default: *outp++ = *(buff+1); break;
                }
                buff += 2;
            } else {
                *outp++ = *buff++;
            }
        }
        *outp = '\0';
    }
    
    static
    int split_argv(char *str, char** argv)
    {
        int result = 0;
        char* curr = str;
        int len = 0;
        for (int i = 0; str[i] != '\0'; i++) {
            if (strchr(" \n\r\t", str[i])) {
                if (len) {  // Found space after non-space
                    str[i] = '\0';
                    unescape(curr);
                    argv[result++] = curr;
                    len = 0;
                }
            } else {
                if (!len) { // Found non-space after space
                    curr = &str[i];
                }
                len++;
            }
        }
        argv[result] = NULL;
        return result;
    }

};

#endif // BlynkConsole
