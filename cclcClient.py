import serial
import argparse
import time
from string import Template

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

    #print(args.__repr__())

    ser = serial.Serial(args.port, args.baud, timeout=2)
    time.sleep(2)
    if args.set_clock:
        print('Set Clock to current time\n')
        # print('#SetClock #day #weekday #month #year #hour #minute #second;')
        ser.write(time.strftime('#SetClock %d %w %m %Y %H %M %S;')
                  .encode('ASCII'))
        print('PLC returned:\n')
        print(ser.readline().decode('ASCII'))
        # TODO OFFSET is not implemeted
    if args.set_sunset:
        sunset_time = dict(hour=args.set_sunset[0], minute=args.set_sunset[1])
        ser.write(Template('#SetSunset $hour $minute;').substitute(sunset_time).encode('ASCII'))
        ser.flush()
    if args.light_duration:
        light_duration = dict(hours=args.light_duration[0], minutes=args.light_duration[1])
        ser.write(Template("#SetLightDuration $hours $minutes;").substitute(light_duration).encode('ASCII'))
        ser.flush()
    if args.sunrise_duration:
        assert 120 >= args.sunrise_duration[0] >= 15, "sunrise-duration (%s minutes) is not between 15 and 120 minutes" % args.sunrise_duration[0]
        sunsrise_duration = dict(minutes=args.sunrise_duration[0])
        ser.write(Template("#SetSunriseDuration $minutes;").substitute(sunsrise_duration).encode('ASCII'))
        ser.flush()
    if args.sunset_duration:
        assert 120 >= args.sunset_duration[0] >= 15, "sunset-duration (%s minutes) is not between 15 and 120 minutes" % args.sunset_duration[0]
        sunset_duration = dict(minutes=args.sunset_duration[0])
        ser.write(Template("#SetSunsetDuration $minutes;").substitute(sunset_duration).encode('ASCII'))
        ser.flush()
    if args.max_brightness:
        setting = dict(brightness=args.max_brightness[0], channel=args.max_brightness[1])
        assert 100 >= setting['brightness'] >= 0, "brightness (%s) is not between 0 and 100 " % setting['brightness']
        assert 2 >= setting['channel'] >= 0, "channel (%s) is not between 0 and 2" % setting['channel']
        ser.write(Template("#SetMaxBrightnessForChannel $brightness $channel;").substitute(setting).encode('ASCII'))
        ser.flush()
    if args.sunset_delay:
        setting = dict(delay=args.sunset_delay[0], channel=args.sunset_delay[1])
        assert 120 >= setting['delay'] >= 0, "delay (%s minutes) is not betweeen 0 and 120 minutes" % setting['delay']
        assert 2 >= setting['channel'] >= 0, "channel (%s) is not between 0 and 2" % setting['channel']
        ser.write(Template("#SetSSDelay $delay $channel;").substitute(setting).encode('ASCII'))
        ser.flush()
    if args.sunrise_delay:
        setting = dict(delay=args.sunrise_delay[0], channel=args.sunrise_delay[1])
        assert 120 >= setting['delay'] >= 0, "delay (%s minutes) is not betweeen 0 and 120 minutes" % setting['delay']
        assert 2 >= setting['channel'] >= 0, "channel (%s) is not between 0 and 2" % setting['channel']
        ser.write(Template("#SetSSDelay $delay $channel;").substitute(setting).encode('ASCII'))
        ser.flush()
    if args.nest_timing:
        setting = dict(openoff=args.nest_timing[0], closeoff=args.nest_timing[1])
        ser.write(Template("#SetNestOffset $openoff $closeoff;").substitute(setting).encode('ASCII'))
        ser.flush()
    if args.water_flush_duration:
        setting = dict(flush_duration=args.water_flush_duration[0])
        assert 1 <= setting['flush_duration'] <= 60, "Flush-duration (%s minutes) is not between 1 and 60 minutes" % setting['flush_duration']
        ser.write(Template("#SetWaterFlushDuration $flush_duration;").substitute(setting).encode('ASCII'))
        ser.flush()
    if args.feeding_motor_timeout:
        setting = dict(motor_timeout=args.feeding_motor_timeout[0])
        assert 1 <= setting['motor_timeout'] <= 300, "Flush-duration (%s seconds) is not between 1 and 300 seconds" % setting['motor_timeout']
        ser.write(Template("#SetFeedMotorTimeoutMillis $motor_timeout;").substitute(setting).encode('ASCII'))
        ser.flush()
    if args.gate_offsets:
        setting = dict(sr_offset=args.gate_offsets[0], ss_offset=args.gate_offsets[1])
        ser.write(Template("#SetGateOffsets $sr_offset $ss_offset;").substitute(setting).encode('ASCII'))
        ser.flush()
    if args.show_config:
        ser.write('#GetConfig;'.encode('ASCII'))
        ser.flush()
        for line in ser.readlines():
            print(line.decode('ASCII').replace('\r', '').replace('\n',''))

if __name__ == '__main__':

    Main()
