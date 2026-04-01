#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <map>
#include <set>
#include <ctime>
#include <chrono>
#include <thread>
#include <shlwapi.h>
#include <aclapi.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "advapi32.lib")


class SimpleMutex {
private:
    CRITICAL_SECTION criticalSection;
public:
    SimpleMutex() {
        InitializeCriticalSection(&criticalSection);
    }
    ~SimpleMutex() {
        DeleteCriticalSection(&criticalSection);
    }
    void lock() {
        EnterCriticalSection(&criticalSection);
    }
    void unlock() {
        LeaveCriticalSection(&criticalSection);
    }
};


class LockGuard {
private:
    SimpleMutex& mutex;
public:
    LockGuard(SimpleMutex& m) : mutex(m) {
        mutex.lock();
    }
    ~LockGuard() {
        mutex.unlock();
    }
};

class AdvancedLogger {
private:
    std::ofstream logFile;
    bool consoleOutput;
    std::string logFileName;
    size_t maxLogSize;
    SimpleMutex logMutex;

    void RotateLogIfNeeded() {
        std::ifstream in(logFileName.c_str(), std::ios::ate | std::ios::binary);
        if (in.is_open()) {
            std::streamoff fileSize = in.tellg();
            size_t size = static_cast<size_t>(fileSize);
            in.close();

            if (size > maxLogSize) {
                std::string backup = logFileName + ".old";
                DeleteFileA(backup.c_str());
                MoveFileA(logFileName.c_str(), backup.c_str());
            }
        }
    }

public:
    AdvancedLogger(bool console = true, size_t maxSize = 5 * 1024 * 1024)
        : consoleOutput(console), maxLogSize(maxSize) {
        logFileName = "spoofer_log.txt";
        logFile.open(logFileName.c_str(), std::ios::app);
        RotateLogIfNeeded();
        Log("=== Advanced Spoofer Started ===");
    }

    ~AdvancedLogger() {
        Log("=== Advanced Spoofer Finished ===");
        if (logFile.is_open()) logFile.close();
    }

    void Log(const std::string& message, bool error = false) {
        LockGuard lock(logMutex);
        std::string prefix = error ? "[ERROR] " : "[INFO] ";
        std::string timestamp = GetTimestamp();
        std::string fullMessage = timestamp + " " + prefix + message;

        if (consoleOutput) {
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            if (error) {
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
                std::cout << fullMessage << std::endl;
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            }
            else {
                std::cout << fullMessage << std::endl;
            }
        }

        if (logFile.is_open()) {
            logFile << fullMessage << std::endl;
            logFile.flush();
        }
    }

private:
    std::string GetTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        struct tm tstruct;
        char buf[80];
        localtime_s(&tstruct, &time_t_now);
        strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S]", &tstruct);

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        std::stringstream ss;
        ss << buf << "." << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
};


class AdvancedRegistryManager {
private:
    AdvancedLogger& logger;

public:
    AdvancedRegistryManager(AdvancedLogger& log) : logger(log) {}

    bool SetRegistryValue(const std::string& path, const std::string& name,
        const std::string& value, HKEY root = HKEY_LOCAL_MACHINE) {
        HKEY hKey;
        std::string fullPath = path;
        DWORD disposition;

        LONG result = RegCreateKeyExA(root, fullPath.c_str(), 0, NULL,
            REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_WOW64_64KEY, NULL, &hKey, &disposition);

        if (result == ERROR_SUCCESS) {
            result = RegSetValueExA(hKey, name.c_str(), 0, REG_SZ,
                (BYTE*)value.c_str(), (DWORD)(value.length() + 1));

            RegCloseKey(hKey);

            if (result == ERROR_SUCCESS) {
                logger.Log("Registry set: " + fullPath + "\\" + name + " = " + value);
                return true;
            }
            else {
                logger.Log("Failed to set registry value: " + fullPath + "\\" + name, true);
            }
        }
        else {
            logger.Log("Failed to open registry key: " + fullPath, true);
        }
        return false;
    }

    bool SetDWORDValue(const std::string& path, const std::string& name, DWORD value,
        HKEY root = HKEY_LOCAL_MACHINE) {
        HKEY hKey;
        std::string fullPath = path;
        DWORD disposition;

        if (RegCreateKeyExA(root, fullPath.c_str(), 0, NULL,
            REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_WOW64_64KEY, NULL, &hKey, &disposition) == ERROR_SUCCESS) {

            LONG result = RegSetValueExA(hKey, name.c_str(), 0, REG_DWORD,
                (BYTE*)&value, sizeof(value));

            RegCloseKey(hKey);
            return result == ERROR_SUCCESS;
        }
        return false;
    }
};


struct GPUModel {
    std::string vendor;
    std::string model;
    std::string deviceID;
};


class AdvancedSpoofer {
private:
    AdvancedLogger logger;
    AdvancedRegistryManager registry;
    std::mt19937 gen;
    std::vector<GPUModel> gpuDatabase;
    std::vector<std::string> backupFiles;

    void InitializeGPUDatabase() {
        // NVIDIA GTX 10 Series
        gpuDatabase.push_back({ "NVIDIA", "GeForce GTX 1050", "1C81" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce GTX 1050 Ti", "1C82" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce GTX 1060 3GB", "1C83" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce GTX 1060 6GB", "1C83" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce GTX 1070", "1C84" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce GTX 1070 Ti", "1C85" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce GTX 1080", "1C86" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce GTX 1080 Ti", "1C87" });

        // NVIDIA GTX 16 Series
        gpuDatabase.push_back({ "NVIDIA", "GeForce GTX 1650", "1F82" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce GTX 1650 SUPER", "1F83" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce GTX 1660", "1F84" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce GTX 1660 SUPER", "1F85" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce GTX 1660 Ti", "1F86" });

        // NVIDIA RTX 20 Series
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 2060", "1F87" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 2060 SUPER", "1F88" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 2070", "1F89" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 2070 SUPER", "1F8A" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 2080", "1F8B" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 2080 SUPER", "1F8C" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 2080 Ti", "1F8D" });

        // NVIDIA RTX 30 Series
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 3050", "1F8E" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 3060", "1F8F" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 3060 Ti", "1F90" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 3070", "1F91" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 3070 Ti", "1F92" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 3080", "1F93" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 3080 Ti", "1F94" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 3090", "1F95" });

        // NVIDIA RTX 40 Series
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 4060", "1F98" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 4060 Ti", "1F99" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 4070", "1F9A" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 4070 Ti", "1F9C" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 4080", "1F9E" });
        gpuDatabase.push_back({ "NVIDIA", "GeForce RTX 4090", "1FA0" });

        // AMD Series
        gpuDatabase.push_back({ "AMD", "Radeon RX 6600", "73FF" });
        gpuDatabase.push_back({ "AMD", "Radeon RX 6600 XT", "73FF" });
        gpuDatabase.push_back({ "AMD", "Radeon RX 6700 XT", "73DF" });
        gpuDatabase.push_back({ "AMD", "Radeon RX 6800", "73BF" });
        gpuDatabase.push_back({ "AMD", "Radeon RX 6800 XT", "73BF" });
        gpuDatabase.push_back({ "AMD", "Radeon RX 6900 XT", "73BF" });
        gpuDatabase.push_back({ "AMD", "Radeon RX 7600", "73EF" });
        gpuDatabase.push_back({ "AMD", "Radeon RX 7700 XT", "73E0" });
        gpuDatabase.push_back({ "AMD", "Radeon RX 7800 XT", "73E1" });
        gpuDatabase.push_back({ "AMD", "Radeon RX 7900 XT", "73E3" });
        gpuDatabase.push_back({ "AMD", "Radeon RX 7900 XTX", "73E4" });
    }

    void GenerateRandomSeed() {
        auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
        seed ^= std::random_device()();
        seed ^= (size_t)this;
        gen.seed(static_cast<unsigned int>(seed));
    }

    std::string GenerateRandomString(int length, bool hex = false) {
        static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        static const char hexchars[] = "0123456789ABCDEF";
        std::string result;
        result.reserve(length);

        if (hex) {
            for (int i = 0; i < length; i++) {
                result += hexchars[gen() % 16];
            }
        }
        else {
            for (int i = 0; i < length; i++) {
                result += alphanum[gen() % (sizeof(alphanum) - 1)];
            }
        }
        return result;
    }

    std::string GenerateUUID() {
        std::stringstream ss;
        ss << std::hex << std::uppercase;
        ss << GenerateRandomString(8, true) << "-";
        ss << GenerateRandomString(4, true) << "-";
        ss << "4" << GenerateRandomString(3, true) << "-";
        ss << GenerateRandomString(4, true) << "-";
        ss << GenerateRandomString(12, true);
        return ss.str();
    }

    GPUModel GetRandomGPU() {
        return gpuDatabase[gen() % gpuDatabase.size()];
    }

    bool CreateFullSystemBackup() {
        logger.Log("Creating full system backup...");

        std::string timestamp = std::to_string(
            std::chrono::system_clock::now().time_since_epoch().count());
        std::string backupDir = "backup_" + timestamp;

        CreateDirectoryA(backupDir.c_str(), NULL);

        std::vector<std::string> keysToBackup;
        keysToBackup.push_back("HKLM\\SYSTEM\\CurrentControlSet");
        keysToBackup.push_back("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
        keysToBackup.push_back("HKLM\\HARDWARE\\DESCRIPTION");

        for (size_t i = 0; i < keysToBackup.size(); i++) {
            std::string backupFile = backupDir + "\\" + keysToBackup[i] + ".reg";
            std::replace(backupFile.begin(), backupFile.end(), '\\', '_');
            std::replace(backupFile.begin(), backupFile.end(), ':', '_');

            std::string command = "reg export \"" + keysToBackup[i] + "\" \"" + backupFile + "\" /y";
            if (system(command.c_str()) == 0) {
                backupFiles.push_back(backupFile);
                logger.Log("Backed up: " + keysToBackup[i]);
            }
        }

        std::ofstream restore((backupDir + "\\restore_all.bat").c_str());
        if (restore.is_open()) {
            restore << "@echo off\n";
            restore << "echo Restoring system from backup...\n";
            restore << "echo.\n";

            for (size_t i = 0; i < backupFiles.size(); i++) {
                restore << "reg import \"" << backupFiles[i] << "\"\n";
            }

            restore << "echo.\n";
            restore << "echo Restore completed! Restart your computer.\n";
            restore << "pause\n";
            restore.close();

            logger.Log("Restore script created: " + backupDir + "\\restore_all.bat");
        }

        return true;
    }

    void CleanupTempFiles() {
        logger.Log("Cleaning temporary files...");

        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);

        std::string command = "del /f /s /q \"" + std::string(tempPath) + "\\*.*\" > nul 2>&1";
        system(command.c_str());

        command = "del /f /s /q \"C:\\Windows\\Prefetch\\*.*\" > nul 2>&1";
        system(command.c_str());

        command = "del /f /s /q \"%APPDATA%\\Microsoft\\Windows\\Recent\\*.*\" > nul 2>&1";
        system(command.c_str());

        logger.Log("Temporary files cleaned");
    }

    void ClearEventLogs() {
        logger.Log("Clearing Windows event logs...");

        system("wevtutil cl Application");
        system("wevtutil cl System");
        system("wevtutil cl Security");
        system("wevtutil cl Setup");

        logger.Log("Event logs cleared");
    }

public:
    AdvancedSpoofer() :
        logger(true),
        registry(logger) {
        InitializeGPUDatabase();
        GenerateRandomSeed();
    }

    void SpoofRegistryIDs() {
        logger.Log("Spoofing Registry IDs...");
        registry.SetRegistryValue("Software\\Microsoft\\Cryptography", "MachineGuid", GenerateUUID());
        registry.SetRegistryValue("Software\\Microsoft\\Windows NT\\CurrentVersion", "ProductId", GenerateRandomString(20));
        registry.SetRegistryValue("Control\\IDConfigDB\\Hardware Profiles\\0001", "HwProfileGuid", GenerateUUID());
    }

    void SpoofHWID() {
        logger.Log("Spoofing HWID...");
        registry.SetRegistryValue("Control\\ComputerName\\ActiveComputerName", "ComputerName", "PC-" + GenerateRandomString(8));
        registry.SetRegistryValue("Control\\ComputerName\\ComputerName", "ComputerName", "PC-" + GenerateRandomString(8));

        DWORD installDate = (DWORD)time(NULL) - (gen() % 365 * 86400);
        registry.SetDWORDValue("Control\\Windows", "InstallDate", installDate);
    }

    void SpoofSMBIOS() {
        logger.Log("Spoofing SMBIOS data...");

        std::vector<std::string> manufacturers;
        manufacturers.push_back("ASUS");
        manufacturers.push_back("MSI");
        manufacturers.push_back("Gigabyte");
        manufacturers.push_back("ASRock");
        manufacturers.push_back("Dell");
        manufacturers.push_back("HP");
        manufacturers.push_back("Lenovo");

        std::vector<std::string> models;
        models.push_back("ROG STRIX");
        models.push_back("AORUS");
        models.push_back("MAG");
        models.push_back("Phantom Gaming");
        models.push_back("OptiPlex");
        models.push_back("EliteBook");
        models.push_back("ThinkPad");

        std::string manufacturer = manufacturers[gen() % manufacturers.size()];
        std::string model = models[gen() % models.size()] + " " + GenerateRandomString(4, true);
        std::string serial = GenerateRandomString(12, true);
        std::string uuid = GenerateUUID();

        registry.SetRegistryValue("Hardware\\Description\\System\\BIOS", "SystemManufacturer", manufacturer);
        registry.SetRegistryValue("Hardware\\Description\\System\\BIOS", "SystemProductName", model);
        registry.SetRegistryValue("Hardware\\Description\\System\\BIOS", "SystemSerialNumber", serial);
        registry.SetRegistryValue("Hardware\\Description\\System\\BIOS", "SystemUUID", uuid);

        logger.Log("SMBIOS spoofed:");
        logger.Log("  Manufacturer: " + manufacturer);
        logger.Log("  Product: " + model);
    }

    void SpoofGPU() {
        logger.Log("Spoofing GPU...");

        GPUModel gpu = GetRandomGPU();

        registry.SetRegistryValue("Hardware\\Description\\System\\BIOS", "VideoManufacturer", gpu.vendor);
        registry.SetRegistryValue("Hardware\\Description\\System\\BIOS", "VideoBIOSVersion", GenerateRandomString(10, true));

        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {

            DWORD index = 0;
            char subkey[256];
            DWORD size = 256;

            while (RegEnumKeyExA(hKey, index++, subkey, &size, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                HKEY hDisplay;
                std::string path = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\";
                path += subkey;

                if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_WRITE, &hDisplay) == ERROR_SUCCESS) {
                    std::string driverDesc = gpu.vendor + " " + gpu.model;
                    RegSetValueExA(hDisplay, "DriverDesc", 0, REG_SZ, (BYTE*)driverDesc.c_str(), (DWORD)driverDesc.length() + 1);
                    RegSetValueExA(hDisplay, "HardwareInformation.AdapterString", 0, REG_SZ, (BYTE*)gpu.model.c_str(), (DWORD)gpu.model.length() + 1);

                    RegCloseKey(hDisplay);
                    logger.Log("  Spoofed GPU: " + driverDesc);
                    break;
                }
                size = 256;
            }
            RegCloseKey(hKey);
        }

        logger.Log("GPU spoofed: " + gpu.vendor + " " + gpu.model);
    }

    void SpoofVolumeID() {
        logger.Log("Spoofing Volume ID...");

        DWORD drives = GetLogicalDrives();
        char driveLetter[] = "A:\\";

        for (char c = 'C'; c <= 'Z'; c++) {
            driveLetter[0] = c;
            if (drives & (1 << (c - 'A'))) {
                std::string regPath = "Control\\Session Manager\\DOS Devices";
                registry.SetRegistryValue(regPath, std::string(1, c) + ":", "\\Device\\HarddiskVolume" + GenerateRandomString(2));
                logger.Log("  Volume " + std::string(1, c) + ": spoofed");
            }
        }

        logger.Log("Volume IDs spoofed");
    }

    void SpoofAll() {
        logger.Log("========================================");
        logger.Log("     ADVANCED SPOOFER - FULL SPOOF");
        logger.Log("========================================");

        CreateFullSystemBackup();

        logger.Log("\n[1/6] Spoofing Registry IDs...");
        SpoofRegistryIDs();

        logger.Log("\n[2/6] Spoofing HWID...");
        SpoofHWID();

        logger.Log("\n[3/6] Spoofing SMBIOS...");
        SpoofSMBIOS();

        logger.Log("\n[4/6] Spoofing GPU...");
        SpoofGPU();

        logger.Log("\n[5/6] Spoofing Volume IDs...");
        SpoofVolumeID();

        logger.Log("\n[6/6] Cleaning up...");
        ClearEventLogs();
        CleanupTempFiles();

        logger.Log("\n========================================");
        logger.Log("ALL SPOOFING COMPLETED!");
        logger.Log("========================================");
        logger.Log("Important steps:");
        logger.Log("1. Restart your computer to apply changes");
        logger.Log("2. To restore, run the restore script in the backup folder");
        logger.Log("========================================");
    }

    void ShowStatus() {
        std::cout << "\n========================================" << std::endl;
        std::cout << "     SYSTEM STATUS" << std::endl;
        std::cout << "========================================" << std::endl;

        HKEY hKey;
        char guid[256];
        DWORD size = sizeof(guid);
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Cryptography", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueExA(hKey, "MachineGuid", NULL, NULL, (LPBYTE)guid, &size) == ERROR_SUCCESS) {
                std::cout << "Machine GUID: " << guid << std::endl;
            }
            RegCloseKey(hKey);
        }

        std::cout << "========================================" << std::endl;
    }

    void ShowHelp() {
        std::cout << "\nSpoofer v1.0 - Hardware ID Spoofer" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Usage:" << std::endl;
        std::cout << "  spoofer.exe --all          - Spoof everything (full mode)" << std::endl;
        std::cout << "  spoofer.exe --status       - Show current system status" << std::endl;
        std::cout << "  spoofer.exe --help         - Show this help" << std::endl;
        std::cout << "\nGPU Models Supported:" << std::endl;
        std::cout << "  - NVIDIA: GTX 1050 to RTX 4090" << std::endl;
        std::cout << "  - AMD: RX 6600 to RX 7900 XTX" << std::endl;
        std::cout << "  - GTX 1060 3GB/6GB included" << std::endl;
        std::cout << "========================================" << std::endl;
    }
};

bool IsAdministrator() {
    HANDLE hToken = NULL;


    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        return false;
    }

    TOKEN_ELEVATION elevation;
    DWORD dwSize = sizeof(TOKEN_ELEVATION);

    BOOL isElevated = FALSE;
    if (GetTokenInformation(hToken, TokenElevation, &elevation, dwSize, &dwSize)) {
        isElevated = elevation.TokenIsElevated;
    }

    CloseHandle(hToken);

    if (!isElevated) {
        HANDLE hToken2 = NULL;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken2)) {
            return false;
        }

        DWORD dwSize2 = 0;
        PTOKEN_GROUPS pGroups = NULL;
        bool isAdmin = false;

        if (GetTokenInformation(hToken2, TokenGroups, NULL, 0, &dwSize2) && dwSize2 > 0) {
            pGroups = (PTOKEN_GROUPS)malloc(dwSize2);
            if (GetTokenInformation(hToken2, TokenGroups, pGroups, dwSize2, &dwSize2)) {
                SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
                PSID administratorsGroup = NULL;

                if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &administratorsGroup)) {

                    for (DWORD i = 0; i < pGroups->GroupCount; i++) {
                        if (EqualSid(administratorsGroup, pGroups->Groups[i].Sid)) {
                            isAdmin = true;
                            break;
                        }
                    }
                    FreeSid(administratorsGroup);
                }
            }
            free(pGroups);
        }
        CloseHandle(hToken2);
        return isAdmin;
    }

    return isElevated;
}

int main() {
    SetConsoleTitleA("Spoofer v1.0");

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);


    if (!IsAdministrator()) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
        std::cout << "[ERROR] This program must be run as Administrator!" << std::endl;
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        std::cout << "Please right-click and select 'Run as administrator'" << std::endl;
        system("pause");
        return 1;
    }

    AdvancedSpoofer spoofer;


    std::cout << "\n========================================" << std::endl;
    std::cout << "     SPOOFER v1.0" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. Full System Spoof (All modules)" << std::endl;
    std::cout << "2. Show System Status" << std::endl;
    std::cout << "3. Exit" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Choice: ";

    int choice;
    std::cin >> choice;

    switch (choice) {
    case 1:
        spoofer.SpoofAll();
        break;
    case 2:
        spoofer.ShowStatus();
        break;
    case 3:
        return 0;
    default:
        std::cout << "Invalid choice" << std::endl;
    }

    std::cout << "\nPress any key to exit...";
    system("pause > nul");
    return 0;
}