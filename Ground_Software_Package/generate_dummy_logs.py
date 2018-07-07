# generate log files to test live plots
import time

# get the data to use
file_name = r'C:\Users\puetz\Desktop\livetest.txt'
data = open(file_name, 'rb').readlines()
for i in range(len(data)):
    data[i] = data[i].decode()[:-2]

# save the data at a normal rate
fil = r'C:\Users\puetz\Desktop\Telemtry_logs\Test\test.txt'
for i in range(len(data)):
    with open(fil, 'a') as file:
        file.write(data[i]+"\n")
    time.sleep(3)
    print("did it")