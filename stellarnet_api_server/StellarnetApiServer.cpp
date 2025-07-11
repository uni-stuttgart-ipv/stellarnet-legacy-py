#include <Windows.h>
#include <KnownFolders.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <iostream>
#include <stdio.h>
#include <cstdint>
#include <string>
#include <sstream>
#include <vector>
#include "StellarnetApiServer.h"

#define BUFSIZE 1024
#define DATA_BUFFER_SIZE 2051
#define GAIN_TABLE_SIZE 1024
#define EE_PROM_DATA_SIZE 32

const wchar_t* STELLARNET_DLL_FOLDER = L"StellarNet\\SpectraWiz";

typedef void(_stdcall* FncPtrVoid)(void);
typedef void(_stdcall* FncPtrInt)(int);
typedef int(_stdcall* FncPtrIntFloatP)(int, float*);
typedef int(_stdcall* FncPtrIntFloatPLV)(int, float*);
typedef int(_stdcall* FncPtrIntIntInt)(int, int, int);
typedef int(_stdcall* FncPtrIntVoid)(void);
typedef BOOL(_stdcall* FncPtrWordWordCharP)(_int16, _int16, char*);
typedef int(_stdcall* FncPtrIntIntIntInt)(int, int, int, int);

FncPtrVoid SWDinit;
FncPtrInt SWDrate;
FncPtrIntFloatP SWDscan;
FncPtrIntFloatPLV SWDscanLV;
FncPtrVoid SWDclose;
FncPtrIntIntInt SWDupdate;
FncPtrInt SWDxtrate;
FncPtrIntVoid SWDdevcnt;
FncPtrWordWordCharP SWDeeRead;
FncPtrIntIntIntInt SWDnew;

static BOOL g_LibraryOpen;
HINSTANCE hDll;

void parse_command(const char* input, char* command, std::vector<std::string>& args);
bool stellarnet_path(LPWSTR path);
void parse_gain_table(string table_str, int* gain_table);
std::string wchar_to_string(const wchar_t* wpath);
std::wstring string_to_wchar(const std::string& input);
void rtrim(char* buffer);
int string_to_int(string value);
bool dir_exists(LPWSTR path);

int main() {
	CHAR cmd_buffer[BUFSIZE];
	CHAR cmd[BUFSIZE];
	std::vector<std::string> args;
	DWORD dwBytes;
	HANDLE hRx, hTx;
	BOOL bSuccess;
	float data[DATA_BUFFER_SIZE] = { 0.0f };

	hTx = GetStdHandle(STD_OUTPUT_HANDLE);
	hRx = GetStdHandle(STD_INPUT_HANDLE);
	if (hTx == INVALID_HANDLE_VALUE || hRx == INVALID_HANDLE_VALUE) {
		std::cerr << "Error getting standard handles." << std::endl;
		ExitProcess(1);
	}

	for (;;) {
		bSuccess = ReadFile(hRx, cmd_buffer, BUFSIZE - 1, &dwBytes, nullptr);
		if (!bSuccess || dwBytes == 0) {
			break;
		}
		cmd_buffer[dwBytes] = '\0';
		rtrim(cmd_buffer);
		if (strlen(cmd_buffer) == 0) {
			continue;
		}

		parse_command(cmd_buffer, cmd, args);
		if (strcmp(cmd, "kill") == 0) {
			break;
		}
		else if (strcmp(cmd, "stellarnet_path") == 0) {
			LPWSTR path = new WCHAR[MAX_PATH];
			bool success = stellarnet_path(path);
			bSuccess = WriteFile(hTx, &success, sizeof(success), &dwBytes, nullptr);
			if (!bSuccess || dwBytes != sizeof(success)) {
				ExitProcess(GetLastError());
			}

			if (!success) {
				continue;
			}

			std::string path_str = wchar_to_string(path);
			bSuccess = WriteFile(hTx, path_str.data(), path_str.size(), &dwBytes, nullptr);
			if (!bSuccess || dwBytes != path_str.size()) {
				ExitProcess(GetLastError());
			}
		}
		else if (strcmp(cmd, "open") == 0) {
			int res = open();
			bSuccess = WriteFile(hTx, &res, sizeof(res), &dwBytes, nullptr);
			if (!bSuccess || dwBytes != sizeof(res)) {
				ExitProcess(GetLastError());
			}
		}
		else if (strcmp(cmd, "close") == 0) {
			close();
		}
		else if (strcmp(cmd, "device_count") == 0) {
			int count = device_count();
			bSuccess = WriteFile(hTx, &count, sizeof(count), &dwBytes, nullptr);
			if (!bSuccess || dwBytes != sizeof(count)) {
				ExitProcess(GetLastError());
			}
		}
		else if (strcmp(cmd, "set_integration_time") == 0) {
			int rate = std::stoi(args[0].c_str(), nullptr);
			set_integration_time(rate);
		}
		else if (strcmp(cmd, "scan") == 0) {
			int channel = std::stoi(args[0].c_str(), nullptr);
			int status = scan(channel, data);

			bSuccess = WriteFile(hTx, &status, sizeof(status), &dwBytes, nullptr);
			if (!bSuccess || dwBytes != sizeof(status)) {
				ExitProcess(GetLastError());
			}

			if (status == 0) {
				bSuccess = WriteFile(hTx, data, sizeof(data), &dwBytes, nullptr);
				if (!bSuccess || dwBytes != sizeof(data)) {
					ExitProcess(GetLastError());
				}
			}
		}
		else if (strcmp(cmd, "scan_c") == 0) {
			int channel = std::stoi(args[0].c_str(), nullptr);
			int status = scan_c(channel, data);

			bSuccess = WriteFile(hTx, &status, sizeof(status), &dwBytes, nullptr);
			if (!bSuccess || dwBytes != sizeof(status)) {
				ExitProcess(GetLastError());
			}

			if (status == 0) {
				bSuccess = WriteFile(hTx, data, sizeof(data), &dwBytes, nullptr);
				if (!bSuccess || dwBytes != sizeof(data)) {
					ExitProcess(GetLastError());
				}
			}
		}
		else if (strcmp(cmd, "read_ee_prom") == 0) {
			int channel = std::stoi(args[0].c_str(), nullptr);
			int address = std::stoi(args[1].c_str(), nullptr);
			char data[EE_PROM_DATA_SIZE];

			read_ee_prom(channel, address, data);
			bSuccess = WriteFile(hTx, &data, sizeof(data), &dwBytes, nullptr);
			if (!bSuccess || dwBytes != sizeof(data)) {
				ExitProcess(GetLastError());
			}
		}
		else if (strcmp(cmd, "set_update") == 0) {
			int n_scans = std::stoi(args[0].c_str(), nullptr);
			int window = std::stoi(args[1].c_str(), nullptr);
			int temperature_compensation = std::stoi(args[2].c_str(), nullptr);

			int res = set_update(n_scans, window, temperature_compensation);
			bSuccess = WriteFile(hTx, &res, sizeof(res), &dwBytes, nullptr);
			if (!bSuccess || dwBytes != sizeof(res)) {
				ExitProcess(GetLastError());
			}
		}
		else if (strcmp(cmd, "set_x_timing") == 0) {
			int rate = std::stoi(args[0].c_str(), nullptr);
			set_x_timing(rate);
		}
		else if (strcmp(cmd, "device_name") == 0) {
			int channel = std::stoi(args[0].c_str(), nullptr);
			std::string name = device_name(channel);
			if (name.empty()) {
				char empty[BUFSIZE] = { 0 };
				bSuccess = WriteFile(hTx, empty, sizeof(empty), &dwBytes, nullptr);
				if (!bSuccess || dwBytes != sizeof(empty)) {
					ExitProcess(GetLastError());
				}
			}
			else {
				char buffer[BUFSIZE] = { 0 };
				strncpy_s(buffer, name.c_str(), sizeof(buffer) - 1);
				bSuccess = WriteFile(hTx, buffer, sizeof(buffer), &dwBytes, nullptr);
				if (!bSuccess || dwBytes != sizeof(buffer)) {
					ExitProcess(GetLastError());
				}
			}
		}
		else if (strcmp(cmd, "calibration_coefficients") == 0) {
			_int16 channel = std::stoi(args[0].c_str(), nullptr);
			double c1, c2, c3, c4;
			int res = calibration_coefficients(channel, &c1, &c2, &c3, &c4);
			bSuccess = WriteFile(hTx, &res, sizeof(res), &dwBytes, nullptr);
			if (!bSuccess || dwBytes != sizeof(res)) {
				ExitProcess(GetLastError());
			}
			if (res == 0) {
				double coeffs[] = { c1, c2, c3, c4 };
				bSuccess = WriteFile(hTx, coeffs, sizeof(double) * 4, &dwBytes, nullptr);
				if (!bSuccess || dwBytes != sizeof(double) * 4) {
					ExitProcess(GetLastError());
				}
			}
		}
		else if (strcmp(cmd, "initialize_spectrometer") == 0) {
			int integration_time = std::stoi(args[0].c_str(), nullptr);
			int scans_to_avg = std::stoi(args[1].c_str(), nullptr);
			int smoothing_window = std::stoi(args[2].c_str(), nullptr);
			int x_timing = std::stoi(args[3].c_str(), nullptr);
			BOOL temperature_compensation = (args[4] == "1") ? TRUE : FALSE;
			initialize_spectrometer(integration_time, scans_to_avg, smoothing_window, x_timing, temperature_compensation);
		}
		else if (strcmp(cmd, "gain_table") == 0) {
			char table_str_raw[BUFSIZE] = { 0 };
			int gain_table[10] = { 0 };
			_int16 channel = std::stoi(args[0].c_str(), nullptr);
			get_gain_table(channel, table_str_raw);
			table_str_raw[BUFSIZE - 1] = '\0';
			string table_str(table_str_raw);
			parse_gain_table(table_str, gain_table);

			bSuccess = WriteFile(hTx, gain_table, sizeof(gain_table), &dwBytes, nullptr);
			if (!bSuccess || dwBytes != sizeof(gain_table)) {
				ExitProcess(GetLastError());
			}
		}
		else if (strcmp(cmd, "read_spectrometer_c") == 0) {
			int channel = std::stoi(args[0].c_str(), nullptr);
			int status = read_spectrometer_c(channel, data);

			bSuccess = WriteFile(hTx, &status, sizeof(status), &dwBytes, nullptr);
			if (!bSuccess || dwBytes != sizeof(status)) {
				ExitProcess(GetLastError());
			}

			if (status == 0) {
				bSuccess = WriteFile(hTx, data, sizeof(data), &dwBytes, nullptr);
				if (!bSuccess || dwBytes != sizeof(data)) {
					ExitProcess(GetLastError());
				}
			}
			}
		else {
			std::cerr << "Unknown command: " << cmd_buffer << std::endl;
			ExitProcess(ERROR_PROC_NOT_FOUND);
		}
	}
	return 0;
}

/**
 * @brief Parses a space-delimited string into command and arguments.
 * @param input The input buffer (null-terminated string).
 * @param command Output buffer for the command (first token).
 * @param args Output vector of argument strings.
 */
void parse_command(const char* input, char* command, std::vector<std::string>& args) {
	std::istringstream iss(input);
	std::string cmd;
	if (!(iss >> cmd)) {
		command[0] = '\0';
		return;
	}
	strncpy_s(command, BUFSIZE, cmd.c_str(), _TRUNCATE);
	command[BUFSIZE - 1] = '\0';

	std::string arg;
	while (iss >> arg) {
		args.push_back(arg);
	}
}

bool stellarnet_path(LPWSTR path) {
	WCHAR path_t[MAX_PATH];
	PWSTR program_files;
	HRESULT res;
	size_t env_len;

	res = SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, nullptr, &program_files);
	if (!SUCCEEDED(res)) {
		std::wcerr << L"Failed to get Program Files path: " << res << std::endl;
		ExitProcess(GetLastError());
	}

	if (!PathCombine(path_t, program_files, STELLARNET_DLL_FOLDER)) {
		std::wcerr << L"Failed to combine paths." << std::endl;
		CoTaskMemFree(program_files);
		ExitProcess(GetLastError());
	};

	CoTaskMemFree(program_files);
	if (dir_exists(path_t)) {
		wcscpy_s(path, MAX_PATH, path_t);
		return true;
	}

	WCHAR* path_env;
	errno_t err = _wdupenv_s(&path_env, &env_len, L"ProgramW6432");
	if (err) {
		return false;
	}

	if (!PathCombine(path_t, path_env, STELLARNET_DLL_FOLDER)) {
		std::wcerr << L"Failed to combine paths." << std::endl;
		ExitProcess(GetLastError());
	};
	
	if (dir_exists(path_t)) {
		wcscpy_s(path, MAX_PATH, path_t);
		return true;
	}

	return false;
}

int open() {
	if (g_LibraryOpen) {
		return TRUE;
	}

	LPWSTR dll_path = new WCHAR[MAX_PATH];
	stellarnet_path(dll_path);
	if (!SetDllDirectory(dll_path)) {
		std::wcerr << L"Failed to set DLL directory." << std::endl;
		delete[] dll_path;
		return -21;
	}

	if (!PathAppend(dll_path, L"swdll64.dll")) {
		std::wcerr << L"Failed to append DLL name." << std::endl;
		delete[] dll_path;
		return -22;
	}

	hDll = LoadLibrary(dll_path);
	delete[] dll_path;
	if (hDll == NULL) {
		std::wcerr << L"Failed to load DLL." << std::endl;
		return -23;
	}

	SWDinit = (FncPtrVoid)GetProcAddress(hDll, "SWDinit");
	SWDrate = (FncPtrInt)GetProcAddress(hDll, "SWDrate");
	SWDscan = (FncPtrIntFloatP)GetProcAddress(hDll, "SWDscan");
	SWDscanLV = (FncPtrIntFloatPLV)GetProcAddress(hDll, "SWDscanLV");
	SWDclose = (FncPtrVoid)GetProcAddress(hDll, "SWDclose");
	SWDupdate = (FncPtrIntIntInt)GetProcAddress(hDll, "SWDupdate");
	SWDxtrate = (FncPtrInt)GetProcAddress(hDll, "SWDxtrate");
	SWDdevcnt = (FncPtrIntVoid)GetProcAddress(hDll, "SWDdevcnt");
	SWDeeRead = (FncPtrWordWordCharP)GetProcAddress(hDll, "SWDeeRead");
	SWDnew = (FncPtrIntIntIntInt)GetProcAddress(hDll, "SWDnew");

	if (!SWDinit) {
		FreeLibrary(hDll);
		return -10;
	}
	if (!SWDrate) {
		FreeLibrary(hDll);
		return -11;
	}
	if (!SWDscan) {
		FreeLibrary(hDll);
		return -12;
	}
	if (!SWDscanLV) {
		FreeLibrary(hDll);
		return -13;
	}
	if (!SWDupdate) {
		FreeLibrary(hDll);
		return -14;
	}
	if (!SWDxtrate) {
		FreeLibrary(hDll);
		return -15;
	}
	if (!SWDclose) {
		FreeLibrary(hDll);
		return -16;
	}
	if (!SWDnew) {
		FreeLibrary(hDll);
		return -19;
	}

	SWDinit();	// SAFTEY: `SWDinit` should only be called once.
	g_LibraryOpen = TRUE;

	if (!SWDdevcnt) {
		return(-17);
	}
	if (!SWDeeRead) {
		return(-18);
	}

	return TRUE;
}

void close() {
	if (g_LibraryOpen != TRUE) {
		return;
	}

	SWDclose();
	FreeLibrary(hDll);
}


int device_count() {
	if (!g_LibraryOpen) {
		std::cerr << "Stellarnet library not open. You must call `open` before any other commands." << std::endl;
		return -20;
	}

	return SWDdevcnt();
}


void set_integration_time(int rate) {
	if (!g_LibraryOpen) {
		std::cerr << "Stellarnet library not open. You must call `open` before any other commands." << std::endl;
	}

	SWDrate(rate);
}

int scan(int channel, float* buffer) {
	if (!g_LibraryOpen) {
		std::cerr << "Stellarnet library not open. You must call `open` before any other commands." << std::endl;
		return -20;
	}

	return SWDscan(channel, buffer);
}

int scan_c(int channel, float* buffer) {
	if (!g_LibraryOpen) {
		std::cerr << "Stellarnet library not open. You must call `open` before any other commands." << std::endl;
		return -20;
	}

	return SWDscanLV(channel, buffer);
}

int set_update(int n_scans, int window, int temperature_compensation) {
	if (!g_LibraryOpen) {
		std::cerr << "Stellarnet library not open. You must call `open` before any other commands." << std::endl;
		return -20;
	}

	return SWDupdate(n_scans, window, temperature_compensation);
}

void set_x_timing(int rate) {
	if (!g_LibraryOpen) {
		std::cerr << "Stellarnet library not open. You must call `open` before any other commands." << std::endl;
	}

	SWDxtrate(rate);
}

void read_ee_prom(_int16 address, _int16 length, char* data) {
	if (!g_LibraryOpen) {
		std::cerr << "Stellarnet library not open. You must call `open` before any other commands." << std::endl;
	}

	SWDeeRead(address, length, data);
}

int new_device(int pFunc, int pval1, int pval2, int pval3) {
	return -1;
}

std::string device_name(_int16 channel) {
	static const _int16 NAME_ADDR = 0x20;
	char buffer[BUFSIZE] = {0};
	std::string empty = "";

	if (!SWDeeRead(channel, NAME_ADDR, buffer)) {
		return empty;
	}
	buffer[BUFSIZE - 1] = 0;
	std::string name(buffer);
	return name;
}


/**
 * @brief Get callibration coefficients c1, c2, c3, and c4 stored in spectrometer.
 * @param channel Channel number (0 - 8).
 * @param c1 Pointer to store coefficient c1.
 * @param c2 Pointer to store coefficient c2.
 * @param c3 Pointer to store coefficient c3.
 * @param c4 Pointer to store coefficient c4.
 * @return 0 on success, 1-4 on read error, 10 on invalid data.
 */
int calibration_coefficients(_int16 channel, double* c1, double* c2, double* c3, double* c4) {
	static const _int16 C1_ADDR = 0x80;
	static const _int16 C2_ADDR = 0xA0;
	static const _int16 C3_ADDR = 0xC0;
	static const _int16 C4_ADDR = 0xE0;

	char buffer[32];
	if (!SWDeeRead(channel, C1_ADDR, buffer)) {
		return 1;
	}

	for (int i = 0; i <= 16; i++) {
		// check if the character is printable ASCII
		if (buffer[i] < 0x20 || buffer[i] > 0x7F) {
			return 10;
		}
	}

	buffer[14] = 0;
	*c1 = atof(buffer);

	if (!SWDeeRead(channel, C2_ADDR, buffer)) {
		return 2;
	}
	buffer[14] = 0;
	*c2 = atof(buffer);

	if (!SWDeeRead(channel, C3_ADDR, buffer)) {
		return 3;
	}
	buffer[14] = 0;
	*c3 = atof(buffer);

	if (!SWDeeRead(channel, C4_ADDR, buffer)) {
		return 4;
	}
	buffer[14] = 0;
	*c4 = atof(buffer);

	return 0;
}

void initialize_spectrometer(int integration_time, int scans_to_avg, int smoothing_window, int x_timing, BOOL temperature_compensation) {
	SWDrate(integration_time);
	SWDupdate(scans_to_avg, smoothing_window, temperature_compensation);
	SWDxtrate(x_timing);
}

void get_gain_table(_int16 channel, char* table) {
	static const _int16 GAIN_TABLE_ADDR = 0xFF;
	SWDeeRead(channel, GAIN_TABLE_ADDR, table);
}

/**
 * @brief Parses a gain table string into an array of integers.
 * @param table_str String representation of the gain table.
 * @param gain_table Gain table data.
 * @note `gain_table` should be an array of size 10.
 */
void parse_gain_table(string table_str, int* gain_table) {
	gain_table[0] = string_to_int(table_str.substr(1, 3));
	gain_table[1] = string_to_int(table_str.substr(4, 3));
	gain_table[2] = string_to_int(table_str.substr(7, 3));
	gain_table[3] = string_to_int(table_str.substr(10, 3));
	gain_table[4] = string_to_int(table_str.substr(13, 3));
	gain_table[5] = string_to_int(table_str.substr(17, 3));
	gain_table[6] = string_to_int(table_str.substr(19, 3));
	gain_table[7] = string_to_int(table_str.substr(22, 3));
	gain_table[8] = string_to_int(table_str.substr(25, 3));
	gain_table[9] = string_to_int(table_str.substr(28, 3));
}


int read_spectrometer_c(_int16 channel, float* data) {
	int attempts = 0;
	int status = -1;
	do {
		if (status != 0) {
			Sleep(1);
		}

		status = SWDscanLV(channel, data); 
		attempts++;
		if (attempts > 3000) {
			break;
		}
	} while (status != 0);

	return status;
}

/**
 * @brief Removes trailing newline and carriage return characters from the command buffer.
 * @param buffer The character buffer to trim.
 * @note This function modifies the command buffer in place.
 */
void rtrim(char* buffer) {
	size_t len = strlen(buffer);
	while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
		buffer[--len] = '\0';
	}
}


std::string wchar_to_string(const wchar_t* wpath) {
	if (!wpath) return {};
	int size = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, nullptr, 0, nullptr, nullptr);
	if (size <= 0) return {};
	std::string result(size - 1, 0); // Exclude null terminator
	WideCharToMultiByte(CP_UTF8, 0, wpath, -1, &result[0], size, nullptr, nullptr);
	return result;
}

std::wstring string_to_wchar(const std::string& input) {
	if (input.empty()) return {};
	int size = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), input.length(), nullptr, 0);
	if (size <= 0) return {};
	std::wstring result(size, 0);
	MultiByteToWideChar(CP_UTF8, 0, input.c_str(), input.length(), &result[0], size);
	return result;
}


int string_to_int(string value) {
	string temp;
	for (size_t i = 0; i < value.length(); i++) {
		if (!isspace(value[i])) {
			temp += value[i];
		}
	}
	try {
		return stoi(temp);
	}
	catch (...) {
		return -1;
	}
}

bool dir_exists(LPWSTR path) {
	DWORD attrs = GetFileAttributesW(path);
	if (attrs == INVALID_FILE_ATTRIBUTES)
		return false;

	if (attrs & FILE_ATTRIBUTE_DIRECTORY)
		return true; 

	return false;
}