# coding: utf-8
import datetime
print("Lieferdatum")
dl_day = int(input("Tag: "))
dl_month = int(input("Monat: "))
dl_year = int(input("Jahr: "))
deliver_date = datetime.datetime(year=dl_year, month=dl_month, day=dl_day)
print("\n\nAlter der Junghennen: ")
hen_age = datetime.timedelta(weeks=18, days=int(
    input("Abweichung von 18 Wochen in Tagen: ")))
print("\nGeburtstag der Henne ")
schlupf_date = deliver_date - hen_age
print(schlupf_date)
print("\n\nCommando zum einstellen der Steuerung\n")
print(schlupf_date.strftime("  # SetBirthday %y %m %d;"))
