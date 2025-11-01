# SPDX-FileCopyrightText: 2025-present Brian Carlsen <carlsen.bri@gmail.com>
#
# SPDX-License-Identifier: GPL-3.0-or-later
import platform

if platform.system() != "Windows":
    raise OSError("stellarnet_legacy can only be used on Windows.")

from .kinds import (
    ScanStatus,
    SmoothingWindow,
    TemperatureCompensation,
    XTiming,
    CalibrationCoefficients,
)

from .api import (
    init, 
    kill, 
    open, 
    close,
    enable_logging,
    disable_logging,
    stellarnet_path,
    device_count,
    set_integration_time,
    scan,
    scan_c,
    read_ee_prom,
    set_update,
    set_x_timing,
    device_name,
    calibration_coefficients,
    initialize_spectrometer,
    gain_table,
    read_spectrometer_c,
)