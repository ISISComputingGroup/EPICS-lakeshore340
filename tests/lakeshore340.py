import unittest

import itertools
from parameterized import parameterized

from utils.channel_access import ChannelAccess
from utils.ioc_launcher import get_default_ioc_dir
from utils.test_modes import TestModes
from utils.testing import get_running_lewis_and_ioc, skip_if_recsim, parameterized_list

DEVICE_PREFIX = "LKSH340_01"
_EMULATOR_NAME = "lakeshore340"

IOCS = [
    {
        "name": DEVICE_PREFIX,
        "directory": get_default_ioc_dir("LKSH340"),
        "emulator": _EMULATOR_NAME,
    },
]


TEST_MODES = [TestModes.RECSIM, TestModes.DEVSIM]


SENSORS = ["A", "B", "C", "D"]
TEST_TEMPERATURES = [0.0, 0.0001, 1.0001, 123.456]
TEST_READINGS = TEST_TEMPERATURES

PID_SETTINGS = ["P", "I", "D"]
PID_TEST_VALUES = TEST_TEMPERATURES

PID_MODES = [
    "Manual PID",
    "Zone",
    "Open Loop",
    "Autotune PID",
    "Autotune PI",
    "Autotune P",
]

LOOP_STATES = ["On", "Off"]

HEATER_PERCENTAGES = [0, 0.01, 12.34, 99.99, 100.]
RANGES = ["0%", "0.01%", "0.1%", "1%", "10%", "100%"]
EXCITATIONS = [
    "Off",
    "30 nA", "100 nA", "300 nA",
    "1 uA", "3 uA", "10 uA", "30 uA", "100 uA", "300 uA",
    "1 mA",
    "10 mV", "1 mV"
]


class Lakeshore340Tests(unittest.TestCase):
    """
    Tests for the lakeshore 340 IOC.
    """
    def setUp(self):
        self._lewis, self._ioc = get_running_lewis_and_ioc(_EMULATOR_NAME, DEVICE_PREFIX)
        self.ca = ChannelAccess(device_prefix=DEVICE_PREFIX, default_timeout=15)

    def test_WHEN_device_is_started_THEN_it_is_not_disabled(self):
        self.ca.assert_that_pv_is("DISABLE", "COMMS ENABLED")

    @parameterized.expand(parameterized_list(itertools.product(SENSORS, TEST_TEMPERATURES)))
    @skip_if_recsim("Lewis backdoor not available in recsim")
    def test_WHEN_temperature_set_via_backdoor_THEN_it_can_be_read_back(self, _, sensor, value):
        self._lewis.backdoor_set_on_device("temp_{}".format(sensor.lower()), value)
        self.ca.assert_that_pv_is_number("{}:TEMP".format(sensor.upper()), value)

    @parameterized.expand(parameterized_list(itertools.product(SENSORS, TEST_TEMPERATURES)))
    @skip_if_recsim("Lewis backdoor not available in recsim")
    def test_WHEN_measurement_set_via_backdoor_THEN_it_can_be_read_back(self, _, sensor, value):
        self._lewis.backdoor_set_on_device("measurement_{}".format(sensor.lower()), value)
        self.ca.assert_that_pv_is_number("{}:RDG".format(sensor.upper()), value)

    @parameterized.expand(parameterized_list(TEST_TEMPERATURES))
    def test_WHEN_tset_is_changed_THEN_readback_updates(self, _, val):
        self.ca.assert_setting_setpoint_sets_readback(val, readback_pv="A:TEMP:SP:RBV", set_point_pv="A:TEMP:SP")

    @parameterized.expand(parameterized_list(itertools.product(PID_SETTINGS, PID_TEST_VALUES)))
    def test_WHEN_pid_settings_changed_THEN_can_be_read_back(self, _, setting, value):
        if setting == "D":
            value = int(value)  # Derivative is only allowed to take integer values.

        self.ca.assert_setting_setpoint_sets_readback(value, setting)

    @parameterized.expand(parameterized_list(PID_MODES))
    def test_WHEN_pid_settings_changed_THEN_can_be_read_back(self, _, mode):
        self.ca.assert_setting_setpoint_sets_readback(mode, "PIDMODE")

    @parameterized.expand(parameterized_list(LOOP_STATES))
    def test_WHEN_loop_turned_on_or_off_THEN_can_be_read_back(self, _, loopstate):
        self.ca.assert_setting_setpoint_sets_readback(loopstate, "LOOP")

    @parameterized.expand(parameterized_list(TEST_TEMPERATURES))
    def test_WHEN_max_temperature_set_THEN_can_be_read_back(self, _, temp):
        self.ca.assert_setting_setpoint_sets_readback(temp, "TEMP:MAX")

    @parameterized.expand(parameterized_list(HEATER_PERCENTAGES))
    @skip_if_recsim("Lewis backdoor not available in recsim")
    def test_WHEN_heater_power_set_via_backdoor_THEN_can_be_read_back(self, _, output):
        self._lewis.backdoor_set_on_device("heater_output", output)
        self.ca.assert_that_pv_is_number("OUTPUT", output)

    @parameterized.expand(parameterized_list(RANGES))
    def test_WHEN_heater_range_set_THEN_can_be_read_back(self, _, range):
        self.ca.assert_setting_setpoint_sets_readback(range, "RANGE")

    @parameterized.expand(parameterized_list(EXCITATIONS))
    def test_WHEN_heater_range_set_THEN_can_be_read_back(self, _, excitation):
        self.ca.assert_setting_setpoint_sets_readback(excitation, "EXCITATION")
