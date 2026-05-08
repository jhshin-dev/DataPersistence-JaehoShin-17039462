#pragma once
#include <functional>
#include "JsonRepository.h"
#include "Order.h"
#include "Sample.h"

class SampleRepository : public JsonRepository<Sample> {
    std::function<std::vector<Order>()> getOrders_;

public:
    explicit SampleRepository(std::string filePath,
                              std::function<std::vector<Order>()> getOrders = nullptr)
        : JsonRepository<Sample>(std::move(filePath))
        , getOrders_(std::move(getOrders)) {}

    bool remove(int id) override {
        if (getOrders_) {
            for (const auto& order : getOrders_())
                if (order.sampleId == id) return false;
        }
        return JsonRepository<Sample>::remove(id);
    }
};
