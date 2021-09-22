from collections import OrderedDict
from .states import DefaultState
from lewis.devices import StateMachineDevice


class SimulatedControlChannel:
    temp = 0 
    measurement = 0
    setpoint = 0 
    p = 0 
    i = 0
    d = 0
    max_temp = 0
    pid_mode = 1
    loop_on = 1
    heater_output = 0
    heater_range = 0
    

class SimulatedLakeshore340(StateMachineDevice):

    def _initialize_data(self):
        """
        Initialize all of the device's attributes.
        """
        self.channels = {k:SimulatedControlChannel() for k in ["A", "B", "C", "D"]}

        # For 340 we set these via the backdoor, which is near impossible with the channels dict, so leave in here 
        self.pid_mode = 1
        self.loop_on = True
        self.max_temp = 0
        self.heater_output = 0
        self.heater_range = 0
        self.excitationa = 0
    
    def set_temp(self, channel_name, val):
        self.channels[channel_name].temp = val

    def set_meas(self, channel_name, val):
        self.channels[channel_name].measurement = val

    def _get_state_handlers(self):
        return {'default': DefaultState()}

    def _get_initial_state(self):
        return 'default'

    def _get_transition_handlers(self):
        return OrderedDict([])
