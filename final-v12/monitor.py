import os
from subprocess import check_output
import time
def get_pid(name):
    return check_output(["pidof",name])

while True:
    print(time.ctime())
    for i in range(1, 2):
        try:
            s = get_pid('m%s' % i).decode('ascii').strip()
            print(f'Program running: m{i}, pid is {s}')
        except:
            'Program dead: m%s, starting..' % i
            print(f'nohup ./m{i} >> m{i}_log 2>&1 &')
            # t = os.popen(f'nohup ./m{i} >> m{i}_log 2>&1 &').read()
            os.system(f'nohup ./m{i} >> m{i}_log 2>&1 &')
            print(f'Start finished: m{i}')

    time.sleep(60) 