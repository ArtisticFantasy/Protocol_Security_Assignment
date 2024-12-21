#ifndef NETWORK_FEATURE_H
#define NETWORK_FEATURE_H

#include "Features/Feature-Base.h"
#include <arpa/inet.h>

class Network_Feature : public Feature_Base {
public:
    std::vector<std::string> getFeatures(json j) override;
private:
    static bool registered;
};

#endif
/* NETWORK_FEATURE_H */