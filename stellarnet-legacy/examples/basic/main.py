#%%
import matplotlib.pyplot as plt
import stellarnet_legacy as sn

#%%
# initialize the server
# this must be run before any other commands
sn.init()

# print the path to the stellarnet resource directory.
# this directory must include `USBDRVD.dll` and `swdll.dll` or `swdll64.dll`.
# these are installed when you install the stellarnet spectrawizard software.
print("StellarNet program folder:", sn.stellarnet_path())

# open the stellarnet library
# this must be run before interacting with the spectrometers
sn.open() 

device_count = sn.device_count()
print(f"Number of connected devices: {device_count}")

#%%
# channels are 1 indexed
channel = 2
sn.initialize_spectrometer(100, 5, sn.SmoothingWindow.NONE, sn.XTiming.MEDIUM, sn.TemperatureCompensation.OFF)
cal_coeffs = sn.calibration_coefficients(channel)
data = sn.read_spectrometer_c(channel)
if data is sn.ScanStatus:
    print(data)
else:
    x = cal_coeffs.wavelengths()
    plt.plot(x, data)
# %%
