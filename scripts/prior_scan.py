import subprocess
import sys, os
import getopt

def print_help():
    print("Usage: python prior_scan.py -i <input-file> -o <output-file> -d <send-interface> -s <source-IP> -g <gateway-MAC> -w <work-threads> -l <lzr-bin> -r <packet-rate>")

input_file = None
output_file = None
source_ip = None
gateway_mac = None
interface = None
lzr = None
work_threads = None
packet_rate = None
protocols = "ftp,http,ssh,telnet,tls,vnc,mongodb,mqtt,mssql,mysql,oracle,pop3,postgres,pptp,rdp,redis,rtsp,amqp,dnp3,dns,fox,imap,ipmi,ipp,kubernetes,memcached_ascii,memcached_binary,modbus,newlines,newlines50,siemens,smb,smtp,wait,x11"

if __name__ == '__main__':
    opts, _ = getopt.getopt(sys.argv[1:], "i:o:d:s:g:w:l:r:h", ["input-file", "output-file", "send-interface", "source-IP", "gateway-MAC", "work-threads", "lzr-bin", "packet-rate", "help"])
    for opt, arg in opts:
        if opt in ("-i", "--input-file"):
            input_file = arg
        elif opt in ("-o", "--output-file"):
            output_file = arg
        elif opt in ("-d", "--send-interface"):
            interface = arg
        elif opt in ("-s", "--source-IP"):
            source_ip = arg
        elif opt in ("-g", "--gateway-MAC"):
            gateway_mac = arg
        elif opt in ("-w", "--work-threads"):
            work_threads = int(arg)
        elif opt in ("-l", "--lzr-bin"):
            lzr = arg
        elif opt in ("-r", "--packet-rate"):
            packet_rate = int(arg)
        elif opt in ("-h", "--help"):
            print_help()
            sys.exit(0)
    
    if input_file is None or output_file is None or interface is None or source_ip is None or gateway_mac is None or work_threads is None or lzr is None or packet_rate is None:
        print_help()
        sys.exit(1)
            
    with open("temp_file", 'w') as f:
        pass
    
    scan_range = [[] for i in range(65536)]
    ports = set()
    with open(input_file, 'r') as f:
        lines = f.readlines()
        for line in lines:
            line = line.strip()
            port, net = line.split(',')
            port = int(port.strip())
            net = net.strip()
            if port < 1 or port > 65535:
                continue
            ports.add(port)
            scan_range[port].append(net)
    
    ports = sorted(list(ports))
    tot = len(ports)
    for i, port in enumerate(ports):
        print(f'[{i + 1}/{tot}] Scanning port {port}...')
        with open("temp_net_span", 'w') as f:
            f.write("\n".join(scan_range[port]) + "\n")
        command = ' '.join(['sudo', 'zmap', '--output-filter="success = 1 && repeat = 0"',
                    '--max-sendto-failures=1000', '--cooldown-time=3', '--output-fields="saddr,sport"',
                    '-i', interface, '-S', source_ip, '-G', gateway_mac,
                    '-r', str(packet_rate), '-T', str(work_threads), '-w', 'temp_net_span', '-p', str(port), '>>', 'temp_file'])
        print(command)
        subprocess.run(command, shell=True, stdout=sys.stdout, stderr=sys.stderr)
    
    with open("new_temp_file", 'w') as f:
        pass
    
    with open("temp_file", 'r') as f:
        lines = f.readlines()
        for line in lines:
            if line.startswith("saddr"):
                continue
            line = line.strip()
            line = line.replace(',', ':')
            with open("new_temp_file", 'a') as fw:
                fw.write(line+'\n')
    
    with open(output_file, 'w') as f:
        pass
    
    command = ' '.join(['cat', 'new_temp_file', '|', 'pv', f'-L{packet_rate}', '-l', '--quiet', '|', 'sudo', lzr, '-w', str(work_threads), '-t', '3', '-sendSYNs', '-onlyDataRecord',
                       '-sendInterface', interface, '-sourceIP', source_ip, '-gatewayMac', gateway_mac,
                       '--handshakes', protocols, '-f', output_file])
    print(command)
    subprocess.run(command, shell=True, stdout=sys.stdout, stderr=sys.stderr)
    subprocess.run("rm -f temp_file new_temp_file temp_net_span", shell=True, stdout=sys.stdout, stderr=sys.stderr)