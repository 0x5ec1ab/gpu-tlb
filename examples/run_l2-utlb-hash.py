#!/usr/bin/python3

import subprocess
import time
import pagemap

dumper_path = '../dumper/dumper'
extractor_path = '../extractor/extractor'
modifier_path = '../modifier/modifier'

start_va = 0x700000000000
target = 32
dummy_va = 0x7f0000000000

#===============================================================================
# modify PTE for target_va to make it point to the dummy_va's page frame
#===============================================================================
sudo = subprocess.Popen(['sudo', 'echo', ''])
sudo.wait()

# dump GPU memory and retrieve the pagemap from the dump
a_out = subprocess.Popen(['./l2-utlb-hash'])
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

hash_bit = []

for bit in range(7, 47):
  a_out = subprocess.Popen(['./l2-utlb-hash', str(bit)])
  b_out = subprocess.Popen(['nvidia-smi', '-f', '/dev/null'])
  b_out.wait() # in case the driver warms up very slowly
  time.sleep(2)
  subprocess.Popen(['sudo', modifier_path, hex(pte_pa), hex(pte_val)])
  time.sleep(10)
  
  a_out.kill()
  if a_out.poll() == None:
    print("bit {} is needed".format(bit))
    hash_bit.append(bit)
  else:
    print("\tignore bit {}".format(bit))
  
  time.sleep(4)

print("bits {} are used in the hash function\n".format(hash_bit))

for i in range(len(hash_bit)):
  for j in range(i + 1, len(hash_bit)):
    bit0 = hash_bit[i]
    bit1 = hash_bit[j]
    
    a_out = subprocess.Popen(['./l2-utlb-hash', str(bit0), str(bit1)])
    b_out = subprocess.Popen(['nvidia-smi', '-f', '/dev/null'])
    b_out.wait() # in case the driver warms up very slowly
    time.sleep(2)
    subprocess.Popen(['sudo', modifier_path, hex(pte_pa), hex(pte_val)])
    time.sleep(10)
    
    a_out.kill()
    if a_out.poll() != None: 
      print("bits {} and {} are XOR'ed in the hash".format(bit0, bit1))
    time.sleep(4)
  print()

