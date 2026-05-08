#pragma once
#include <ctime>
#include <format>
#include "JsonRepository.h"
#include "Order.h"

class OrderRepository : public JsonRepository<Order> {
    static std::string nowIso8601() {
        std::time_t t = std::time(nullptr);
        std::tm     tm{};
        gmtime_s(&tm, &t);
        return std::format("{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}Z",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);
    }

public:
    explicit OrderRepository(std::string filePath)
        : JsonRepository<Order>(std::move(filePath)) {}

    Order create(Order entity) override {
        entity.status    = OrderStatus::RESERVED;
        entity.createdAt = nowIso8601();
        entity.updatedAt = entity.createdAt;
        return JsonRepository<Order>::create(entity);
    }

    bool updateStatus(int id, OrderStatus newStatus) {
        auto opt = findById(id);
        if (!opt) return false;
        if (!isValidTransition(opt->status, newStatus)) return false;
        auto order      = *opt;
        order.status    = newStatus;
        order.updatedAt = nowIso8601();
        return update(order);
    }

    bool remove(int id) override {
        auto opt = findById(id);
        if (!opt) return false;
        if (opt->status == OrderStatus::RELEASED) return false;
        return JsonRepository<Order>::remove(id);
    }
};
