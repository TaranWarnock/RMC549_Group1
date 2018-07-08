from Common.FSW_Common import *
try:
    import smbus
except:
    pass


"""
Class which handles the i2c light sensors for the RMC 549 Balloons

Written by Lukas Fehr, 2018-07-05, extremely minor adaptation by Daniel Letros, 2018-07-05
"""

class I2C_Photosensor(object):
    def __init__(self, i2c_address: hex=0x39, class_name: str="I2CPhotosensor") -> None:
        super().__init__()
        self.class_name           = class_name
        self.system_name          = socket.gethostname()
        self.data_header_addition = "%sVIS, %sIR" % (self.class_name, self.class_name)
        if i2c_address not in [0x39, 0x49, 0x59]:
            self.sensor_is_valid = False
        elif socket.gethostname() != "Rocky":
            self.sensor_is_valid = False
        else:
            self.sensor_is_valid = True
            self._addr = i2c_address
            self._initialize()

    def _initialize(self) -> None:
        """ Initializing commands. Taken from TSL2561.py script. """
        if self.sensor_is_valid:
            try:
                # Get I2C bus
                self._bus = smbus.SMBus(1)

                # TSL2561 address, 0x39(57)
                # Select control register, 0x00(00) with command register, 0x80(128)
                #		0x03(03)	Power ON mode
                self._bus.write_byte_data(self._addr, 0x00 | 0x80, 0x03)
                # TSL2561 address, 0x39(57)
                # Select timing register, 0x01(01) with command register, 0x80(128)
                #		0x02(02)	Nominal integration time = 402ms (not doing this)

                # 13 ms, 0 gain (0x00) (0x10 would be 16x gain 13 ms, 0x01 would be 1x gain and 101 ms, etc)
                self._bus.write_byte_data(self._addr, 0x01 | 0x80, 0x00)
            except:
                self.sensor_is_valid = False

    def _get_data(self):
        """ Read the data from the sensor. Returns lux values (VIS, IR). Taken from TSL2561.py script. """
        try:
            if self.sensor_is_valid:
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
            else:
                return None
        except:
            self.sensor_is_valid = False
            return None


