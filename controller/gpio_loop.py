import RPi.GPIO as GPIO
import time
import sys

#if len(sys.argv) != 2:
#    print(f"Usage: python {sys.argv[0]} <loop_count>")
#    sys.exit(1)
#
#try:
#    n_loop = int(sys.argv[1])
#    if n_loop <= 0:
#        raise ValueError
#except ValueError:
#    print("Please provide a positive integer for loop_count.")
#    sys.exit(1)

PIN = 5
VETO = 13
period = 0.02     # sec
ontime = 0.0003    # sec   must be > 0.0002
offtime = period - ontime

GPIO.setmode(GPIO.BCM)
GPIO.setup(PIN,  GPIO.OUT)
GPIO.setup(VETO, GPIO.OUT)

try:
    count = 0
    while True:
        if count < 1000:
            GPIO.output(VETO, GPIO.HIGH)
        else:
            GPIO.output(VETO, GPIO.LOW)

        GPIO.output(PIN, GPIO.HIGH)
        time.sleep(ontime)
        GPIO.output(PIN, GPIO.LOW)
        time.sleep(offtime)

        count += 1

except KeyboardInterrupt:
    print("")
    GPIO.output(PIN, GPIO.LOW)
    GPIO.output(VETO, GPIO.LOW)
    print("DONE")

finally:
    GPIO.output(PIN, GPIO.LOW)
    GPIO.output(VETO, GPIO.LOW)
    GPIO.cleanup()

