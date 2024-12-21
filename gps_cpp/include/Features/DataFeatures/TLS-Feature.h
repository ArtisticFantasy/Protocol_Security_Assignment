#ifndef TLS_FEATURE_H
#define TLS_FEATURE_H

#include "Features/Feature-Base.h"

class TLS_Feature : public Feature_Base {
public:
    std::vector<std::string> getFeatures(json j) override;
private:
    std::string extract_TLS_cert(std::string data);
    std::vector<std::string> extract_TLS_oid(std::string data);
    static bool registered;
};

#endif
/* TLS_FEATURE_H */