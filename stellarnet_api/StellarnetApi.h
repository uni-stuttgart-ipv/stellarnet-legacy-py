#pragma once

#ifdef STELLARNETAPI_EXPORTS
#define STELLARNETAPI_API __declspec(dllexport)
#else
#define STELLARNETAPI_API __declspec(dllimport)
#endif

#include <iostream>

/**
 @brief Launch the StellarNet API server process.
 @param server_exe_path Path to the Stellarnet API server exectuable file.
 */
extern "C" STELLARNETAPI_API
void init(const char* server_exe_path);

/**
 @brief Kill the StellarNet API server process.
 */ 
extern "C" STELLARNETAPI_API
void kill();

/**
 @brief Get the path to the StellarNet program folder.
 @return Path to the StellarNet program directory or NULL if not found.
 @note Does not require `open` to be called first.
 */
extern "C" STELLARNETAPI_API
const char* stellarnet_path();

/**
 @brief Inialize the StellarNet library.
 @note Must be called before any communication with the spectrometer can occur.
 */
extern "C" STELLARNETAPI_API
int open();

/**
 @brief Close the StellarNet library.
 */
extern "C" STELLARNETAPI_API
void close();

extern "C" STELLARNETAPI_API
int device_count();

extern "C" STELLARNETAPI_API
void set_integration_time(int rate);

extern "C" STELLARNETAPI_API
int scan(_int16 channel, float* data);

extern "C" STELLARNETAPI_API
int scan_c(_int16 channel, float* data);

extern "C" STELLARNETAPI_API
void read_ee_prom(_int16 channel, _int16 address, char* data);

extern "C" STELLARNETAPI_API
int set_update(int n_scans, int window, int temperature_compensation);

extern "C" STELLARNETAPI_API
void set_x_timing(int rate);

extern "C" STELLARNETAPI_API
const char* device_name(_int16 channel);

extern "C" STELLARNETAPI_API
int calibration_coefficients(_int16 channel, double* coeffs);

extern "C" STELLARNETAPI_API
void initialize_spectrometer(int integration_time, int scans_to_avg, int smoothing_window, int x_timing, BOOL temperature_compensation);

extern "C" STELLARNETAPI_API
const int* gain_table(_int16 channel);

extern "C" STELLARNETAPI_API
int read_spectrometer_c(_int16 channel, float* data);

//extern "C" STELLARNETAPI_API
//void swdnew(int pFunc, int pval1, int pval2, int pval3);