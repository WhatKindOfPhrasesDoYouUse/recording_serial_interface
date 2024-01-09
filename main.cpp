#include <iostream>
#include <windows.h>
#include <string>

void writeCOM1();
BOOL WINAPI CtrlHandler(_In_ DWORD fdwCtrlType);

bool processInput = true;

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
    writeCOM1();
    return 0;
}

void writeCOM1() {
    HANDLE hSerial = CreateFile("COM1",
                                GENERIC_WRITE,
                                0,
                                0,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                0);

    if (hSerial == INVALID_HANDLE_VALUE || hSerial == nullptr) {
        fprintf(stderr, "Не удалось открыть порт\n");
        return;
    }

    DCB portConfig = { 0 };
    COMMTIMEOUTS timeouts = { 0 };
    DWORD bytesWritten;
    const char* portName = "COM1";

    portConfig.DCBlength = sizeof(portConfig);

    if (!GetCommState(hSerial, &portConfig)) {
        fprintf(stderr, "Ошибка получения состояния порта\n");
        CloseHandle(hSerial);
        return;
    }

    portConfig.BaudRate = CBR_9600;
    portConfig.ByteSize = 8;
    portConfig.StopBits = ONESTOPBIT;
    portConfig.Parity = NOPARITY;

    if (!SetCommState(hSerial, &portConfig)) {
        fprintf(stderr, "Ошибка установки состояния порта\n");
        CloseHandle(hSerial);
        return;
    }

    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        fprintf(stderr, "Ошибка установки таймаутов\n");
        CloseHandle(hSerial);
        return;
    }

    std::string input;
    printf("Введите текст для отправки (для завершения введите 'Ctrl+C'):\n");

    while (processInput) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8001) {
            break;
        }
        std::getline(std::cin, input);

        input += "\n";

        if (!WriteFile(hSerial, input.c_str(), input.size(), &bytesWritten, NULL)) {
            fprintf(stderr, "Ошибка записи в порт\n");
            CloseHandle(hSerial);
            return;
        }

        printf("Отправлено: %s\n", input.c_str());
    }
    CloseHandle(hSerial);
}

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    if (fdwCtrlType == CTRL_C_EVENT) {
        printf("Ctrl+C получен. Выход...\n");
        processInput = false;
        return true;
    }
    return false;
}