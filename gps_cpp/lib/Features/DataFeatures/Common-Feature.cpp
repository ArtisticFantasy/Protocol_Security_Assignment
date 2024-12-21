#include "Features/DataFeatures/Common-Feature.h"

REGISTER_DATAFEATURE(Common_Feature)

std::string Common_Feature::fingerprintFeature(json j) {
    if (!j.contains("fingerprint")) {
        return "";
    }
    std::string fingerprint = j["fingerprint"];
    return "fingerprint: " + fingerprint;
}

std::string Common_Feature::windowFeature(json j) {
    if (!j.contains("window") || !j["window"].is_number()) {
        return "";
    }
    int window = j["window"];
    return "window: " + std::to_string(window);
}

std::vector<std::string> Common_Feature::getFeatures(json j) {
    std::vector<std::string> res;
    std::string fp = fingerprintFeature(j);
    if (!fp.empty()) {
        res.push_back(fp);
    }
    std::string window = windowFeature(j);
    if (!window.empty()) {
        res.push_back(window);
    }
    return res;
}