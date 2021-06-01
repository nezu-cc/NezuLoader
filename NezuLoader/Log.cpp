#include "pch.h"
#include "Log.h"

std::mutex L::lock;
std::vector<std::tuple<MessageType, std::string>> L::messages;
bool suppressed = false;

void L::MessageV(MessageType type, const char* fmt, va_list args) {
    if (suppressed)
        return;
    SIZE_T bufsz = vsnprintf(NULL, 0, fmt, args) + 1;
    char* buf = new char[bufsz];
    vsnprintf(buf, bufsz, fmt, args);
    std::string str(buf, bufsz);
    lock.lock();
    messages.push_back(std::make_tuple(type, str));
    lock.unlock();
    delete[] buf;
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

void L::SuppressAllMessages(bool suppress) {
    suppressed = suppress;
}
