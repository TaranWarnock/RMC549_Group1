from Common.FSW_Common import *
import smbus

class I2C_Photosensor(FlightSoftwareParent):
    def __init__(self, logging_object: Logger, i2c_address=0x39):
        super().__init__("I2CPhotosensor", logging_object)
        if i2c_address not in [0x39, 0x49, 0x59]:
            self.log_warning(f'invalid TSL2561 I2C address {i2c_address}')
        self._addr = i2c_address
        self._initialize()

    def _initialize(self):
        """ Initializing commands. Taken from TSL2561.py script. """

        # Get I2C bus
        self._bus = smbus.SMBus(1)

        # TSL2561 address, 0x39(57)
        # Select control register, 0x00(00) with command register, 0x80(128)
        #		0x03(03)	Power ON mode
        self._bus.write_byte_data(self._addr, 0x00 | 0x80, 0x03)
        # TSL2561 address, 0x39(57)
        # Select timing register, 0x01(01) with command register, 0x80(128)
        #		0x02(02)	Nominal integration time = 402ms
        self._bus.write_byte_data(self._addr, 0x01 | 0x80, 0x02)

    def _get_data(self):
        """ Read the data from the sensor. Returns lux values (VIS, IR). Taken from TSL2561.py script. """

        # Read data back from 0x0C(12) with command register, 0x80(128), 2 bytes
        # ch0 LSB, ch0 MSB
        data = self._bus.read_i2c_block_data(self._addr, 0x0C | 0x80, 2)

        # Read data back from 0x0E(14) with command register, 0x80(128), 2 bytes
        # ch1 LSB, ch1 MSB
        data1 = self._bus.read_i2c_block_data(self._addr, 0x0E | 0x80, 2)

        # Convert the data
        ch0 = data[1] * 256 + data[0]
        ch1 = data1[1] * 256 + data1[0]

        return ch0 - ch1, ch1


