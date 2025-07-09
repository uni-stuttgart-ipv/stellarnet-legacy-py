#include "pch.h"
#include "StellarnetApi.h"
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <cstdint>

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

void launch_server(LPWSTR exe_path);
std::wstring str_to_wstr(const char* input);

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

const char* stellarnet_path() {
	bSuccess = WriteFile(hClientTx, "stellarnet_path", 15, NULL, NULL);
	if (!bSuccess) {
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

int open() {
	bSuccess = WriteFile(hClientTx, "open", 4, NULL, NULL);
	if (!bSuccess) {
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
	bSuccess = WriteFile(hClientTx, "close", 5, NULL, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}
}

int device_count() {
	bSuccess = WriteFile(hClientTx, "device_count", 12, NULL, NULL);
	if (!bSuccess) {
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
	snprintf(cmd, sizeof(cmd), "set_integration_time %d", rate);
	dwBytes = static_cast<DWORD>(strlen(cmd));
	bSuccess = WriteFile(hClientTx, &cmd, dwBytes, NULL, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}
}

int scan(_int16 channel, float* data) {
	snprintf(cmd, sizeof(cmd), "scan %d", channel);
	dwBytes = static_cast<DWORD>(strlen(cmd));
	bSuccess = WriteFile(hClientTx, &cmd, dwBytes, NULL, NULL);
	if (!bSuccess) {
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
	snprintf(cmd, sizeof(cmd), "scan_c %d", channel);
	dwBytes = static_cast<DWORD>(strlen(cmd));
	bSuccess = WriteFile(hClientTx, &cmd, dwBytes, NULL, NULL);
	if (!bSuccess) {
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
	snprintf(cmd, sizeof(cmd), "read_ee_prom %d %d", channel, address);
	dwBytes = static_cast<DWORD>(strlen(cmd));
	bSuccess = WriteFile(hClientTx, &cmd, dwBytes, NULL, NULL);
	if (!bSuccess) {
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
	snprintf(cmd, sizeof(cmd), "set_update %d %d %d", n_scans, window, temperature_compensation);
	dwBytes = static_cast<DWORD>(strlen(cmd));
	bSuccess = WriteFile(hClientTx, &cmd, dwBytes, NULL, NULL);
	if (!bSuccess) {
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
	snprintf(cmd, sizeof(cmd), "set_x_timing %d", rate);
	dwBytes = static_cast<DWORD>(strlen(cmd));
	bSuccess = WriteFile(hClientTx, &cmd, dwBytes, NULL, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}
}

const char* device_name(_int16 channel) {
	snprintf(cmd, sizeof(cmd), "device_name %d", channel);
	dwBytes = static_cast<DWORD>(strlen(cmd));
	bSuccess = WriteFile(hClientTx, &cmd, dwBytes, NULL, NULL);
	if (!bSuccess) {
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
	snprintf(cmd, sizeof(cmd), "calibration_coefficients %d", channel);
	dwBytes = static_cast<DWORD>(strlen(cmd));
	bSuccess = WriteFile(hClientTx, &cmd, dwBytes, NULL, NULL);
	if (!bSuccess) {
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
	snprintf(
		cmd,
		sizeof(cmd),
		"initialize_spectrometer %d %d %d %d %d",
		integration_time,
		scans_to_avg,
		smoothing_window,
		x_timing,
		temperature_compensation
	);
	dwBytes = static_cast<DWORD>(strlen(cmd));
	bSuccess = WriteFile(hClientTx, &cmd, dwBytes, NULL, NULL);
	if (!bSuccess) {
		ExitProcess(GetLastError());
	}
}

const int* gain_table(_int16 channel) {
	snprintf(cmd, sizeof(cmd), "gain_table %d", channel);
	dwBytes = static_cast<DWORD>(strlen(cmd));
	bSuccess = WriteFile(hClientTx, &cmd, dwBytes, NULL, NULL);
	if (!bSuccess) {
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
	snprintf(cmd, sizeof(cmd), "read_spectrometer_c %d", channel);
	dwBytes = static_cast<DWORD>(strlen(cmd));
	bSuccess = WriteFile(hClientTx, &cmd, dwBytes, NULL, NULL);
	if (!bSuccess) {
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


std::wstring str_to_wstr(const char* input) {
	std::string input_str = std::string(input);
	int wlen = MultiByteToWideChar(CP_UTF8, 0, input_str.c_str(), -1, NULL, 0);
	std::wstring wstr(wlen, 0);
	MultiByteToWideChar(CP_UTF8, 0, input_str.c_str(), -1, &wstr[0], wlen);
	return wstr;
}