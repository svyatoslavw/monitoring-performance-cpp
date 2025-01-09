#include <fstream>
#include <vector>
#include <locale>
#include <string>
#include <windows.h>

class Logger {
public:
    Logger() : isLogging(false) {}

    void StartLogging() {
        if (!isLogging) {
            isLogging = true;
        }
    }

    void StopLogging(HWND hDlg, const std::vector<int>& loadHistory, const std::wstring& fileName) {
        if (isLogging) {
            isLogging = false;

            std::wofstream logFile(fileName, std::ios::out | std::ios::app);
            logFile.imbue(std::locale("uk_UA.UTF-8"));

            if (logFile.is_open()) {
                for (const auto& entry : loadHistory) {
                    logFile << L"Відсоток використання пам'яті: " << entry << L"%" << std::endl;
                }
                logFile.close();

                MessageBox(hDlg, L"Дані успішно записані у файл.", L"Інформація", MB_OK);
            }
        }
    }

private:
    bool isLogging;
};
