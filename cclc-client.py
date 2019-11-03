import serial
import argparse
import time


class minTempThresholdAction(argparse.Action):
    def __init__(self, option_strings, dest, nargs=None, **kwargs):
        if nargs is not None:
            raise ValueError("nargs not allowed")
            super(minTempThresholdAction, self).\
                __init__(option_strings, dest, **kwargs)

    def __call__(self, parser, namespace, values, option_string=None):
        print('%r %r %r' % (namespace, values, option_string))
        setattr(namespace, self.dest, values)


def Main():
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
                        default=9600)
    parser.add_argument("-c", "--setClock",
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
                        metavar=("percent", "channel nr")
                        )
    parser.add_argument("--sunset-delay",
                        help="set the delay off the sunset off the given channel",
                        type=int,
                        nargs=2,
                        metavar=("DELAY", "ChannelNr"),
                        )
    parser.add_argument("--sunrise-delay",
                        help="set the delay off the sunrise off the given channel",
                        type=int,
                        nargs=2,
                        metavar=("DELAY", "ChannelNr"),
                        )
    parser.add_argument("--nest-timing",
                        help="time when the nest opens and closes relative to the sunset",
                        type=int,
                        nargs=2,
                        metavar=("OpenOffset", "CloseOffset"),
                        )
    args = parser.parse_args()

    print(args.__repr__())

    ser = serial.Serial(args.port, args.baud, timeout=2)
    time.sleep(2)
    if args.setClock is not None:
        print('Set Clock to current time\n')
        # print('#SetClock #day #weekday #month #year #hour #minute #second;')
        ser.write(time.strftime('#SetClock %d %w %m %Y %H %M %S;')
                  .encode('ASCII'))
        print('PLC returned:\n')
        print(ser.readline().decode('ASCII'))


if __name__ == '__main__':
    Main()
