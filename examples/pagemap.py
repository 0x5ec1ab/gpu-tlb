def retrieve_ptes(pagemap):
  with open(pagemap, 'r') as f:
    lines = f.readlines()
  
  ptes = dict()
  tabs = [0, 0]  
  for ent in lines:
    parts = ent.strip().split('-->')
    if 'PD0' in ent:
      tabs[0] = int(parts[1].split('@')[1], 16)
    elif 'PT' in ent:
      tabs[1] = int(parts[1].split('@')[1], 16)
    elif 'Page' in ent:
      idx = int(parts[0])
      parts = parts[1].split('@')
      addrs = parts[1].split('VA:')
      pa = int(addrs[0], 16)
      va = int(addrs[1], 16)
      epa = tabs[0] + 16 * idx if '2MB' in ent else tabs[1] + 8 * idx
      ptes[va] = [pa, epa]
  return ptes

def make_pte_value(pa):
  # 4KB frame number
  fn = pa >> 12
  val = fn << 8
  val |= 0x0600000000000001
  return val

if __name__ == "__main__":
  parsed_data = retrieve_ptes("pagemap")
  print(parsed_data)


