from subprocess import run, PIPE
import sys

def get_loopback_dev_num():
  p = run(["losetup"], shell=True, stdout=PIPE, text=True)
  max_loopback = 0
  for line in p.stdout.splitlines():
    try:
      device = line.split()[0] 
      if device.startswith("/dev/loop"):
        index = int(device.lstrip("/dev/loop"))
        max_loopback = max(max_loopback, index)
    except:
        continue

  return max_loopback + 1


if __name__ == "__main__":
  dev_num = get_loopback_dev_num()
  if len(sys.argv) > 1:
    print(dev_num + 1)
  else:
    print(dev_num)