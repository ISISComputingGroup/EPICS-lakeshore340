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

        self.channels = {k:SimulatedControlChannel() for k in ["a", "b", "c", "d"]}

        self.temp_a = 0
        self.temp_b = 0
        self.temp_c = 0
        self.temp_d = 0
        self.measurement_a = 0
        self.measurement_b = 0
        self.measurement_c = 0
        self.measurement_d = 0

        self.tset_a = 0
        self.tset_b = 0 
        self.tset_c = 0
        self.test_d = 0 

        self.p, self.i, self.d = 0, 0, 0

        self.pid_mode = 1
        self.loop_on = True

        self.max_temp = 0
        self.heater_output = 0
        self.heater_range = 0
        self.excitationa = 0

    def _get_state_handlers(self):
        return {'default': DefaultState()}

    def _get_initial_state(self):
        return 'default'

    def _get_transition_handlers(self):
        return OrderedDict([])
