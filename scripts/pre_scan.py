import getopt
import sys
import os
import random
import ipaddress

popular_ports = [
    20,   # FTP data transfer
    21,   # FTP control
    22,   # SSH
    23,   # Telnet
    25,   # SMTP
    53,   # DNS
    80,   # HTTP
    110,  # POP3
    143,  # IMAP
    443,  # HTTPS
    445,  # SMB
    993,  # IMAPS
    995,  # POP3S
    1433, # Microsoft SQL Server
    1434, # Microsoft SQL Monitor
    1521, # Oracle Database
    3306, # MySQL
    3389, # Remote Desktop Protocol (RDP)
    5432, # PostgreSQL
    5900, # VNC
    8080, # HTTP (alternative)
    8443  # HTTPS (alternative)
]

def main(argv):
    output_file = ''
    network_file = ''
    send_interface = ''
    source_ip = ''
    gateway_mac = ''
    work_threads = ''
    packet_rate = ''
    scan_percentage = 100
    
    opts, _ = getopt.getopt(argv, "o:n:i:s:g:w:r:p:")
    
    for opt, arg in opts:
        if opt == '-o':
            output_file = arg
        elif opt == '-n':
            network_file = arg
        elif opt == '-i':
            send_interface = arg
        elif opt == '-s':
            source_ip = arg
        elif opt == '-g':
            gateway_mac = arg
        elif opt == '-w':
            work_threads = arg
        elif opt == '-r':
            packet_rate = arg
        elif opt == '-p':
            scan_percentage = int(arg)
    
    if not output_file or not network_file or not send_interface or not source_ip or not gateway_mac or not work_threads or not packet_rate:
        print('Usage: pre_scan.py -o <output_file> -n <network_file> -i <send_interface> -s <source_ip> -g <gateway_mac> -w <work_threads> -r <packet_rate> -p <scan_percentage>')
        sys.exit(1)
    
    if not os.path.isfile(network_file):
        print(f'Network file {network_file} does not exist.')
        sys.exit(1)
    
    with open(network_file, 'r') as f:
        networks = f.readlines()
    
    selected_ips = []
    for network in networks:
        net = ipaddress.ip_network(network.strip())
        ips = list(map(str, net.hosts()))
        selected_ips.extend(random.sample(ips, max(1, int(len(ips) * scan_percentage / 100))))
    
    remaining_ips = set(selected_ips)
    results = []
    for i, port in enumerate(popular_ports):
        print(f'[{i + 1}/{len(popular_ports)}] Scanning port {port} ...')
        with open('temp_network_file', 'w') as f:
            f.writelines("\n".join(remaining_ips) + "\n")
        
        stream = os.popen(f'sudo zmap --max-sendto-failures=1000 --cooldown-time=3 -p {port} -w temp_network_file -r {packet_rate} '
                          f'-i {send_interface} -S {source_ip} -G {gateway_mac} -T {work_threads}')
        output = stream.read()
        for line in output.splitlines():
            ip = line.strip()
            if ip in remaining_ips:
                results.append(ip)
                remaining_ips.remove(ip)
                
    results = sorted(set(results))
    with open(output_file, 'w') as f:
        f.write("\n".join(results) + "\n")
    
    os.remove('temp_network_file')

if __name__ == "__main__":
    main(sys.argv[1:])
