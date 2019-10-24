#include "Console.h"
namespace NF {
    Console::Console() {}
    Console::~Console() {}
    const vector<pair<string, LogType>>& Console::Logs() const {
        return m_logs;
    }
    void Console::Add(const string& log, LogType type, bool addTime) {
        string timeStr{};
        if (addTime) {
            SYSTEMTIME time{};
            GetLocalTime(&time);
            stringstream ss{};
            ss << "["
                << std::setw(2) << std::setfill('0') << time.wHour << ":"
                << std::setw(2) << std::setfill('0') << time.wMinute << ":" 
                << std::setw(2) << std::setfill('0') << time.wSecond << "] ";
            timeStr = ss.str();
        }
        m_logs.push_back({ timeStr + log, type });
        m_scrollToBottom = true;
    }
    void Console::Clear() {
        m_logs.clear();
    }
    bool Console::GetScrollToBottom() const {
        return m_scrollToBottom;
    }
    void Console::SetScrollToBottom(bool scroll) {
        m_scrollToBottom = scroll;
    }
}
