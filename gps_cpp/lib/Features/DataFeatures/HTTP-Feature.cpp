#include "Features/DataFeatures/HTTP-Feature.h"

REGISTER_DATAFEATURE(HTTP_Feature)

std::vector<std::string> HTTP_Feature::getFeatures(json j) {
    if (!j.contains("fingerprint") || !j.contains("data")) {
        return {};
    }
    std::string fingerprint = j["fingerprint"];
    std::string data = j["data"];

    if (fingerprint != "http") {
        return {};
    }
    std::vector<std::string> res;
    std::string cur = "";
    for (char c: data) {
        cur += c;
        if (c == '\n') {
            if (cur.size() > 0) {
                res.push_back("http line: " + cur);
            }
            cur = "";
        }
    }
    if (cur.size() > 0) {
        res.push_back("http line: " + cur);
    }
    return res;
}