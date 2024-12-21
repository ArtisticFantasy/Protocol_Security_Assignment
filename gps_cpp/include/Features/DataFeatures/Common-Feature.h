#ifndef COMMON_FEATURE_H
#define COMMON_FEATURE_H

#include "Features/Feature-Base.h"

class Common_Feature : public Feature_Base {
public:
    std::vector<std::string> getFeatures(json j) override;
private:
    std::string fingerprintFeature(json j);
    std::string windowFeature(json j);
    static bool registered;
};

#endif
/* COMMON_FEATURE_H */