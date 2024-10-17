#include "transport_catalogue.h"

void TransportCatalogue::AddRoute(const std::string& route_number, const std::vector<std::string>& route_stops, bool circular_route) {
    all_buses_.push_back({ route_number, route_stops, circular_route });
    for (const auto& route_stop : route_stops) {
        for (auto& stop_ : all_stops_) {
            if (stop_.name == route_stop) {
                stop_.buses.insert(route_number);
            }
        }
    }
    busname_to_bus_[all_buses_.back().number] = &all_buses_.back();
}

void TransportCatalogue::AddStop(const std::string& stop_name, Coordinates& coordinates) {
    all_stops_.push_back({ stop_name, coordinates, {} });
    stopname_to_stop_[all_stops_.back().name] = &all_stops_.back();
}

// Use std::string_view to avoid string copying
const Bus* TransportCatalogue::FindRoute(std::string_view route_number) const {
    auto it = busname_to_bus_.find(route_number);
    return (it != busname_to_bus_.end()) ? it->second : nullptr;
}

// Use std::string_view to avoid string copying
const Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
    auto it = stopname_to_stop_.find(stop_name);
    return (it != stopname_to_stop_.end()) ? it->second : nullptr;
}

const RouteInfo TransportCatalogue::RouteInformation(std::string_view route_number) const {
    RouteInfo route_info{};
    const Bus* bus = FindRoute(route_number);

    if (!bus) {
        throw std::invalid_argument("bus not found");
    }

    if (bus->circular_route) {
        route_info.stops_count = bus->stops->size();
    } else {
        route_info.stops_count = bus->stops->size() * 2 - 1;
    }

    double route_length = 0.0;
    for (auto iter = bus->stops.value().begin(); iter + 1 != bus->stops.value().end(); ++iter) {
        auto from = stopname_to_stop_.find(*iter)->second->coordinates;
        auto to = stopname_to_stop_.find(*(iter + 1))->second->coordinates;

        if (bus->circular_route) {
            route_length += ComputeDistance(from, to);
        } else {
            route_length += ComputeDistance(from, to) * 2;
        }
    }
    route_info.unique_stops_count = UniqueStopsCount(route_number);
    route_info.route_length = route_length;

    return route_info;
}

size_t TransportCatalogue::UniqueStopsCount(std::string_view route_number) const {
    std::unordered_set<std::string_view> unique_stops;
    for (const auto& stop : busname_to_bus_.at(route_number)->stops.value()) {
        unique_stops.insert(stop);
    }
    return unique_stops.size();
}

// Return a constant reference to avoid copying the set of buses
const std::set<std::string>& TransportCatalogue::GetBusesOnStop(std::string_view stop_name) const {
    return stopname_to_stop_.at(stop_name)->buses;
}
