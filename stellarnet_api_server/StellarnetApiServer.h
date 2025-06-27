#pragma once
#include <Windows.h>
#include <iostream>
using std::string;

/**
 * @brief Gets the full path to the StellarNet program folder.
 * @param path Output buffer to receive the path.
 */
void stellarnet_path(LPWSTR path);

/**
 * @brief Opens and initializes the StellarNet library.
 * @return TRUE on success, negative value on error.
 */
int open();

/**
 * @brief Closes the StellarNet library and releases resources.
 */
void close();


/**
 * @brief Returns the number of connected devices.
 * @return Device count, or negative value on error.
 * @note Corresponds to the `SWDdevcnt` function in the StellarNet API.
 */
int device_count();

/**
 * @brief Set the integration time is milliseconds.
 * @note Valid values between 4 and 65500.
 * @note Minimum integration time for `XTRate` {0 is 1, 1 is 15, 2 is 30}. See `set_x_timing`.
 * @note Corresponds to the `SWDrate` function in the StellarNet API.
 */
void set_integration_time(int rate);

/**
 * @brief Acquire a scan.
 * @param channel Channel number (0 - 8).
 * @param data Pointer to a float array to receive the scan data. Must be 2051 elements long.
 * @returns 0 on success, 1 on busy, 2 on timeout, 3 on error.
 * @note Corresponds to the `SWDscan` function in the StellarNet API.
 */
int scan(int channel, float* data);

/**
 * @brief Acquire a scan for LabView and C++ programs.
 * @param channel Channel number (0 - 8).
 * @param data Pointer to a float array to receive the scan data. Must be 2051 elements long.
 * @returns 0 on success, 1 on busy, 2 on timeout, 3 on error.
 * @note Corresponds to the `SWDscanLV` function in the StellarNet API.
 */
int scan_c(int channel, float* data);

/**
 * @brief Update data processing dynamically.
 * @param n_scans Number of scans to average. Must be in [1, 99].
 * @param window Window for smoothing data. 0 = none, 1 = 5 pixels, 2 = 9 pixels, 3 = 17 pixels, 4 = 33 pixels.
 * @param temperature_compensation Enable temperature compensation (0 = off, 1 = on).
 * @note Smoothing uses boxcar averaging over the window size.
 * @note Temperature compensation uses detector optical black region pixels 1 - 12.
 * @note Corresponds to the `SWDupdate` function in the StellarNet API.
 */
int set_update(int n_scans, int window, int temperature_compensation);

/**
 * @brief Set the x timing resolution clock.
 * @param rate Timing rate. 0 = fastest with lowest resolution, 1 = medium, 2 = slowest with highest resolution.
 * @note Corresponds to the `SWDxtrate` function in the StellarNet API.
 */
void set_x_timing(int rate);

/**
 * @brief Read the EEPROM of the device.
 * @param channel Channel number (0 - 8).
 * @param address EEPROM address to read from.
 * @param data Pointer to a char array to receive the data.
 * @note Corresponds to the `SWDeeRead` function in the StellarNet API.
 */
void read_ee_prom(_int16 channel, _int16 address, char* data);

/**
 * @note Corresponds to the `SWDnew` function in the StellarNet API.
 */
int new_device(int pFunc, int pval1, int pval2, int pval3);

std::string device_name(_int16 chan);
int calibration_coefficients(_int16 channel, double* c1, double* c2, double* c3, double* c4);
void initialize_spectrometer(int integration_time, int scans_to_avg, int smoothing_window, int x_timing, BOOL temperature_compensation);
void get_gain_table(_int16 channel, char* table);
int read_spectrometer_c(_int16 channel, float* data);