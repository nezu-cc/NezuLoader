#pragma once
#include "pch.h"

enum class MessageType {
	Debug = 0,
	Info,
	Warning,
	Error,
};

namespace L {
	void MessageV(MessageType type, const char* fmt, va_list args);
	void Message(MessageType type, const char* fmt, ...);
	void Debug(const char* fmt, ...);
	void Info(const char* fmt, ...);
	void Warning(const char* fmt, ...);
	void Error(const char* fmt, ...);
	void Clear();
	void SetHwndToNotify(HWND hwnd);
	
	extern std::mutex lock;
	extern std::vector<std::tuple<MessageType, std::string>> messages;
};
