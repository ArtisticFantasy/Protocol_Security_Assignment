#define TBB_PREVIEW_CONCURRENT_ORDERED_CONTAINERS 1
#include "Features/Feature-Base.h"
#include <fstream>
#include <vector>
#include <execution>
#include <algorithm>
#include <unordered_map>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_set.h>
#include <filesystem>
#include <unistd.h>
#include <getopt.h>

using json = nlohmann::json;

unsigned int ip_string_to_int(std::string ip) {
    unsigned int res = 0;
    int cnt = 0;
    for (int i = 0; i < ip.size(); i++) {
        if (ip[i] == '.') {
            res <<= 8;
            res += cnt;
            cnt = 0;
        } else {
            cnt = cnt * 10 + ip[i] - '0';
        }
    }
    res <<= 8;
    res += cnt;
    return res;
}

std::string ip_int_to_string(unsigned int ip) {
    std::string res;
    for (int i = 0; i < 4; i++) {
        res = std::to_string(ip & 255) + res;
        if (i != 3) {
            res = "." + res;
        }
        ip = ip >> 8;
    }
    return res;
}

struct server_tuple {
    unsigned int ip;
    int port;
    std::vector<std::string> servers;
    int beg, end;
};

struct single_server_tuple {
    unsigned int ip;
    int port;
    std::string server;
};

struct pattern_tuple {
    unsigned int ip;
    int port;
    int nport;
    std::string server;
    bool operator < (const pattern_tuple& pt) const {
        if (ip == pt.ip && port == pt.port) {
            return nport < pt.nport;
        }
        if (ip == pt.ip) {
            return port < pt.port;
        }
        return ip < pt.ip;
    }
    bool operator == (const pattern_tuple& pt) const {
        return ip == pt.ip && port == pt.port && nport == pt.nport && server == pt.server;
    }
};

bool cmp(const struct server_tuple& a, const struct server_tuple& b) {
    if (a.ip == b.ip) {
        return a.port < b.port;
    }
    return a.ip < b.ip;
}

long getFileLineNumber(const std::string& file_path) {
    // use wc -l to get the number of lines in the file
    std::string cmd = "wc -l " + file_path + " | awk '{print $1}'";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        std::cerr << "popen failed" << std::endl;
        exit(1);
    }
    char buffer[128];
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL) {
            result += buffer;
        }
    }
    pclose(pipe);
    return std::stol(result);
}


void read_input_json(const std::string& file_path, std::vector<struct server_tuple>& res) {
    std::cout << "Reading input json file " << file_path << "..." << std::endl;
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << file_path << std::endl;
        exit(1);
    }
    int cnt = 0;
    std::string line;
    long lines = getFileLineNumber(file_path);
    while (std::getline(file, line)) {
        cnt ++;
        if(cnt % 10000 == 0 ) {
            std::cout << "\rProcessing " << cnt << "/" << lines << "..." << std::flush;
        }
        try {
            json j = json::parse(line);

            if (!j.empty()) {
                if (!j.contains("saddr") || !j.contains("sport") || !j.contains("fingerprint") || !j.contains("data") || !j.contains("window")) {
                    continue;
                }
                struct server_tuple st = {
                    .ip = ip_string_to_int(j["saddr"]),
                    .port = j["sport"],
                    .servers = Feature_List::getAllFeatures(j)
                };
                res.push_back(st);
            }
        } catch (const json::parse_error& e) {
            std::cerr << "Parsing error: " << e.what() << std::endl;
        }
    }
    std::cout << std::endl;
}

void read_predpat_json(const std::string& file_path, tbb::concurrent_vector<struct pattern_tuple>& res) {
    std::cout << "Reading prediction pattern json file " << file_path << "..." << std::endl;
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << file_path << std::endl;
        exit(1);
    }
    int cnt = 0;
    std::string line;
    long lines = getFileLineNumber(file_path);
    while (std::getline(file, line)) {
        cnt ++;
        if(cnt % 10000 == 0 ) {
            std::cout << "\rProcessing " << cnt << "/" << lines << "..." << std::flush;
        }
        try {
            json j = json::parse(line);

            if (!j.empty()) {
                if (!j.contains("ip") || !j.contains("port") || !j.contains("nport") || !j.contains("server")) {
                    continue;
                }
                struct pattern_tuple pt = {
                    .ip = ip_string_to_int(j["ip"]),
                    .port = j["port"],
                    .nport = j["nport"],
                    .server = j["server"]
                };
                res.push_back(pt);
            }
        } catch (const json::parse_error& e) {
            std::cerr << "Parsing error: " << e.what() << std::endl;
        }
    }
    std::cout << std::endl;
}

#define HITRATE 0.00001
unsigned int MASK = 0xFFFF0000;
std::vector<struct server_tuple> server_tuples;
tbb::concurrent_vector<struct single_server_tuple> single_server_tuples;
tbb::concurrent_hash_map<unsigned int , std::vector<int> > ports_map;
tbb::concurrent_hash_map<unsigned int , std::vector<std::pair<int, std::string> > > port_server_map;
tbb::concurrent_hash_map<std::pair<int, std::string> , int > port_server_count;
tbb::concurrent_hash_map<std::pair<int, std::string> , int > port_server_hit[65536];
tbb::concurrent_vector<struct pattern_tuple> pred_pattern;
tbb::concurrent_set<std::pair<unsigned int, int> > cand_ip_port, scanning_plan1, scanning_plan2;
tbb::concurrent_hash_map<std::pair<unsigned int, int>, int > port_count_in_net;
tbb::concurrent_set<struct pattern_tuple> filtered_pattern;
tbb::concurrent_hash_map<std::pair<unsigned int, std::string>, std::vector<int> > infer_port[65536];

void print_usage() {
    std::cerr << "Usage: ./gps --input-dir <input_folder> --output-file <output_file> --pred-pattern-file <pred-pattern-file> [--part1/part2]" << std::endl;
}

extern int prefix_len;

int main(int argc, char **argv) {
    int opt, long_index;
    std::string input_file;
    std::string output_file;
    std::string pred_pattern_file;
    bool part1 = false;
    bool part2 = false;

    static struct option long_options[] = {
        {"input-file", required_argument, 0, 'i'},
        {"output-file", required_argument, 0, 'o'},
        {"pred-pattern-file", required_argument, 0, 'p'},
        {"prefix-len", required_argument, 0, 'l'},
        {"part1", no_argument, 0, 0},
        {"part2", no_argument, 0, 0},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    while((opt = getopt_long(argc, argv, "i:o:p:l:h", long_options, &long_index)) != -1) {
        switch(opt) {
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'p':
                pred_pattern_file = optarg;
                break;
            case 'l':
                prefix_len = std::stoi(optarg);
                break;
            case 'h':
                print_usage();
                exit(0);
                break;
            case 0:
                if (std::string(long_options[long_index].name) == "part1") {
                    if (part2) {
                        std::cerr << "part1 and part2 cannot be specified at the same time" << std::endl;
                        print_usage();
                        exit(1);
                    }
                    part1 = true;
                }
                else if (std::string(long_options[long_index].name) == "part2") {
                    if (part1) {
                        std::cerr << "part1 and part2 cannot be specified at the same time" << std::endl;
                        print_usage();
                        exit(1);
                    }
                    part2 = true;
                }
                
                break;
            default:
                print_usage();
                exit(1);
        }
    }

    if (!part1 && !part2) {
        std::cerr << "part1 or part2 must be specified" << std::endl;
        print_usage();
        exit(1);
    }

    if (input_file.empty() || output_file.empty() || pred_pattern_file.empty()) {
        print_usage();
        exit(1);
    }

    // check if input file exists
    if (!std::filesystem::exists(input_file)) {
        std::cerr << "Input file does not exist: " << input_file << std::endl;
        exit(1);
    }

    //input_file must be json
    if (input_file.substr(input_file.find_last_of(".") + 1) != "json") {
        std::cerr << "Input file must be a json file" << std::endl;
        exit(1);
    }

    // check if the directory of output file exists
    std::filesystem::path output_path(output_file);
    if (!std::filesystem::exists(output_path.parent_path())) {
        std::cerr << "Output file directory does not exist: " << output_path.parent_path() << std::endl;
        exit(1);
    }

    //pred_pattern_file must be json
    if (pred_pattern_file.substr(pred_pattern_file.find_last_of(".") + 1) != "json") {
        std::cerr << "Pred pattern file must be a json file" << std::endl;
        exit(1);
    }

    if (part2) {
        if (!std::filesystem::exists(pred_pattern_file)) {
            std::cerr << "Pred pattern file does not exist: " << pred_pattern_file << std::endl;
            exit(1);
        }
    } else {
        std::filesystem::path pred_pattern_path(pred_pattern_file);
        if (!std::filesystem::exists(pred_pattern_path.parent_path())) {
            std::cerr << "Pred pattern file directory does not exist: " << pred_pattern_path.parent_path() << std::endl;
            exit(1);
        }
    }

    if (prefix_len < 0 || prefix_len > 32) {
        std::cerr << "Prefix length must be between 0 and 32" << std::endl;
        exit(1);
    }

    MASK = 0xFFFFFFFF << (32 - prefix_len);

    read_input_json(input_file, server_tuples);

    std::sort(std::execution::par, server_tuples.begin(), server_tuples.end(), cmp);

    auto last = std::unique(std::execution::par, server_tuples.begin(), server_tuples.end(), [](const struct server_tuple& a, const struct server_tuple& b) {
        return a.ip == b.ip && a.port == b.port;
    });
    server_tuples.erase(last, server_tuples.end());

    std::for_each(std::execution::par, server_tuples.begin(), server_tuples.end(), [](const struct server_tuple& st) {
        tbb::concurrent_hash_map<unsigned int, std::vector<int>>::accessor a;
        if (ports_map.find(a, st.ip)) {
            a->second.push_back(st.port);
        } else {
            ports_map.insert(a, st.ip);
            a->second.push_back(st.port);
        }
    });

    std::vector<struct server_tuple> new_server_tuples;
    for (int i = 0; i < server_tuples.size(); i++) {
        tbb::concurrent_hash_map<unsigned int, std::vector<int>>::const_accessor ca;
        ports_map.find(ca, server_tuples[i].ip);
        if (ca->second.size() <= 10) {
            new_server_tuples.push_back(server_tuples[i]);
        }
    }

    server_tuples = new_server_tuples;
    new_server_tuples.clear();
    
    if (part1) {
        std::cout<<"GPS part1"<<std::endl;
        
        int tot = 0;
        for (int i = 0; i < server_tuples.size(); i++) {
            server_tuples[i].beg = tot;
            server_tuples[i].end = tot + server_tuples[i].servers.size() - 1;
            tot += server_tuples[i].servers.size();
        }
        single_server_tuples.resize(tot);
        std::for_each(std::execution::par, server_tuples.begin(), server_tuples.end(), [](const struct server_tuple& st) {
            for (int i = st.beg; i <= st.end; i++) {
                single_server_tuples[i] = {
                    .ip = st.ip,
                    .port = st.port,
                    .server = st.servers[i - st.beg]
                };
            }
        });

        std::for_each(std::execution::par, single_server_tuples.begin(), single_server_tuples.end(), [](const struct single_server_tuple& sst) {
            tbb::concurrent_hash_map<std::pair<int, std::string>, int>::accessor a;
            if (port_server_count.find(a, {sst.port, sst.server})) {
                a->second++;
            } else {
                port_server_count.insert(a, {sst.port, sst.server});
                a->second = 1;
            }
            std::vector<int> ports;
            tbb::concurrent_hash_map<unsigned int, std::vector<int>>::const_accessor ca;
            ports_map.find(ca, sst.ip);
            ports = ca->second;
            std::for_each(std::execution::par, ports.begin(), ports.end(), [&port_server_hit, &sst](int port) {
                tbb::concurrent_hash_map<std::pair<int, std::string>, int>::accessor a;
                if (port_server_hit[port].find(a, {sst.port, sst.server})) {
                    a->second++;
                } else {
                    port_server_hit[port].insert(a, {sst.port, sst.server});
                    a->second = 1;
                }
            });
        });

        std::for_each(std::execution::par, single_server_tuples.begin(), single_server_tuples.end(), [](const struct single_server_tuple& sst) {
            tbb::concurrent_hash_map<unsigned int, std::vector<std::pair<int, std::string>>>::accessor a;
            if (port_server_map.find(a, sst.ip)) {
                a->second.push_back({sst.port, sst.server});
            } else {
                port_server_map.insert(a, sst.ip);
                a->second.push_back({sst.port, sst.server});
            }
        });

        std::for_each(std::execution::par, server_tuples.begin(), server_tuples.end(), [](const struct server_tuple& st) {
            std::vector<std::pair<int, std::string>> port_servers;
            {
                tbb::concurrent_hash_map<unsigned int, std::vector<std::pair<int, std::string>>>::const_accessor ca;
                port_server_map.find(ca, st.ip);
                port_servers = ca->second;
            }
            std::vector<int> ports;
            {
                tbb::concurrent_hash_map<unsigned int, std::vector<int>>::const_accessor ca;
                ports_map.find(ca, st.ip);
                ports = ca->second;
            }
            if (ports.size() >= 2 && ports.size() <= 10) {
                int port = st.port;
                auto max_ps = std::transform_reduce(
                    std::execution::par,
                    port_servers.begin(),
                    port_servers.end(),
                    std::make_pair(std::make_pair(-1, std::string("")), -1.0),
                    [](const auto& a, const auto& b) { return a.second > b.second ? a : b; },
                    [&port, &port_server_hit, &port_server_count](const std::pair<int, std::string>& ps) {
                        if (ps.first != port) {
                            tbb::concurrent_hash_map<std::pair<int, std::string>, int>::const_accessor ca1, ca2;
                            port_server_count.find(ca1, ps);
                            port_server_hit[port].find(ca2, ps);
                            double hitrate = 1.0 * ca2->second / ca1->second;
                            return std::make_pair(ps, hitrate);
                        }
                        return std::make_pair(std::make_pair(-1, std::string("")), -1.0);
                    }
                );
                if (max_ps.first.first > 0 && max_ps.second > HITRATE) {
                    pred_pattern.push_back({st.ip, port, max_ps.first.first, max_ps.first.second});
                }
            }
            else if(ports.size() == 1) {
                pred_pattern.push_back({st.ip, -1, st.port, ""});
            }
        });

        std::ofstream pred_out(pred_pattern_file);
        for (auto it = pred_pattern.begin(); it != pred_pattern.end(); it++) {
            json j;
            j["ip"] = ip_int_to_string(it->ip);
            j["port"] = it->port;
            j["nport"] = it->nport;
            j["server"] = it->server;
            pred_out << j.dump() << std::endl;
        }
        pred_out.close();

        std::for_each(std::execution::par, pred_pattern.begin(), pred_pattern.end(), [](const struct pattern_tuple& pt) {
            cand_ip_port.insert({pt.ip, pt.nport});
        });

        std::for_each(std::execution::par, cand_ip_port.begin(), cand_ip_port.end(), [](const std::pair<unsigned int, int>& ip_port) {
            unsigned int ip = ip_port.first;
            int port = ip_port.second;
            unsigned int net = ip & MASK;
            tbb::concurrent_hash_map<std::pair<unsigned int, int>, int>::accessor a;
            if (port_count_in_net.find(a, {net, port})) {
                a->second++;
            } else {
                port_count_in_net.insert(a, {net, port});
                a->second = 1;
            }
        });

        std::for_each(std::execution::par, cand_ip_port.begin(), cand_ip_port.end(), [](const std::pair<unsigned int, int>& ip_port) {
            unsigned int ip = ip_port.first;
            int port = ip_port.second;
            unsigned int net = ip & MASK;
            tbb::concurrent_hash_map<std::pair<unsigned int, int>, int>::const_accessor ca;
            port_count_in_net.find(ca, {net, port});
            if (1.0 * ca->second / (1 << (32 - prefix_len)) > HITRATE) {
                scanning_plan1.insert({net, port});
            }
        });

        std::ofstream out(output_file);
        for (auto it = scanning_plan1.begin(); it != scanning_plan1.end(); it++) {
            out << it->second << "," << ip_int_to_string(it->first) << "/" << std::to_string(prefix_len) << std::endl;
        }
        out.close();
    } else {
        std::cout<<"GPS part2"<<std::endl;
        read_predpat_json(pred_pattern_file, pred_pattern);
        std::for_each(std::execution::par, pred_pattern.begin(), pred_pattern.end(), [](const struct pattern_tuple& pt) {
            cand_ip_port.insert({pt.ip, pt.nport});
        });

        std::for_each(std::execution::par, cand_ip_port.begin(), cand_ip_port.end(), [](const std::pair<unsigned int, int>& ip_port) {
            unsigned int ip = ip_port.first;
            int port = ip_port.second;
            unsigned int net = ip & MASK;
            tbb::concurrent_hash_map<std::pair<unsigned int, int>, int>::accessor a;
            if (port_count_in_net.find(a, {net, port})) {
                a->second++;
            } else {
                port_count_in_net.insert(a, {net, port});
                a->second = 1;
            }
        });

        std::for_each(std::execution::par, cand_ip_port.begin(), cand_ip_port.end(), [](const std::pair<unsigned int, int>& ip_port) {
            unsigned int ip = ip_port.first;
            int port = ip_port.second;
            unsigned int net = ip & MASK;
            tbb::concurrent_hash_map<std::pair<unsigned int, int>, int>::const_accessor ca;
            port_count_in_net.find(ca, {net, port});
            if (1.0 * ca->second / (1 << (32 - prefix_len)) > HITRATE) {
                scanning_plan1.insert({net, port});
            }
        });
        std::for_each(std::execution::par, pred_pattern.begin(), pred_pattern.end(), [](const struct pattern_tuple& pt) {
            if (pt.port != -1) {
                struct pattern_tuple npt = {
                    .ip = pt.ip & MASK,
                    .port = pt.port,
                    .nport = pt.nport,
                    .server = pt.server
                };
                filtered_pattern.insert(npt);
            } 
        });
        std::for_each(std::execution::par, filtered_pattern.begin(), filtered_pattern.end(), [](const struct pattern_tuple& pt) {
            tbb::concurrent_hash_map<std::pair<unsigned int, std::string>, std::vector<int>>::accessor a;
            if (infer_port[pt.nport].find(a, {pt.ip, pt.server})) {
                a->second.push_back(pt.port);
            } else {
                infer_port[pt.nport].insert(a, {pt.ip, pt.server});
                a->second.push_back(pt.port);
            }
        });
        std::for_each(std::execution::par, server_tuples.begin(), server_tuples.end(), [](const struct server_tuple& st) {
            for (int i = 0; i < st.servers.size(); i++) {
                tbb::concurrent_hash_map<std::pair<unsigned int, std::string>, std::vector<int>>::const_accessor ca;
                if (infer_port[st.port].find(ca, {st.ip & MASK, st.servers[i]})) {
                    std::vector<int> ports = ca->second;
                    for (int j = 0; j < ports.size(); j++) {
                        if (scanning_plan1.find({st.ip & MASK, ports[j]}) == scanning_plan1.end()) {
                            scanning_plan2.insert({st.ip, ports[j]});
                        }
                    }
                }
            }
        });
        std::ofstream out(output_file);
        for (auto it = scanning_plan2.begin(); it != scanning_plan2.end(); it++) {
            out << ip_int_to_string(it->first) << ":" << it->second << std::endl;
        }
        out.close();
    }

    return 0;
}