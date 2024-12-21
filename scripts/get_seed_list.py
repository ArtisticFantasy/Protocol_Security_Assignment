import ipaddress
import random
import getopt
import sys, os

def generate_ip_list(prescan_file, ratio):
    with open(prescan_file, 'r') as file:
        ips = file.readlines()
    
    ip_list = list(set([ip.strip() for ip in ips]))
    
    for ip in ip_list:
        if ip == '':
            ip_list.remove(ip)
    
    num_ips = int(len(ip_list) * ratio)
    ip_list = random.sample(ip_list, num_ips)
            
    return ip_list

if __name__ == "__main__":
    opts, args = getopt.getopt(sys.argv[1:], "i:o:r:", ["input", "output", "ratio"])
    input_file = ''
    output_file = ''
    ratio = -1e9
    for opt, arg in opts:
        if opt in ("-i", "--input"):
            input_file = arg
        elif opt in ("-o", "--output"):
            output_file = arg
        elif opt in ("-r", "--ratio"):
            ratio = float(arg)
        else:
            print('Usage: python get_seed_list.py -i <input-file> -o <output-file> -r <ratio>')
            sys.exit(1)

    if input_file == '' or ratio == -1e9:
        print('Usage: python get_seed_list.py -i <input-file> -o <output-file> -r <ratio>')
        sys.exit(1)
    
    if not input_file.endswith('.txt'):
        print('Input file must be a .txt file')
        sys.exit(1)
        
    if not output_file.endswith('.txt'):
        print('Output file must be a .txt file')
        sys.exit(1)
        
    if not os.path.exists(input_file):
        print('Input file does not exist')
        sys.exit(1)
    
    if ratio < 0 or ratio > 1:
        print('Ratio must be between 0 and 1')
        sys.exit(1)
    
    ip_list = generate_ip_list(input_file, ratio)
    with open(output_file, 'w') as file:
        for ip in ip_list:
            for port in range(1, 65536):
                file.write(f'{ip}:{port}\n')