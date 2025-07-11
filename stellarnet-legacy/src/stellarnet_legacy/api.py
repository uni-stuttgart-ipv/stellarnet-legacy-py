from typing import Union, Optional
from importlib.resources import files, path
import ctypes as c
import numpy as np
from . import kinds
from .lib import dll

def init() -> None:
    """Start the message server.
    Must run this before any other commands.
    """
    module_path = str(files("stellarnet_legacy.data").joinpath("stellarnet_api_server.exe"))
    module_path = c.create_string_buffer(module_path.encode())
    dll.init(module_path)

def kill() -> None:
    """Kill the message server.
    """
    dll.kill

def open() -> None:
    """Open the StellarNet DLL. 
    Required to run this before call functions that interact with the spectrometers.
    """
    res = dll.open()
    if res != True:
        RuntimeError("Could not open Stellarnet DLL")

def close() -> None:
    """Close the StellarNet DLL.
    """
    dll.close()

def stellarnet_path() -> Optional[str]:
    """
    Returns:
        Optional[str]: Path to StellarNet program resources or None if not found.
    """
    path = dll.stellarnet_path()
    return path.decode() if path else None
    
    

def device_count() -> int:
    """
    Returns:
        int: Number of connected devices.
    """
    return dll.device_count()

def set_integration_time(time: int) -> None:
    dll.set_integration_time(time)

def scan(channel: int) -> Union[np.ndarray, kinds.ScanStatus]:
    """Acquire a spectrometer's scan data.

    Note:
        You probably don't want to call this directly.
        use `read_spectrometer` instead.

    Args:
        channel (int): Spectrometer to read from.

    Returns:
        Union[np.ndarray, kinds.ScanStatus]: Intensity values or spectrometer status if values could not be obained.
    """
    data = (c.c_float * kinds.DATA_BUFFER_SIZE)()
    status = dll.scan(channel, data)
    status = kinds.ScanStatus(status)
    if status is not kinds.ScanStatus.SUCCESS:
        return status
    
    return np.array(data)

def scan_c(channel: int) -> Union[np.ndarray, kinds.ScanStatus]:
    """Acquire spectrometer scan data 

    Note:
        You probably don't want to call this directly.
        use `read_spectrometer` instead.

    Args:
        channel (int): Spectrometer to read from.

    Returns:
        Union[np.ndarray, kinds.ScanStatus]: Intensity values or spectrometer status if values could not be obained.
    """
    data = (c.c_float * kinds.DATA_BUFFER_SIZE)()
    status = dll.scan_c(channel, data)
    status = kinds.ScanStatus(status)
    if status is not kinds.ScanStatus.SUCCESS:
        return status
    
    return np.array(data)

def read_ee_prom(channel: int, address: int) -> str:
    data = c.create_string_buffer(kinds.EE_PROM_DATA_SIZE)
    dll.read_ee_prom(channel, address, data)
    return data.value

def set_update(n_scans: int, smoothing_window: kinds.SmoothingWindow, temperature_compensation: kinds.TemperatureCompensation) -> int:
    res = dll.set_update(n_scans, smoothing_window, temperature_compensation)
    return res

def set_x_timing(rate: kinds.XTiming) -> None:
    """Set spectrometer readout speed.

    Args:
        rate (kinds.XTiming): Readout speed.
    """
    dll.set_x_timing(rate)

def device_name(channel: int) -> str:
    name = dll.device_name(channel)
    return name.decode()

def calibration_coefficients(channel: int) -> kinds.CalibrationCoefficients:
    """Get callibration coefficients for a spectrometer.

    Args:
        channel (int): Spectrometer to read form.

    Returns:
        kinds.CalibrationCoefficients: Calibration coefficients.
    """
    coeffs = (c.c_double * 4)()
    resp = dll.calibration_coefficients(channel, coeffs)
    if resp == 0:
        return kinds.CalibrationCoefficients(c1=coeffs[0], c2=coeffs[1], c3=coeffs[2], c4=coeffs[3])
    else:
        return kinds.CalibrationCoefficients()

def initialize_spectrometer(
    integration_time: int,
    scans_to_avg: int,
    smoothing_window: kinds.SmoothingWindow, 
    x_timing: kinds.XTiming, 
    temperature_compensation: kinds.TemperatureCompensation
) -> None:
    """Initialize a spectrometer whith the given parameters.

    Args:
        integration_time (int): Time for a single integration in milliseconds.
        scans_to_avg (int): Number of scans to average data over.
        smoothing_window (kinds.SmoothingWindow): Smoothing window size for data.
        x_timing (kinds.XTiming): Readout time.
        temperature_compensation (kinds.TemperatureCompensation): Temperature compensation.

    Raises:
        ValueError: If integration time (`integration_time`) and readout speed (`x_timing`) are incompatible.
    """
    if (x_timing is kinds.XTiming.FAST) and (integration_time < 1):
        raise ValueError("Minimum integration time for XTiming.FAST is 1 ms.")
    if (x_timing is kinds.XTiming.MEDIUM) and (integration_time < 15):
        raise ValueError("Minimum integration time for XTiming.MEDIUM is 15 ms.")
    if (x_timing is kinds.XTiming.SLOW) and (integration_time < 30):
        raise ValueError("Minimum integration time for XTiming.SLOW is 30 ms.")

    dll.initialize_spectrometer(
        integration_time, 
        scans_to_avg, 
        smoothing_window.value,
        x_timing.value,
        temperature_compensation.value,
    )

def gain_table(channel: int) -> tuple[tuple[int, int, int, int, int], tuple[int, int, int, int, int]]:
    """Get gain table values.

    Args:
        channel (int): Spectrometer to read from.

    Returns:
        tuple[tuple[int, int, int, int, int], tuple[int, int, int, int, int]]: (gain values, base values)
    """
    GAIN_TABLE_SIZE = 10
    table = dll.gain_table(channel)
    gain_values = tuple(table[:5])
    base_values = tuple(table[5:GAIN_TABLE_SIZE])
    return (gain_values, base_values)
    
def read_spectrometer_c(channel: int) -> Union[np.ndarray, kinds.ScanStatus]:
    """Read a spectrum.

    Args:
        channel (int): Sprectrometer to read from.

    Returns:
        Union[np.ndarray, kinds.ScanStatus]: Intensity values or spectrometer status if values could not be obained.
    """
    data = (c.c_float * kinds.DATA_BUFFER_SIZE)()
    status = dll.read_spectrometer_c(channel, data)
    status = kinds.ScanStatus(status)
    if status is not kinds.ScanStatus.SUCCESS:
        return status
    
    return np.array(data[:kinds.NUMBER_OF_PIXELS])