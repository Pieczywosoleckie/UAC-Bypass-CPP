#include <windows.h>

#include <exports.h>
#include <ExampleThread.h>

void Example() {
    HANDLE hThread = CreateThread(nullptr, 0, ExampleThread, nullptr, 0, nullptr);
    if (hThread) {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }
}