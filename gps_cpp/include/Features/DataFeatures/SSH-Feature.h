#ifndef SSH_FEATURE_H
#define SSH_FEATURE_H

#include "Features/Feature-Base.h"

class SSH_Feature : public Feature_Base {
public:
    std::vector<std::string> getFeatures(json j) override;
private:
    static bool registered;
};

#endif
/* SSH_FEATURE_H */