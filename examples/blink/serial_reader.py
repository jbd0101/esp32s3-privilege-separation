import serial
import time
import pandas as pd

s = serial.Serial('/dev/ttyUSB0', baudrate=115200)

buffer = ""
start_time = 0
cnt = 0

measures = []
MAX_ITER = 100
outfile = 'data/syscall_10.csv'
while True:
    if cnt == MAX_ITER:
        break
    data = s.read()
    if data == b'\n':
        if "end" in buffer:
            end_time = time.time() - start_time
            measures.append(end_time)
            print("Recorded time for index {}".format(cnt))
            start_time = 0
            cnt += 1
        elif "start" in buffer:
            if start_time == 0:
                start_time = time.time()
        buffer = ""
    else:
        buffer += data.decode()  # Append the received data to the buffer

df = pd.DataFrame(measures, columns=['time'])
df.to_csv(outfile, index=False)

