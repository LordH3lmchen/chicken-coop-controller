import serial
import argparse
import time
from string import Template


class ChickenCoopControllerSerial(serial.Serial):
    """Represents a serial connection to a PLC with the chicken-coop-controller.ino Arduino-Sketch"""

    def get_config(self):
        self.write('#GetConfig;'.encode('ASCII'))
        return self.readlines()
    
    def set_clock(self):
        self.write(time.strftime('#SetClock %d %w %m %Y %H %M %S;')
                  .encode('ASCII'))
        return self.readlines()
        # TODO OFFSET is not implemeted
    
    def set_sunset(self, hour, minute):
        self.write(Template("#SetLightDuration $hour $minute;")
                    .substitute(dict(hour=hour, minute=minute))
                    .encode('ASCII'))
    
    def set_light_duration(self, hours, minutes):
        self.write(Template('#SetLightDuration $hours $minutes;')
                    .substitute(dict(hours=hours, minutes=minutes))
                    .encode('ASCII'))
    
    def set_sunrise_duration(self, minutes):
        assert 120 >= minutes >= 15, "sunrise-duration (%s minutes) is not between 15 and 120 minutes" % minutes
        self.write(Template('#SetSunriseDuration $minutes;')
                    .substitute(dict(minutes=minutes))
                    .encode('ASCII'))

    def set_sunset_duration(self, minutes):
        assert 120 >= minutes >= 15, "sunset-duration (%s minutes) is not between 15 and 120 minutes" % minutes
        self.write(Template('#SetSunsetDuration $minutes;')
                    .substitute(dict(minutes=minutes))
                    .encode('ASCII'))

    def set_max_brightness(self, brightness, channel):
        assert 100 >= brightness >= 0, "brightness (%s) is not between 0 and 100 " % brightness
        assert 2 >= channel >= 0, "channel (%s) is not between 0 and 2" % channel
        self.write(Template('#SetMaxBrightnessForChannel $brightness $channel;')
                    .substitute(dict(brightness=brightness, channel=channel))
                    .encode('ASCII'))

    def set_sunset_delay(self, delay, channel):
        assert 120 >= delay >= 0, "delay (%s minutes) is not betweeen 0 and 120 minutes" % delay
        assert 2 >= channel >= 0, "channel (%s) is not between 0 and 2" % channel
        self.write(Template("#SetSSDelay $delay $channel;")
                    .substitute(dict(delay=delay, channel=channel))
                    .encode('ASCII'))
    
    def set_sunrise_delay(self, delay, channel):
        assert 120 >= delay >= 0, "delay (%s minutes) is not betweeen 0 and 120 minutes" % delay
        assert 2 >= channel >= 0, "channel (%s) is not between 0 and 2" % channel
        self.write(Template("#SetSSDelay $delay $channel;")
                    .substitute(dict(delay=delay, channel=channel))
                    .encde('ASCII'))

    def set_nest_timing(self, openoffset, closeoffset):
        self.write(Template('#SetNestOffset $openoff $closeoff;')
                    .substitute(dict(openoff=openoffset, closeoff=closeoffset))
                    .encode('ASCII'))

    def set_waterflush_duration(self, minutes):
        assert 1 <= minutes <= 60, "Flush-duration (%s minutes) is not between 1 and 60 minutes" % minutes
        self.write(Template("#SetWaterFlushDuration $flush_duration;")
                    .substitute(dict(flush_duration=minutes))
                    .encode('ASCII'))

    def set_feeding_motor_timeout(self, seconds):
        assert 1 <= seconds <= 300, "Flush-duration (%s seconds) is not between 1 and 300 seconds" % seconds
        self.write(Template("#SetFeedMotorTimeoutMillis $motor_timeout;")
                    .substitute(dict(motor_timeout=seconds))
                    .encode('ASCII'))
    
    def set_gate_offset(self, sunrise_open_offset, sunsset_close_offset):
        self.write(Template("#SetGateOffsets $sr_offset $ss_offset;")
                    .substitute(dict(sr_offset=sunrise_open_offset, ss_offset=sunsset_close_offset))
                    .encode('ASCII'))

    def freeze_time_to(self, hour, minute, second):
        assert 0 <= hour <= 23, "hour (%s)is not beween 0 and 23" % hour
        assert 0 <= minute <= 59, "hour (%s)is not beween 0 and 23" % minute
        assert 0 <= second <= 59, "hour (%s)is not beween 0 and 23" % second
        self.write(Template("#FreezeTimeTo $hour $minute $second;")
                    .substitute(dict(hour=hour, minute=minute, second=second))
                    .encode('ASCII'))

    def unfreeze_time(self):
        self.write('#UnfreezeTime;')

    def get_current_light_brightness(self, channel):
        assert 0<= channel <= 2, "channel %s is not valid" % channel
        self.write(Template("#GetCurrentLightBrightness $channel;")
                    .substitute(dict(channel=channel))
                    .encode('ASCII'))
        return self.readlines()

    def move_gate_manual(self, direction):
        assert direction == True or direction == False
        if direction == True:
            cmd_parameter = 1
        else:
            cmd_parameter = 0
        self.write(Template("#MoveGateManual $status;")
                    .substitute(dict(status=cmd_parameter))
                    .encode('ASCII'))

    def move_gave_automatic(self):
        self.write("#MoveGateAutomatic;".encode('ASCII'))


def createParser():
    parser = argparse.ArgumentParser(
                description="connects to the given alarm system via serial.\
                             It sets the ")
    parser.add_argument("-p", "--port",
                        help="Serial port Arduino is on.",
                        type=str,
                        default="/dev/ttyACM1")
    parser.add_argument("-b", "--baud",
                        help="Baudrate (bps) of Arduino.",
                        type=int,
                        default=115200)
    parser.add_argument("-c", "--set-clock",
                        help="set the RTC to the time of the host",
                        type=float,
                        const=0.0,
                        metavar="UTC-OFFSET-IN-HOURS",
                        nargs='?')
    parser.add_argument("--set-sunset",
                        help="set the sunset to given time",
                        type=int,
                        nargs=2,
                        metavar=("HOURS", "MINUTES")
                        )
    parser.add_argument("--light-duration",
                        help="set the light duration",
                        type=int,
                        nargs=2,
                        metavar=("HOURS", "MINUTES")
                        )
    parser.add_argument("--sunrise-duration",
                        help="set the sunrise duration",
                        type=int,
                        nargs=1,
                        metavar="MINUTES"
                        )
    parser.add_argument("--sunset-duration",
                        help="set the sunset duration",
                        type=int,
                        nargs=1,
                        metavar="MINUTES"
                        )
    parser.add_argument("--max-brightness",
                        help="set the max brightness",
                        type=int,
                        nargs=2,
                        metavar=("percent", "channel-nr")
                        )
    parser.add_argument("--sunset-delay",
                        help="set the delay off the sunset off the given channel",
                        type=int,
                        nargs=2,
                        metavar=("DELAY", "channel-nr"),
                        )
    parser.add_argument("--sunrise-delay",
                        help="set the delays off the sunrise off the given channel",
                        type=int,
                        nargs=2,
                        metavar=("DELAY", "channel-nr"),
                        )
    parser.add_argument("--nest-timing",
                        help="time in minutes when the nest opens and closes relative to the sunset",
                        type=int,
                        nargs=2,
                        metavar=("OpenOffset", "CloseOffset"),
                        )
    parser.add_argument("--show-config",
                        help="show the current config from the PLC",
                        action='store_true'
                        )
    parser.add_argument("--water-flush-duration",
                        help="duration of the flush process",
                        type=int,
                        nargs=1,
                        metavar=("minutes")
                        )
    parser.add_argument("--feeding-motor-timeout",
                        help="timeout of the feeding transport motor. Sets how long it takes to timeout the feeding process if the feed is stuck",
                        type=int,
                        nargs=1,
                        metavar=('seconds')
                        )
    parser.add_argument("--gate-offsets",
                        help="defines when the gate (freerange chickens) shoud be opened. the first argument spiecifies the offset from the sunrise (opening the gate). The second argument specifies the offset in minutes from the sunset (closing time of the gate)",
                        type=int,
                        nargs=2,
                        metavar=('sunrise-offset', 'sunset-offset')
                        )
    return parser



def Main():
    parser = createParser()
    args = parser.parse_args()

    cccser = ChickenCoopControllerSerial(args.port, args.baud, timeout=2)
    time.sleep(2)
    if args.set_clock: #TODO implement OFFSET of time (to set a diffrent time than the actual time)
        print('Set Clock to current time\n')
        print('PLC returned:\n')
        print(cccser.set_clock().decode('ASCII'))
    if args.set_sunset:
        cccser.set_sunset(args.set_sunset[0], args.set_sunset[1])
        cccser.flush()
    if args.light_duration:
        cccser.set_light_duration(args.light_duration[0], args.light_duration[1])
        cccser.flush()
    if args.sunrise_duration:
        cccser.set_sunrise_duration(args.sunrise_duration[0])
        cccser.flush()
    if args.sunset_duration:
        cccser.set_sunset_duration(args.sunset_duration[0])
        cccser.flush()
    if args.max_brightness:
        cccser.set_max_brightness(args.max_brightness[0], args.max_brightness[1])
        cccser.flush()
    if args.sunset_delay:
        cccser.set_sunset_delay(args.sunset_delay[0], args.sunset_delay[1])
        cccser.flush()
    if args.sunrise_delay:
        cccser.set_sunrise_delay(args.sunrise_delay[0], args.sunrise_delay[1])
        cccser.flush()
    if args.nest_timing:
        cccser.set_nest_timing(args.nest_timing[0], args.nest_timing[1])
        cccser.flush()
    if args.water_flush_duration:
        cccser.set_waterflush_duration(args.water_flush_duration[0])
        cccser.flush()
    if args.feeding_motor_timeout:
        cccser.set_feeding_motor_timeout(args.feeding_motor_timeout[0])
        cccser.flush()
    if args.gate_offsets:
        cccser.set_gate_offset(args.gate_offsets[0], args.gate_offsets[1])
        cccser.flush()
    if args.show_config:
        for line in cccser.getConfig():
            print(line.decode('ASCII').replace('\r', '').replace('\n',''))

if __name__ == '__main__':

    Main()
