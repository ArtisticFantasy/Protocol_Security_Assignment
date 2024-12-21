#ifndef FEATURE_BASE_H
#define FEATURE_BASE_H

#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define REGISTER_DATAFEATURE(feature) \
    bool feature::registered = []() { \
        Feature_List::getInstance().registerDataFeature(new feature()); \
        return true; \
    }();

#define REGISTER_SPATIALFEATURE(feature) \
    bool feature::registered = []() { \
        Feature_List::getInstance().registerSpatialFeature(new feature()); \
        return true; \
    }();

class Feature_Base {
public:
    virtual std::vector<std::string> getFeatures(json j) = 0;
};

class Feature_List {
public:
    static Feature_List& getInstance() {
        static Feature_List instance;
        return instance;
    }

    static void registerDataFeature(Feature_Base* feature) {
        DataFeatures.push_back(feature);
    }

    static void registerSpatialFeature(Feature_Base* feature) {
        SpatialFeatures.push_back(feature);
    }

    static std::vector<std::string> getAllFeatures(json j) {
        std::vector<std::string> res;
        // Data Features
        for (auto feature : DataFeatures) {
            std::vector<std::string> temp = feature->getFeatures(j);
            res.insert(res.end(), temp.begin(), temp.end());
        }
        // Spatial Features
        for (auto feature : SpatialFeatures) {
            std::vector<std::string> temp = feature->getFeatures(j);
            res.insert(res.end(), temp.begin(), temp.end());
        }
        // Crossjoin Features
        for (auto feature1 : DataFeatures) {
            for (auto feature2 : SpatialFeatures) {
                std::vector<std::string> temp1 = feature1->getFeatures(j);
                std::vector<std::string> temp2 = feature2->getFeatures(j);
                for (auto t1 : temp1) {
                    for (auto t2 : temp2) {
                        res.push_back(t1 + " " + t2);
                    }
                }
            }
        }
        return res;
    }
private:
    static std::vector<Feature_Base*> DataFeatures;
    static std::vector<Feature_Base*> SpatialFeatures;
};
#endif