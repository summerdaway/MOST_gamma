ps aux | grep m1
ps aux | grep m1|  awk '{print $2}'|xargs kill -9
ps aux | grep m2
ps aux | grep m2|  awk '{print $2}'|xargs kill -9
ps aux | grep m3
ps aux | grep m3|  awk '{print $2}'|xargs kill -9
ps aux | grep m4
ps aux | grep m4|  awk '{print $2}'|xargs kill -9
ps aux | grep monitor.py
ps aux | grep monitor.py|  awk '{print $2}'|xargs kill -9
ps aux | grep submit
ps aux | grep submit|  awk '{print $2}'|xargs kill -9
ps aux | grep monitor_one.py|  awk '{print $2}'|xargs kill -9
