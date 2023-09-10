#!/usr/bin/python3

import subprocess
import time
import pagemap

idx_grp = [
  [20, 27, 34, 41],
  [21, 28, 35, 42],
  [22, 29, 36, 43],
  [23, 30, 37, 44],
  [24, 31, 38, 45],
  [25, 32, 39, 46],
  [26, 33, 40]
]

dumper_path = '../dumper/dumper'
extractor_path = '../extractor/extractor'
modifier_path = '../modifier/modifier'

start_va = 0x700000000000
target = 32
dummy_va = 0x7f0000000000
page_num = 135000

#===============================================================================
# modify PTE for target_va to make it point to the dummy_va's page frame
#===============================================================================
def match(target_va, candid_va):
  for idx in idx_grp:
    bit_vec0 = [(target_va >> i) & 0x01 for i in idx]
    bit_vec1 = [(candid_va >> i) & 0x01 for i in idx]
    x = 0
    for k in bit_vec0:
      x ^= k
    y = 0
    for k in bit_vec1:
      y ^= k
    if x != y:
      return False
  return True

sudo = subprocess.Popen(['sudo', 'echo', ''])
sudo.wait()

# dump GPU memory and retrieve the pagemap from the dump
a_out = subprocess.Popen(['./l3-utlb-set', str(target)])
b_out = subprocess.Popen(['nvidia-smi', '-f', '/dev/null']) 
b_out.wait() # in case the driver warms up very slowly
time.sleep(2)
tmp_path = '/tmp/'
dump = subprocess.Popen([dumper_path, '-b', '0x10000000', '-o', tmp_path + 'dump'])
dump.wait()
pagemap_file = open(tmp_path + 'pagemap', 'w')
extract = subprocess.Popen([extractor_path, tmp_path + 'dump'], stdout=pagemap_file)
extract.wait()
pagemap_file.close()
a_out.kill()
time.sleep(5)

target_va = start_va + target * (1 * 1024 * 1024)
ptes = pagemap.retrieve_ptes(tmp_path + 'pagemap')
pte_pa = ptes[target_va][1]
pte_val = pagemap.make_pte_value(ptes[dummy_va][0])

candidates = []
for i in range(page_num):
  candid_va = start_va + i * (1 * 1024 * 1024)
  if match(target_va, candid_va):
    candidates.append(i)

results = list()
head = 0
tail = len(candidates) - 1
current = -(-(tail + head) // 2)
while tail >= head:
  print("current:" + str(current) + "\thead:" + str(head) + "\ttail:" + str(tail))
  print(results)
  page_vec = [candidates[i] for i in range(current) if candidates[i] != target] + results
  page_vec.sort()
  
  tmp = [str(i) for i in page_vec]
  a_out = subprocess.Popen(['./l3-utlb-set', str(target)] + tmp)
  b_out = subprocess.Popen(['nvidia-smi', '-f', '/dev/null'])
  b_out.wait() # in case the driver warms up very slowly
  time.sleep(2)
  subprocess.Popen(['sudo', modifier_path, hex(pte_pa), hex(pte_val)])
  time.sleep(10)
  
  a_out.kill()
  if a_out.poll() == None:
    if head == tail:
      print("\t\tfind " + str(candidates[current]) + " at " + str(current))
      results.append(candidates[current])
      head = 0
      tail = current - 1
    else:
      print("\tmove head to " + str(current))
      head = current
  else:
    print("\tmove tail to " + str(current - 1))
    tail = current - 1
  current = -(-(tail + head) // 2)
  
  time.sleep(4)  
  
print(results)


