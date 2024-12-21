#ifndef L4_FEATURE_H
#define L4_FEATURE_H

#include "Features/Feature-Base.h"

class L4_Feature : public Feature_Base {
public:
    std::vector<std::string> getFeatures(json j) override;
private:
    static bool registered;
};

#endif
/* L4_FEATURE_H */