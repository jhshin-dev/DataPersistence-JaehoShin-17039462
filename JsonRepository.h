#pragma once
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "IRepository.h"
#include "json.hpp"

template<typename T>
class JsonRepository : public IRepository<T> {
protected:
    std::string    filePath_;
    std::vector<T> data_;
    int            nextId_ = 1;

    void load() {
        std::ifstream file(filePath_);
        if (!file.is_open()) {
            data_.clear();
            nextId_ = 1;
            return;
        }
        nlohmann::json j;
        try {
            file >> j;
        } catch (const nlohmann::json::parse_error& e) {
            throw std::runtime_error(
                std::string("JSON parse error in ") + filePath_ + ": " + e.what());
        }
        data_ = j.get<std::vector<T>>();
        for (const auto& item : data_)
            if (item.id >= nextId_) nextId_ = item.id + 1;
    }

    void save() const {
        std::ofstream file(filePath_);
        if (!file.is_open())
            throw std::runtime_error("Cannot open file for writing: " + filePath_);
        file << nlohmann::json(data_).dump(2);
    }

public:
    explicit JsonRepository(std::string filePath)
        : filePath_(std::move(filePath)) { load(); }

    T create(T entity) override {
        entity.id = nextId_++;
        data_.push_back(entity);
        save();
        return entity;
    }

    std::vector<T> findAll() const override { return data_; }

    std::optional<T> findById(int id) const override {
        for (const auto& item : data_)
            if (item.id == id) return item;
        return std::nullopt;
    }

    bool update(const T& entity) override {
        for (auto& item : data_) {
            if (item.id == entity.id) {
                item = entity;
                save();
                return true;
            }
        }
        return false;
    }

    bool remove(int id) override {
        auto it = std::find_if(data_.begin(), data_.end(),
            [id](const T& item) { return item.id == id; });
        if (it == data_.end()) return false;
        data_.erase(it);
        save();
        return true;
    }
};
