#ifndef HTTP_FEATURE_H
#define HTTP_FEATURE_H

#include "Features/Feature-Base.h"

class HTTP_Feature : public Feature_Base {
public:
    std::vector<std::string> getFeatures(json j) override;
private:
    static bool registered;
};

#endif
/* HTTP_FEATURE_H */