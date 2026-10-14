import enum
import numpy as np

DATA_BUFFER_SIZE = 2051
NUMBER_OF_PIXELS = 2048
EE_PROM_DATA_SIZE = 32


class ScanStatus(enum.Enum):
    SUCCESS = 0
    BUSY = 1
    TIMEOUT = 2
    ERROR = 3


class SmoothingWindow(enum.Enum):
    NONE = 0
    PX_5 = 1
    PX_9 = 2
    PX_17 = 3
    PX_33 = 4


class TemperatureCompensation(enum.Enum):
    OFF = 0
    ON = 1


class XTiming(enum.Enum):
    """Spectrometer readout speed."""

    FAST = 0
    MEDIUM = 1
    SLOW = 2


class CalibrationCoefficients:
    """Calibration coefficients.

    + `c1`, `c2`, and `c3` are used to calculate the spectrometer's measurement points in nanometers.
    + `c4` is currently unused.
    + Values for the coefficients can be found on the bottom spectrometer label.
    """

    def __init__(self, c1=0.2, c2=0.0, c3=300.0, c4=0.0):
        self.c1 = c1
        self.c2 = c2
        self.c3 = c3
        self.c4 = c4

    def wavelengths(self, pixels: int = NUMBER_OF_PIXELS) -> np.ndarray:
        """
        Args:
            pixels (int): Number of pixels. Defaults to 2048.

        Returns:
            np.ndarray: Spectrometer's measurement points in nanometers.
        """
        a0 = self.c3
        a1 = self.c1 / 2
        a2 = self.c2 / 4
        x = [a0 + a1 * i + a2 * i**2 for i in range(pixels)]
        return np.array(x)
