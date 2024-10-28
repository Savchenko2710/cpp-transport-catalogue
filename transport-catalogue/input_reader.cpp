#include "input_reader.h"

namespace transport_catalogue {

void PopulateCatalogue(std::istream& input_stream, Catalogue& transport_catalog) {
    std::vector<std::string> bus_requests;
    std::vector<std::string> stop_requests;
    std::vector<std::string> stop_distance_requests;
    size_t total_requests;

    input_stream >> total_requests;
    input_stream.ignore();  // Ignore remaining newline

    for (size_t i = 0; i < total_requests; ++i) {
        std::string command_type, raw_line;
        input_stream >> command_type;
        std::getline(input_stream, raw_line);

        if (command_type == "Stop") {
            stop_requests.emplace_back(raw_line);
        } else if (command_type == "Bus") {
            bus_requests.emplace_back(raw_line);
        }
    }
    
    stop_distance_requests = stop_requests;
    
    for (const auto& stop_data : stop_requests) {
        auto [stop_name, coords] = ParseStopData(stop_data);
        transport_catalog.AddStop(stop_name, coords);
    }
    
    for (const auto& distance_data : stop_distance_requests) {
        ParseAndSetDistances(distance_data, transport_catalog);
    }
    
    for (const auto& bus_data : bus_requests) {
        auto [route_number, stop_sequence, is_circular] = ParseBusRoute(bus_data, transport_catalog);
        transport_catalog.AddRoute(route_number, stop_sequence, is_circular);
    }
}

std::pair<std::string, geo::Coordinates> ParseStopData(const std::string& stop_input) {
    std::string stop_name = stop_input.substr(0, stop_input.find(':'));
    std::string coord_part = stop_input.substr(stop_input.find(':') + 1);
    auto coords = geo::ParseCoordinates(coord_part);
    return {stop_name, coords};
}

void ParseAndSetDistances(const std::string& stop_data, Catalogue& catalogue) {
    std::string stop_name = stop_data.substr(0, stop_data.find(':'));
    std::string distance_info = stop_data.substr(stop_data.find(':') + 1);
    
    std::istringstream dist_stream(distance_info);
    std::string segment;
    while (std::getline(dist_stream, segment, ',')) {
        auto [distance, destination_stop] = SplitDistanceData(segment);
        catalogue.SetDistance(stop_name, destination_stop, distance);
    }
}

std::pair<int, std::string> SplitDistanceData(const std::string& segment) {
    int distance = std::stoi(segment.substr(0, segment.find("m to ")));
    std::string destination_stop = segment.substr(segment.find("m to ") + 5);
    return {distance, destination_stop};
}

std::tuple<std::string, std::vector<const Stop*>, bool> ParseBusRoute(const std::string& bus_input, Catalogue& catalogue) {
    std::string route_id = bus_input.substr(0, bus_input.find(':'));
    std::string stops_data = bus_input.substr(bus_input.find(':') + 1);
    bool is_circular = stops_data.find('-') == std::string::npos;

    std::vector<const Stop*> stops;
    std::istringstream stops_stream(stops_data);
    std::string stop_name;
    char delimiter = is_circular ? '>' : '-';

    while (std::getline(stops_stream, stop_name, delimiter)) {
        stops.push_back(catalogue.FindStop(stop_name));
    }

    return {route_id, stops, is_circular};
}

}  
