#pragma once
#include <stdexcept>
#include <string>
#include "json.hpp"

enum class OrderStatus { RESERVED, REJECTED, PRODUCING, CONFIRMED, RELEASED };

inline std::string orderStatusToString(OrderStatus s) {
    switch (s) {
    case OrderStatus::RESERVED:  return "RESERVED";
    case OrderStatus::REJECTED:  return "REJECTED";
    case OrderStatus::PRODUCING: return "PRODUCING";
    case OrderStatus::CONFIRMED: return "CONFIRMED";
    case OrderStatus::RELEASED:  return "RELEASED";
    }
    return "UNKNOWN";
}

inline OrderStatus orderStatusFromString(const std::string& s) {
    if (s == "RESERVED")  return OrderStatus::RESERVED;
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    if (s == "RELEASED")  return OrderStatus::RELEASED;
    throw std::invalid_argument("Unknown OrderStatus: " + s);
}

// PRD 상태 전이 규칙
inline bool isValidTransition(OrderStatus from, OrderStatus to) {
    switch (from) {
    case OrderStatus::RESERVED:
        return to == OrderStatus::CONFIRMED
            || to == OrderStatus::PRODUCING
            || to == OrderStatus::REJECTED;
    case OrderStatus::PRODUCING:
        return to == OrderStatus::CONFIRMED;
    case OrderStatus::CONFIRMED:
        return to == OrderStatus::RELEASED;
    default:
        return false;
    }
}

struct Order {
    int         id        = 0;
    int         sampleId  = 0;
    int         quantity  = 0;
    OrderStatus status    = OrderStatus::RESERVED;
    std::string createdAt;
    std::string updatedAt;
};

inline void to_json(nlohmann::json& j, const Order& o) {
    j = {
        {"id",        o.id},
        {"sampleId",  o.sampleId},
        {"quantity",  o.quantity},
        {"status",    orderStatusToString(o.status)},
        {"createdAt", o.createdAt},
        {"updatedAt", o.updatedAt}
    };
}

inline void from_json(const nlohmann::json& j, Order& o) {
    j.at("id").get_to(o.id);
    j.at("sampleId").get_to(o.sampleId);
    j.at("quantity").get_to(o.quantity);
    o.status = orderStatusFromString(j.at("status").get<std::string>());
    j.at("createdAt").get_to(o.createdAt);
    j.at("updatedAt").get_to(o.updatedAt);
}
