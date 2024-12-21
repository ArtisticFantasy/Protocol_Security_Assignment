#include "Features/SpatialFeatures/Network-Feature.h"

REGISTER_SPATIALFEATURE(Network_Feature)

extern int prefix_len;

static std::string calculateSubnet(const std::string& ip, int prefixLength) {
    struct in_addr addr;
    if (inet_pton(AF_INET, ip.c_str(), &addr) != 1) {
        throw std::invalid_argument("Invalid IP address");
    }

    uint32_t mask = (prefixLength == 0) ? 0 : (~uint32_t(0) << (32 - prefixLength));
    uint32_t subnet = ntohl(addr.s_addr) & mask;

    struct in_addr subnet_addr;
    subnet_addr.s_addr = htonl(subnet);

    char subnet_str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &subnet_addr, subnet_str, INET_ADDRSTRLEN) == nullptr) {
        throw std::runtime_error("Failed to convert subnet to string");
    }

    std::ostringstream oss;
    oss << subnet_str << "/" << prefixLength;
    return oss.str();
}

std::vector<std::string> Network_Feature::getFeatures(json j) {
    if (!j.contains("saddr")) {
        return {};
    }
    
    return {calculateSubnet(j["saddr"], prefix_len)};
}