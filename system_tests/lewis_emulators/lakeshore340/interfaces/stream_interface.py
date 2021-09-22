import enum
from lewis.adapters.stream import StreamInterface
from lewis.core.logging import has_log
from lewis.utils.command_builder import CmdBuilder

_CONTROL_CHANNEL_TO_INDICES = {k:v for k,v in enumerate(["A", "B", "C", "D"])}
_CONTROL_CHANNEL, _CONTROL_CHANNEL_INDEX = "B", 1
_SENSOR_UNITS = 1
_POWERUPENABLE = 1


@has_log
class Lakeshore340StreamInterface(StreamInterface):

    # Commands that we expect via serial during normal operation
    commands = {
        CmdBuilder("get_temperature").escape("KRDG? ").int().eos().build(),
        CmdBuilder("get_measurement").escape("SRDG? ").int().eos().build(),

        CmdBuilder("set_tset").escape("SETP ").int().escape(",").float().eos().build(),
        CmdBuilder("get_tset").escape("SETP? ").int().eos().build(),

        CmdBuilder("set_pid").escape("PID ").int().escape(",").float().escape(",").float().escape(",").int().eos().build(),
        CmdBuilder("get_pid").escape("PID? ").int().eos().build(),

        # 350 only
        CmdBuilder("set_temp_limit").escape("TLIMIT ").int().escape(",").float().eos().build(),
        CmdBuilder("get_temp_limit").escape("TLIMIT? ").int().eos().build(),
        CmdBuilder("get_heater_output_350").escape("HTR? ").int().eos().build(),
        CmdBuilder("set_heater_range_350").escape("RANGE ").int().escape(",").int().eos().build(),
        CmdBuilder("get_heater_range_350").escape("RANGE? ").int().eos().build(),
        CmdBuilder("set_pid_mode").escape("OUTMODE ").int().escape(",").int().eos().build(),
        CmdBuilder("get_pid_mode").escape("OUTMODE? ").int().eos().build(),
        CmdBuilder("set_control_mode").escape("ANALOG ").int().escape(",{},{},".format(_CONTROL_CHANNEL, _SENSOR_UNITS)).int().escape(",{}".format(_POWERUPENABLE)).eos().build(),
        CmdBuilder("get_control_mode").escape("ANALOG? ").int().eos().build(),

        # 340 only 
        CmdBuilder("set_pid_mode").escape("CMODE ").int().escape(",").int().eos().build(),
        CmdBuilder("get_pid_mode").escape("CMODE? ").int().eos().build(),
        CmdBuilder("set_temp_limit").escape("CLIMIT ").int().escape(",").float().eos().build(),
        CmdBuilder("get_temp_limit").escape("CLIMIT? ").int().eos().build(),
        CmdBuilder("get_heater_output").escape("HTR?").eos().build(),
        CmdBuilder("set_heater_range").escape("RANGE ").int().eos().build(),
        CmdBuilder("get_heater_range").escape("RANGE?").eos().build(),
        CmdBuilder("get_excitation_a").escape("INTYPE? A").eos().build(),
        CmdBuilder("set_excitation_a").escape("INTYPE A, 1, , , , ").int().eos().build(),

        CmdBuilder("set_control_mode").escape("CSET ").int().escape(",{},{},".format(_CONTROL_CHANNEL, _SENSOR_UNITS)).int().escape(",{}".format(_POWERUPENABLE)).eos().build(),
        CmdBuilder("get_control_mode").escape("CSET? ").int().eos().build(),
    }

    in_terminator = "\r\n"
    out_terminator = "\r\n"

    def handle_error(self, request, error):
        err_string = "command was: {}, error was: {}: {}\n".format(request, error.__class__.__name__, error)
        print(err_string)
        self.log.error(err_string)
        return err_string

    def get_temperature(self, chan):
        return self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].temp

    def get_measurement(self, chan):
        return self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].measurement

    def set_tset(self, chan, val):
        # Device is 1-indexed
        chan -= 1
        self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].setpoint = float(val)

    def get_tset(self, chan):
        # Device is 1-indexed
        chan -= 1
        return self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].setpoint

    def set_pid(self, chan, p, i, d):
        # Device is 1-indexed
        chan -= 1
        self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].p, self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].i, self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].d = p, i, d

    def get_pid(self, chan):
        # Device is 1-indexed
        chan -= 1
        return "{},{},{}".format(self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].p, self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].i, self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].d)

    def get_pid_mode(self, chan):
        # Device is 1-indexed
        chan -= 1
        return self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].pid_mode

    def set_pid_mode(self, chan, mode):
        if not 1 <= mode <= 6:
            raise ValueError("Mode must be 1-6")
        # Device is 1-indexed
        chan -= 1
        self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].pid_mode = mode

    def get_control_mode(self, chan):
        # Device is 1-indexed
        chan -= 1
        return "{},{},{},{}".format(_CONTROL_CHANNEL, _SENSOR_UNITS, 1 if self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].loop_on else 0, _POWERUPENABLE)

    def set_control_mode(self, chan, val):
        # Device is 1-indexed
        chan -= 1
        self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].loop_on = bool(val)

    def set_temp_limit(self, chan, val):
        # Device is 1-indexed
        chan -= 1
        self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].max_temp = val

    def get_temp_limit(self, chan):
        # Device is 1-indexed
        chan -= 1
        return f"{self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].max_temp},0,0,0,0"

    def get_heater_output(self):
        return "{:.2f}".format(self._device.heater_output)
    
    def get_heater_output_350(self, chan):
        # Device is 1-indexed
        chan -= 1
        return "{:.2f}".format(self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].heater_output)

    def get_heater_range(self):
        return self._device.heater_range

    def get_heater_range_350(self, chan):
        # Device is 1-indexed
        chan -= 1
        return self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].heater_range

    def set_heater_range(self, val):
        if not 0 <= val <= 5:
            raise ValueError("Heater range must be 0-5")
        self._device.heater_range = val
    
    def set_heater_range_350(self, chan, val):
        # Device is 1-indexed
        chan -= 1
        if not 0 <= val <= 5:
            raise ValueError("Heater range must be 0-5")
        self._device.channels[_CONTROL_CHANNEL_TO_INDICES[chan]].heater_range = val

    def get_excitation_a(self):
        return self._device.excitationa

    def set_excitation_a(self, val):
        if not 0 <= val <= 12:
            raise ValueError("Excitations range must be 0-12")
        self._device.excitationa = val
