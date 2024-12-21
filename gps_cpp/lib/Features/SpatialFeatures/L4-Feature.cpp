#include "Features/SpatialFeatures/L4-Feature.h"

REGISTER_SPATIALFEATURE(L4_Feature)

std::vector<std::string> L4_Feature::getFeatures(json j) {
    if (!j.contains("saddr") || !j.contains("sport")) {
        return {};
    }
    
    return {"L4"};
}