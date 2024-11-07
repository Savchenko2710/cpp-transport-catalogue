#include "transport_catalogue.h"
#include "geo.h"

#include <algorithm>
#include <numeric>

namespace transport {

void Database::RegisterStation(std::string_view name, const geo::Coordinates coords) {
    all_stations_.push_back({std::string(name), coords, {}});
    const Station* station_ptr = &all_stations_.back();
    station_lookup_[station_ptr->station_name] = station_ptr;
}

void Database::RegisterTransit(std::string_view number, const std::vector<const Station*>& stops, bool circular) {
    all_transits_.push_back({std::string(number), stops, circular});
    const Transit* transit_ptr = &all_transits_.back();
    transit_lookup_[transit_ptr->transit_number] = transit_ptr;

    for (const Station* station : stops) {
        const_cast<Station*>(station)->buses_serving.insert(transit_ptr->transit_number);
    }
}

const Transit* Database::RetrieveTransit(std::string_view number) const {
    auto it = transit_lookup_.find(number);
    return it != transit_lookup_.end() ? it->second : nullptr;
}

const Station* Database::RetrieveStation(std::string_view name) const {
    auto it = station_lookup_.find(name);
    return it != station_lookup_.end() ? it->second : nullptr;
}

std::optional<TransitStats> Database::ComputeStats(const std::string_view& number) const {
    const Transit* transit = RetrieveTransit(number);
    if (!transit) {
        return std::nullopt;
    }

    size_t total_stops = transit->route_stops.size();
    std::unordered_set<std::string_view> unique_stations;
    double path_length = 0.0;

    for (size_t i = 0; i < total_stops; ++i) {
        unique_stations.insert(transit->route_stops[i]->station_name);
        if (i + 1 < total_stops) {
            path_length += DistanceBetween(transit->route_stops[i], transit->route_stops[i + 1]);
        }
    }

    if (transit->circular_route && total_stops > 1) {
        path_length += DistanceBetween(transit->route_stops.back(), transit->route_stops.front());
    }

    double geographic_length = 0.0;
    for (size_t i = 0; i + 1 < total_stops; ++i) {
        geographic_length += geo::ComputeDistance(transit->route_stops[i]->location, transit->route_stops[i + 1]->location);
    }
    if (transit->circular_route && total_stops > 1) {
        geographic_length += geo::ComputeDistance(transit->route_stops.back()->location, transit->route_stops.front()->location);
    }

    double deviation_ratio = path_length / geographic_length;
    return TransitStats{total_stops, unique_stations.size(), path_length, deviation_ratio};
}

const std::set<std::string>& Database::BusesServingStation(std::string_view name) const {
    const Station* station = RetrieveStation(name);
    if (!station) {
        throw std::out_of_range("Station not found");
    }
    return station->buses_serving;
}

void Database::DefineDistance(const Station* from, const Station* to, int distance) {
    station_distances_[{from, to}] = distance;
    if (station_distances_.find({to, from}) == station_distances_.end()) {
        station_distances_[{to, from}] = distance;
    }
}

int Database::DistanceBetween(const Station* from, const Station* to) const {
    auto it = station_distances_.find({from, to});
    if (it != station_distances_.end()) {
        return it->second;
    } else {
        throw std::out_of_range("Distance between stations not defined");
    }
}

} 

