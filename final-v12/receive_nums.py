import requests
import time
from collections import defaultdict
from threading import Thread
from queue import Queue
import sys

fin = requests.Session().get("http://172.1.1.119:10001", stream=True, headers=None)
for c in fin.iter_content():
    sys.stdout.write(c.decode('ascii'))
