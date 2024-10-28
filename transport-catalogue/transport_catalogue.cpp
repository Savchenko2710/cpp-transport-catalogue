#include "transport_catalogue.h" 

#include <iostream> 
#include <deque> 
#include <string> 
#include <unordered_map> 
#include <vector> 
#include <stdexcept> 
#include <optional> 
#include <unordered_set> 
#include <set> 

namespace transport {

struct Station {
    std::string station_name;
    geo::Coordinates location;
    std::set<std::string> buses_serving;
};

struct Transit {
    std::string transit_number;
    std::vector<const Station*> route_stops;
    bool circular_route;
};

struct TransitStats {
    size_t total_stops;
    size_t distinct_stations;
    double path_length;
    double deviation_ratio;
};

class Database {
public:
    struct DistanceHasher {
        size_t operator()(const std::pair<const Station*, const Station*>& endpoints) const {
            size_t start_hash = std::hash<const void*>{}(endpoints.first);
            size_t end_hash = std::hash<const void*>{}(endpoints.second);
            return start_hash + end_hash * 41;
        }
    };

    void RegisterStation(std::string_view name, const geo::Coordinates coords);
    void RegisterTransit(std::string_view number, const std::vector<const Station*>& stops, bool circular);
    
    const Transit* RetrieveTransit(std::string_view number) const;
    const Station* RetrieveStation(std::string_view name) const;
    std::optional<TransitStats> ComputeStats(const std::string_view& number) const;
    
    const std::set<std::string>& BusesServingStation(std::string_view name) const;

    void DefineDistance(const Station* from, const Station* to, int distance);
    int DistanceBetween(const Station* from, const Station* to) const;

private:
    std::deque<Transit> all_transits_;
    std::deque<Station> all_stations_;
    std::unordered_map<std::string_view, const Transit*> transit_lookup_;
    std::unordered_map<std::string_view, const Station*> station_lookup_;
    std::unordered_map<std::pair<const Station*, const Station*>, int, DistanceHasher> station_distances_;
};

} 
