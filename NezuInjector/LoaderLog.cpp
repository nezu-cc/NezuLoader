#include "pch.h"
#include "LoaderLog.h"

std::mutex L::lock;
std::vector<std::tuple<MessageType, std::string>> L::messages;
HWND notify_hwnd = NULL;

void L::MessageV(MessageType type, const char* fmt, va_list args) {
    SIZE_T bufsz = vsnprintf(NULL, 0, fmt, args) + 1;
    char* buf = new char[bufsz];
    vsnprintf(buf, bufsz, fmt, args);
    std::string str(buf, bufsz);
    lock.lock();
    messages.push_back(std::make_tuple(type, str));
    lock.unlock();
    delete[] buf;
    if (notify_hwnd != NULL) {
        //force render next 10 frames at normal speed even if windows isn't focused
        PostMessage(notify_hwnd, WM_USER, 10, 0);
    }
}

void L::Message(MessageType type, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    MessageV(type, fmt, args);
    va_end(args);
}

void L::Debug(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    MessageV(MessageType::Debug, fmt, args);
    va_end(args);
}

void L::Info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    MessageV(MessageType::Info, fmt, args);
    va_end(args);
}

void L::Warning(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    MessageV(MessageType::Warning, fmt, args);
    va_end(args);
}

void L::Error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    MessageV(MessageType::Error, fmt, args);
    va_end(args);
}

void L::Clear() {
    lock.lock();
    messages.clear();
    lock.unlock();
}

void L::SetHwndToNotify(HWND hwnd) {
    notify_hwnd = hwnd;
}
