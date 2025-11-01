#include "pch.h"
#include "StellarnetApi.h"
#include <tchar.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <time.h>
#include <cstdint>
#include <cstdarg>
#include <chrono>

#define BUFSIZE 1024
#define DATA_BUFFER_SIZE 2051
#define GAIN_TABLE_SIZE 10
#define EE_PROM_DATA_SIZE 32

HANDLE hClientTx;
HANDLE hClientRx;
HANDLE hServerTx;
HANDLE hServerRx;
BOOL bSuccess;
DWORD dwBytes;
CHAR cmd[BUFSIZE];

std::string time_now_str();
BOOL g_loggingEnabled = false;
LPWSTR g_logPath;

static bool send_command(const char* fmt, ...);
void launch_server(LPWSTR exe_path);
std::wstring str_to_wstr(const char* input);
static std::string narrow_from_wide(const WCHAR* wpath);
void log(std::string& message);

static bool send_command(const char* fmt, ...) {
	char buf[BUFSIZE];
	va_list ap;
	va_start(ap, fmt);
	int n = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if (n < 0) return false;

	size_t len = static_cast<size_t>(n);
	if (len >= sizeof(buf) - 1) len = sizeof(buf) - 2;
	buf[len++] = '\n';
	
	DWORD written = 0;
	BOOL ok = WriteFile(hClientTx, buf, static_cast<DWORD>(len), &written, NULL);
	if (!ok || written != len) {
		return false;
	}

	return true;
}

void init(const char* server_exe_path) {
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	if (!CreatePipe(&hClientRx, &hServerTx, &saAttr, 0)) {
		ExitProcess(GetLastError());
	}
	if (!SetHandleInformation(hClientRx, HANDLE_FLAG_INHERIT, 0)) {
		ExitProcess(GetLastError());
	}
	if (!CreatePipe(&hServerRx, &hClientTx, &saAttr, 0)) {
		ExitProcess(GetLastError());
	}
	if (!SetHandleInformation(hClientTx, HANDLE_FLAG_INHERIT, 0)) {
		ExitProcess(GetLastError());
	}

	std::wstring exe_path = str_to_wstr(server_exe_path);
	launch_server(&exe_path[0]);
}

void launch_server(LPWSTR exe_path) {
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = hServerTx;
	siStartInfo.hStdOutput = hServerTx;
	siStartInfo.hStdInput = hServerRx;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	bSuccess = CreateProcess(
		NULL,
		exe_path,
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&siStartInfo,
		&piProcInfo
	);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}

	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);
	CloseHandle(hServerTx);
	CloseHandle(hServerRx);
}

void kill() {
	bSuccess = WriteFile(hClientTx, "kill", 4, NULL, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}

	if (hClientTx) {
		CloseHandle(hClientTx);
		hClientTx = NULL;
	}
	if (hClientRx) {
		CloseHandle(hClientRx);
		hClientRx = NULL;
	}
}

void enable_logging(const char* path) {
	if (g_logPath != nullptr) {
		delete[] g_logPath;
		g_logPath = nullptr;
	}

	g_logPath = new WCHAR[MAX_PATH];
	int conv = MultiByteToWideChar(CP_UTF8, 0, path, -1, g_logPath, MAX_PATH);
	if (conv == 0) {
		delete[] g_logPath;
		g_logPath = nullptr;
		return;
	}
	g_loggingEnabled = true;

	if (!send_command("enable_logging %s", path)) {
		ExitProcess(GetLastError());
	}
}

void disable_logging() {

	if (!send_command("disable_logging")) {
		ExitProcess(GetLastError());
	}

	if (g_logPath != nullptr) {
		delete[] g_logPath;
		g_logPath = nullptr;
	}
	g_loggingEnabled = false;
}

const char* stellarnet_path() {
	std::string log_msg;
	log_msg = std::string("[cmd] stellarnet_path");
	log(log_msg);

	if (!send_command("stellarnet_path")) {
		ExitProcess(GetLastError());
	}
	
	bool success;
	bSuccess = ReadFile(hClientRx, &success, sizeof(success), &dwBytes, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}

	if (!success) {
		return NULL;
	}

	CHAR resp[BUFSIZE];
	bSuccess = ReadFile(hClientRx, resp, BUFSIZE - 1, &dwBytes, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}
	resp[dwBytes] = '\0';

	char* out = (char*)malloc(strlen(resp) + 1);
	if (out == NULL) {
		ExitProcess(GetLastError());
	}
	strcpy_s(out, strlen(resp) + 1, resp);

	log_msg = std::string("[resp] stellarnet_path");
	log(log_msg);

	return out;
}

int open() {
	
	if (!send_command("open")) {
		ExitProcess(GetLastError());
	}

	int resp;
	bSuccess = ReadFile(hClientRx, &resp, sizeof(resp), &dwBytes, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}
	return resp;
}

void close() {
	
	if (!send_command("close")) {
		ExitProcess(GetLastError());
	}
}

int device_count() {
	
	if (!send_command("device_count")) {
		ExitProcess(GetLastError());
	}

	int32_t count;
	bSuccess = ReadFile(hClientRx, &count, sizeof(count), &dwBytes, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}
	if (dwBytes != sizeof(count)) {
		ExitProcess(ERROR_READ_FAULT);
	}

	return static_cast<int>(count);
}

void set_integration_time(int rate) {
	
	if (!send_command("set_integration_time %d", rate)) {
		ExitProcess(GetLastError());
	}
}

int scan(_int16 channel, float* data) {
	
	if (!send_command("scan %d", channel)) {
		ExitProcess(GetLastError());
	}

	int32_t status;
	bSuccess = ReadFile(hClientRx, &status, sizeof(status), &dwBytes, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}
	if (dwBytes != sizeof(status)) {
		ExitProcess(ERROR_READ_FAULT);
	}

	if (status == 0) {
		bSuccess = ReadFile(hClientRx, data, sizeof(float) * DATA_BUFFER_SIZE, &dwBytes, NULL);
		if (!bSuccess) {
			ExitProcess(GetLastError());
		}
		if (dwBytes != sizeof(float) * DATA_BUFFER_SIZE) {
			ExitProcess(ERROR_READ_FAULT);
		}
	}

	return static_cast<int>(status);
}


int scan_c(_int16 channel, float* data) {
	
	if (!send_command("scan_c %d", channel)) {
		ExitProcess(GetLastError());
	}

	int32_t status;
	bSuccess = ReadFile(hClientRx, &status, sizeof(status), &dwBytes, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}
	if (dwBytes != sizeof(status)) {
		ExitProcess(ERROR_READ_FAULT);
	}

	if (status == 0) {
		bSuccess = ReadFile(hClientRx, data, sizeof(float) * DATA_BUFFER_SIZE, &dwBytes, NULL);
		if (!bSuccess) {
			ExitProcess(GetLastError());
		}
		if (dwBytes != sizeof(float) * DATA_BUFFER_SIZE) {
			ExitProcess(ERROR_READ_FAULT);
		}
	}

	return static_cast<int>(status);
}

void read_ee_prom(_int16 channel, _int16 address, char* data) {
	
	if (!send_command("read_ee_prom %d %d", channel, address)) {
		ExitProcess(GetLastError());
	}

	bSuccess = ReadFile(hClientRx, data, sizeof(data), &dwBytes, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}
	if (dwBytes != sizeof(data)) {
		ExitProcess(ERROR_READ_FAULT);
	}
}

int set_update(int n_scans, int window, int temperature_compensation) {
	
	if (!send_command("set_update %d %d %d", n_scans, window, temperature_compensation)) {
		ExitProcess(GetLastError());
	}
	int32_t resp;
	bSuccess = ReadFile(hClientRx, &resp, sizeof(resp), &dwBytes, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}
	if (dwBytes != sizeof(resp)) {
		ExitProcess(ERROR_READ_FAULT);
	}
	return static_cast<int>(resp);
}

void set_x_timing(int rate) {
	
	if (!send_command("set_x_timing %d", rate)) {
		ExitProcess(GetLastError());
	}
}

const char* device_name(_int16 channel) {
	
	if (!send_command("device_name %d", channel)) {
		ExitProcess(GetLastError());
	}

	CHAR resp[BUFSIZE];
	bSuccess = ReadFile(hClientRx, resp, BUFSIZE - 1, &dwBytes, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}
	resp[dwBytes] = '\0';

	char* out = (char*)malloc(strlen(resp) + 1);
	if (out == NULL) {
		ExitProcess(GetLastError());
	}
	strcpy_s(out, strlen(resp) + 1, resp);
	return out;
}

int calibration_coefficients(_int16 channel, double* coeffs) {
	
	if (!send_command("calibration_coefficients %d", channel)) {
		ExitProcess(GetLastError());
	}

	int32_t resp;
	bSuccess = ReadFile(hClientRx, &resp, sizeof(resp), &dwBytes, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}

	if (resp != 0) {
		return static_cast<int>(resp);
	}

	bSuccess = ReadFile(hClientRx, coeffs, sizeof(double) * 4, &dwBytes, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}

	return 0;
}

void initialize_spectrometer(int integration_time, int scans_to_avg, int smoothing_window, int x_timing, BOOL temperature_compensation) {
	
	if (!send_command(
		"initialize_spectrometer %d %d %d %d %d",
		integration_time,
		scans_to_avg,
		smoothing_window,
		x_timing,
		temperature_compensation
	)) {
		ExitProcess(GetLastError());
	}
}

const int* gain_table(_int16 channel) {
	
	if (!send_command("gain_table %d", channel)) {
		ExitProcess(GetLastError());
	}

	int32_t table32[GAIN_TABLE_SIZE];
	bSuccess = ReadFile(hClientRx, table32, sizeof(table32), &dwBytes, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}
	if (dwBytes != sizeof(table32)) {
		ExitProcess(ERROR_READ_FAULT);
	}

	int* table = new int[GAIN_TABLE_SIZE];
	for (int i = 0; i < GAIN_TABLE_SIZE; ++i) {
		table[i] = static_cast<int>(table32[i]);
	}

	return table;
}

int read_spectrometer_c(_int16 channel, float* data) {
	std::string log_msg;
	log_msg = std::string("[cmd] read_spectrometer_c");
	log(log_msg);

	if (!send_command("read_spectrometer_c %d", channel)) {
		ExitProcess(GetLastError());
	}

	int32_t status;
	bSuccess = ReadFile(hClientRx, &status, sizeof(status), &dwBytes, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}
	if (dwBytes != sizeof(status)) {
		ExitProcess(ERROR_READ_FAULT);
	}

	if (status == 0) {
		bSuccess = ReadFile(hClientRx, data, sizeof(float) * DATA_BUFFER_SIZE, &dwBytes, NULL);
		if (!bSuccess) {
			ExitProcess(GetLastError());
		}
		if (dwBytes != sizeof(float) * DATA_BUFFER_SIZE) {
			ExitProcess(ERROR_READ_FAULT);
		}
	}

	log_msg = std::string("[resp] read_spectrometer_c");
	log(log_msg);
	return static_cast<int>(status);
}


std::wstring str_to_wstr(const char* input) {
	std::string input_str = std::string(input);
	int wlen = MultiByteToWideChar(CP_UTF8, 0, input_str.c_str(), -1, NULL, 0);
	std::wstring wstr(wlen, 0);
	MultiByteToWideChar(CP_UTF8, 0, input_str.c_str(), -1, &wstr[0], wlen);
	return wstr;
}

static std::string narrow_from_wide(const WCHAR* wpath) {
	if (!wpath) return std::string();
	int size = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, nullptr, 0, nullptr, nullptr);
	if (size <= 0) return std::string();
	std::string out(size - 1, '\0');
	WideCharToMultiByte(CP_UTF8, 0, wpath, -1, &out[0], size, nullptr, nullptr);
	return out;
}

void log(std::string& message) {
	std::ofstream log_file;

	if (g_loggingEnabled && g_logPath != nullptr) {
		std::string path = narrow_from_wide(g_logPath);
		log_file.open(path, std::ios_base::app);
		if (log_file.is_open()) {
			log_file << "{\"source\": \"api\", \"timestamp\": \"" << time_now_str() << "\", \"msg\": \"" << message << "\"}" << std::endl;
			log_file.close();
		}
		
	}
}

std::string time_now_str() {
	struct tm local_time;
	errno_t err;
	auto now = std::chrono::system_clock::now();
	std::time_t current_time = std::chrono::system_clock::to_time_t(now);
	err = localtime_s(&local_time, &current_time);
	char buffer[40];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local_time);
	return buffer;
}