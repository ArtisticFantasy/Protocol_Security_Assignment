import json
import os
import sys
import getopt
import random

reserved_ports = [20, 21, 22, 23, 25, 53, 67, 68, 69, 80, 110, 119, 123, 135, 137, 138, 139, 143, 161, 162, 179, 194, 201, 443, 445, 465, 514, 515, 587, 993, 995, 1080, 1194, 1433, 1434, 1521, 1723, 2049, 3306, 3389, 5432, 5900, 8080, 8443]

def read_json_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as file:
        return [json.loads(line) for line in file]

def write_json_file(filepath, data):
    with open(filepath, 'w', encoding='utf-8') as file:
        for item in data:
            file.write(json.dumps(item, ensure_ascii=False) + '\n')

def merge_and_deduplicate(json_files):
    combined_data = []
    seen = set()
    
    for file in json_files:
        data = read_json_file(file)
        for item in data:
            key = (item['saddr'], item['sport'])
            if key not in seen:
                seen.add(key)
                combined_data.append({
                    "saddr": item['saddr'],
                    "sport": item['sport'],
                    "fingerprint": item['fingerprint'],
                    "window": item['window'],
                    "data": item['data'],
                })
    
    return combined_data

if __name__ == "__main__":
    opts, _ = getopt.getopt(sys.argv[1:], "i:o:", ["input-file", "output-file"])
    for opt, arg in opts:
        if opt in ("-i", "--input-file"):
            input_file = arg
        elif opt in ("-o", "--output-file"):
            output_file = arg
        else:
            print('Usage: python combine_results.py -i <input_file> -o <output_file>')
            sys.exit(1)
            
    if input_file is None or output_file is None:
        print('Usage: python combine_results.py -i <input_file> -o <output_file>')
        sys.exit(1)
    
    if not input_file.endswith('.json'):
        print('Input file must be a json file')
        sys.exit(1)
        
    if not output_file.endswith('.txt'):
        print('Output file must be a .txt file')
        sys.exit(1)
        
    if not os.path.exists(input_file):
        print(f'Input file {input_file} does not exist')
        sys.exit(1)
    
    items = read_json_file(input_file)
    ips = set()
    ip_ports = set()
    for item in items:
        ip_ports.add((item['saddr'], item['sport']))
        ips.add(item['saddr'])
    
    seed_list= []
    for ip in ips:
        for port in reserved_ports:
            if (ip, port) not in ip_ports:
                seed_list.append(f'{ip}:{port}')
        
        ports = list(range(1, 65536))
        random_ports = random.sample(ports, 50)
        
        for port in random_ports:
            if (ip, port) not in ip_ports:
                seed_list.append(f'{ip}:{port}')
    
    with open(output_file, 'w') as f:
        f.write('\n'.join(seed_list) + '\n')