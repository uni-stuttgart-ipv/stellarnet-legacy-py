import ctypes as c
from importlib.resources import files

dll_path = files("stellarnet_legacy.data").joinpath("stellarnet_api.dll")
dll = c.WinDLL(str(dll_path))
dll.init.argtypes = [c.c_char_p]
dll.init.restype = None
dll.kill.restype = None
dll.open.restype = c.c_int
dll.close.restype = None
dll.enable_logging.argtypes = [c.c_char_p]
dll.enable_logging.restype = None
dll.disable_logging.restype = None
dll.stellarnet_path.restype = c.c_char_p
dll.device_count.restype = c.c_int
dll.set_integration_time.argtypes = [c.c_int]
dll.set_integration_time.restype = None
dll.scan.argtypes = [c.c_int16, c.POINTER(c.c_float)]
dll.scan.restype = c.c_int
dll.scan_c.argtypes = [c.c_int16, c.POINTER(c.c_float)]
dll.scan_c.restype = c.c_int
dll.read_ee_prom.argtypes = [c.c_int16, c.c_int16, c.c_char_p]
dll.read_ee_prom.restype = None
dll.set_update.argtypes = [c.c_int, c.c_int, c.c_int]
dll.set_update.restype = c.c_int
dll.set_x_timing.argtypes = [c.c_int]
dll.set_x_timing.restype = None

dll.device_name.argtypes = [c.c_int16]
dll.device_name.restype = c.c_char_p
dll.calibration_coefficients.argtypes = [c.c_int16, c.POINTER(c.c_double)]
dll.calibration_coefficients.restype = c.c_int
dll.initialize_spectrometer.argtypes = [c.c_int, c.c_int, c.c_int, c.c_int, c.c_bool]
dll.initialize_spectrometer.restype = None
dll.gain_table.argtypes = [c.c_int16]
dll.gain_table.restype = c.POINTER(c.c_int)
dll.read_spectrometer_c.argtypes = [c.c_int16, c.POINTER(c.c_float)]
dll.read_spectrometer_c.restype = int