import RPi.GPIO as GPIO
import time

PIN = 5
VETO = 13
period = 0.005     # sec
ontime = 0.0003    # sec > 0.0002
offtime = period - ontime

GPIO.setmode(GPIO.BCM)
GPIO.setup(PIN,  GPIO.OUT)
GPIO.setup(VETO, GPIO.OUT)

### PEDESTAL MODE
#GPIO.output(VETO, GPIO.HIGH)
### LED MODE
GPIO.output(VETO, GPIO.LOW)


try:
    while True:
        GPIO.output(PIN, GPIO.HIGH)
        time.sleep(ontime)
        GPIO.output(PIN, GPIO.LOW)
        time.sleep(offtime)

except KeyboardInterrupt:
    print("")
    GPIO.output(PIN, GPIO.LOW)
    GPIO.output(VETO, GPIO.LOW)
    print("DONE")

finally:
    GPIO.output(PIN, GPIO.LOW)
    GPIO.output(VETO, GPIO.LOW)
    GPIO.cleanup()

