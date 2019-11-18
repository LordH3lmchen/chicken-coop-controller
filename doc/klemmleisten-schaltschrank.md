# Klemmleisten für Stallung A und Stallung B

Eingesetzt werden zwei Controllino Automation die in einem Schrank auf zwei Klemmleisten (Durchgangsklemmen) angeschlossen werden.

## Klemmleiste 1 (oben) - Steuerleitungen Motoren

| Klemme | Von         | Zu                                                 | Stall | Potential |
| ------ | ----------- | -------------------------------------------------- | ----- | --------- |
| 1      | K1 - 2      | Mist oben Motor - U                                | B     | L1        |
| 2      | K1 - 4      | Mist oben Motor - V                                | B     | L2        |
| 3      | K1 - 6      | Mist oben Motor - W                                | B     | L3        |
| 5      | K2 - 2      | Mist unten Motor - U                               | B     | L1        |
| 6      | K2 - 4      | Mist unten Motor - V                               | B     | L2        |
| 7      | K2 - 6      | Mist unten Motor - W                               | B     | L3        |
| 8      | K2 - A1     | Mist unten Schalter                                | B     | L-ST      |
| 9      | LS St.sp.   | Div Schalter                                       | B     | L-ST      |
| 4      |             | NotAus                                             | B     | L-ST      |
| 5      |             | Bedienfeld Mist Antrieb Stop Taste                 | B     | L-ST      |
| 6      | K1 - A1     | Bedienfeld Mist Antrieb - Start Oben Taste         | B     | L-ST      |
| 7      | K2 - A1     | Bedienfeld Mist Antrieb - Start Unten Taste        | B     | L-ST      |
| 10     | K3 - 2      | Nest Motor - U                                     | B     | L1        |
| 11     | K3 - 4      | Nest Motor - V                                     | B     | L2        |
| 12     | K3 - 6      | Nest Motor - W                                     | B     | L3        |
| 13     | K5 - 12 NC  | Nest Motor Endschalter (Steuerleitung Nummer 4)    | B     | L-ST      |
| 14     | K3 - A1     | Nest Motor Endschalter (Steuerleitung Nummer 5)    | B     | L-ST      |
| 15     | K5 - 14 NO  | Nest Motor Endschalter (Steuerleitung Nummer 2)    | B     | L-ST      |
| 16     | K4 - A1     | Nest Motor Endschalter (Steuerleitung Nummer 1)    | B     | L-ST      |
| 17     | K6 - 2      | Mist oben Motor - U                                | A     | L1        |
| 18     | K6 - 4      | Mist oben Motor - V                                | A     | L2        |
| 19     | K6 - 6      | Mist oben Motor - W                                | A     | L3        |
| 20     | K6 - A1     | Mist oben Schalter                                 | A     | L-ST      |
| 23     | K7 - 6      | Mist unten Motor - W                               | A     | L3        |
| 21     | K7 - 2      | Mist unten Motor - U                               | A     | L1        |
| 22     | K7 - 4      | Mist unten Motor - V                               | A     | L2        |
| 24     |             | NotAus                                             | A     | L-ST      |
| 25     |             | Bedienfeld Mist Antrieb Stop Taste                 | A     | L-ST      |
| 26     | K1 - A1     | Bedienfeld Mist Antrieb - Start Oben Taste         | A     | L-ST      |
| 27     | K2 - A1     | Bedienfeld Mist Antrieb - Start Unten Taste        | A     | L-ST      |
| 28     | LS St.      | Div Schalter                                       | A     | L-ST      |
| 29     | LS St.      | Div Schalter                                       | A     | L-ST      |
| 30     | K8 - 2      | Nest Motor - U                                     | A     | L1        |
| 31     | K8 - 4      | Nest Motor - V                                     | A     | L2        |
| 32     | K8 - 6      | Nest Motor - W                                     | A     | L3        |
| 33     | K10 - 12 NC | Nest Motor Endschalter (Steuerleitung Nummer 4)    | A     | L-ST      |
| 34     | K8 - A1     | Nest Motor Endschalter (Steuerleitung Nummer 5)    | A     | L-ST      |
| 35     | K10 - 14 NO | Nest Motor Endschalter (Steuerleitung Nummer 2)    | A     | L-ST      |
| 36     | K9 - A1     | Nest Motor Endschalter (Steuerleitung Nummer 1)    | A     | L-ST      |
| 37     | ICS_A - DO0 | Spühlung Stall A                                   | A     | 24V       |
| 38     | ICS_B - DO0 | Spühlung Stall B                                   | B     | 24V       |

## Klemmleiste 2 (unten) - Licht (48V PWM)

|Klemme | Von                         | Zu                                                      | Stall | Potential    |
|-------|-----------------------------|---------------------------------------------------------|-------|--------------|
| 1     | Dimmer - CH1 - OUTPUT LED + | LED Licht Anlage Stall B +                              | B     | 48V          |
| 2     | Dimmer - CH1 - OUTPUT LED - | LED Licht Anlage Stall B -                              | B     | PWM --> GND  |
| 3     | Dimmer - CH2 - OUTPUT LED + | LED Licht Boden Stall B +                               | B     | 48V          |
| 4     | Dimmer - CH2 - OUTPUT LED - | LED Licht Boden Stall B -                               | B     | PWM --> GND  |
| 5     | Dimmer - CH1 - OUTPUT LED + | LED Licht Anlage Stall A +                              | A     | 48V          |
| 6     | Dimmer - CH1 - OUTPUT LED - | LED Licht Anlage Stall A -                              | A     | PWM --> GND  |
| 7     | Dimmer - CH2 - OUTPUT LED + | LED Licht Boden Stall A +                               | A     | 48V          |
| 8     | Dimmer - CH2 - OUTPUT LED - | LED Licht Boden Stall A -                               | A     | PWM --> GND  |
| 9-10  | PSU +48V 500W               | Dimmer Stall A Channel 1&2 + Dimmer Stall B Channel 1&2 | A & B | 48V          |
| 11-18 | PSUs GND                    | Alle Dimmer                                             | A & B | GND          |
| 19-20 | PSU +48V 1000W              | Dimmer Stall A Channel 3&4 + Dimmer Stall B Channel 3&4 | A & B | 48V          |
| 21    | Dimmer - CH3 - OUTPUT LED + | LED Licht Decke Links Stall B +                         | B     | 48V          |
| 22    | Dimmer - CH3 - OUTPUT LED - | LED Licht Decke Links Stall B -                         | B     | PWM --> GND  |
| 23    | Dimmer - CH3 - OUTPUT LED + | LED Licht Decke Rechts Stall B +                        | B     | 48V          |
| 24    | Dimmer - CH3 - OUTPUT LED - | LED Licht Decke Rechts Stall B -                        | B     | PWM --> GND  |
| 25    | Dimmer - CH3 - OUTPUT LED + | LED Licht Decke Links Stall A +                         | B     | 48V          |
| 26    | Dimmer - CH3 - OUTPUT LED - | LED Licht Decke Links Stall A -                         | B     | PWM --> GND  |
| 27    | Dimmer - CH3 - OUTPUT LED + | LED Licht Decke Rechts Stall A +                        | B     | 48V          |
| 28    | Dimmer - CH3 - OUTPUT LED - | LED Licht Decke Rechts Stall A -                        | B     | PWM --> GND  |

## Klemmleiste 3 (unten) - Steuerleitung

|Klemme| Von             | Zu                                    | Farbe Steuerleitung |
|------|-----------------|---------------------------------------|---------------------|
|    1 | PSU 24V         | GND                                   | Schwarz             |
|    2 | PSU GND         | +24V                                  | Rot                 |
|    3 | Controllino DO1 | Wasserventil (24V) - Stall A          | Gelb                |
|    4 | Controllino DO1 | Wasserventil (24V) - Stall B          | Grün                |
|    5 | Controllino DI1 | Futter Sensor (Cap) - Stall A         | Grau                |
|    6 | Controllino DI1 | Futter Sensor (Cap) - Stall B         | Blau                |
|    7 | Controllino DO3 | Futter Schnecken Antrieb - Stall A    | Rosa                |
|    8 | Controllino DO3 | Futter Schnecken Antrieb - Stall B    | Braun               |
|    9 | Controllino DO5 | Futterkette Stall A                   | Weiß                |
|   10 | Controllino DO5 | Futterkette Stall B                   | Violet              |
