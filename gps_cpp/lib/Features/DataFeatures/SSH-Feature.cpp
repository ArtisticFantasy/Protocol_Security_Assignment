#include "Features/DataFeatures/SSH-Feature.h"

REGISTER_DATAFEATURE(SSH_Feature)

std::vector<std::string> SSH_Feature::getFeatures(json j) {
    if (!j.contains("fingerprint") || !j.contains("data")) {
        return {};
    }
    std::string fingerprint = j["fingerprint"];
    std::string data = j["data"];

    std::vector<std::string> res;
    if (fingerprint == "ssh") {
        res.push_back("ssh banner: " + data);
    }
    return res;
}