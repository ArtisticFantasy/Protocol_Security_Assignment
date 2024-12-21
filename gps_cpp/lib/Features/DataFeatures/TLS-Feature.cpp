#include "Features/DataFeatures/TLS-Feature.h"

REGISTER_DATAFEATURE(TLS_Feature)

std::string TLS_Feature::extract_TLS_cert(std::string data) {
    std::ostringstream oss;
    for (unsigned char c: data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    data = oss.str();
    if (data.size() < 6) {
        return "";
    }
    std::string delim = data.substr(0, 6);

    //find Certificate List
    size_t pos = 0;
    while (pos + 11 < data.size() && data.substr(pos + 10, 2) != "0b") {
        pos = data.find(delim, pos + 6);
        if (pos == std::string::npos) {
            return "";
        }
    }

    if (pos + 11 >= data.size()) {
        return "";
    }

    pos += 18;
    if (pos + 5 >= data.size()) {
        return "";
    }

    int cert_length = std::stoi(data.substr(pos, 6), nullptr, 16) * 2;

    pos += 6;
    if (pos + cert_length - 1 >= data.size()) {
        return data.substr(pos, data.size() - pos);
    }

    return data.substr(pos, cert_length);
}

std::vector<std::string> TLS_Feature::extract_TLS_oid(std::string cert) {
    // split each certificate
    std::vector<std::string> res;
    size_t pos = 0;
    while (pos < cert.size()) {
        if (pos + 5 < cert.size()) {
            int length = std::stoi(cert.substr(pos, 6), nullptr, 16) * 2;
            if (pos + 6 + length <= cert.size()) {
                res.push_back(cert.substr(pos + 6, length));
                pos += length;
            } else {
                if (cert.size() - pos - 6 > 0) {
                    res.push_back(cert.substr(pos + 6, cert.size() - pos - 6));
                }
                break;
            }
        } else {
            break;
        }
    }
    return res;
}

std::vector<std::string> TLS_Feature::getFeatures(json j) {
    if (!j.contains("fingerprint") || !j.contains("data")) {
        return {};
    }
    std::string fingerprint = j["fingerprint"];
    std::string data = j["data"];
    
    if (fingerprint != "tls") {
        return {};
    }
    std::string cert = extract_TLS_cert(data);
    if (cert.empty()) {
        return {};
    }
    std::vector<std::string> oids = extract_TLS_oid(cert), res;
    res.push_back("TLS_cert: " + cert);
    for (std::string oid: oids) {
        res.push_back("TLS_oid: " + oid);
    }
    return res;
}