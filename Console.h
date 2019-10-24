#pragma once
#include "BaseType.h"
#include <Windows.h>
#include <iomanip>
namespace NF {
    enum LogType {
        Log_Normal,
        Log_Attention,
        Log_Debug,
        Log_Error
    };
    class Console : public Singleton<Console> {
    public:
        friend Singleton<Console>;
        ~Console();
        void Add(const string& log, LogType debugMode = Log_Normal, bool addTime = true);
        void Clear();
        const vector<pair<string, LogType>>& Logs() const;
        bool GetScrollToBottom() const;
        void SetScrollToBottom(bool scroll);
    private:
        Console();
        vector<pair<string, LogType>> m_logs{};
        bool m_scrollToBottom = false;
    };
}
