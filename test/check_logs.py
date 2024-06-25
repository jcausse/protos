from datetime import datetime
import os

def do_exit(exitcode: int):
    os.system("rm ./logfile.log")
    exit(exitcode)

now = datetime.now()
dtime = now.strftime("%Y/%m/%d-%H:%M:%S")

expected_logs: list[str] = [
    f"[This_is a Test 0123456789 &] @[{dtime}] [ NORMAL ] --> hello\n",
    f"[This_is a Test 0123456789 &] @[{dtime}] [CRITICAL] --> this is a test, a nice test\n",
    f"[This_is a Test 0123456789 &] @[{dtime}] [ NORMAL ] --> -15 I am printing two variables! 2378\n"
]

with open("logfile.log", "r") as logfile:
    logs: list[str] = logfile.readlines()

if len(logs) != len(expected_logs):
    print(f"*** Log count does not match: got {len(logs)}, expected {len(expected_logs)} ***")
    do_exit(1)

for i in range(len(logs)):
    if logs[i] != expected_logs[i]:
        print(f"*** Log does not match: got \"{logs[i]}\", expected \"{expected_logs[i]}\" ***")
        do_exit(1)

do_exit(0)
