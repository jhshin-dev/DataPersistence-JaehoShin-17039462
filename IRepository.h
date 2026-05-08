#pragma once
#include <optional>
#include <vector>

template<typename T>
class IRepository {
public:
    virtual ~IRepository() = default;
    virtual T                    create(T entity)            = 0;
    virtual std::vector<T>       findAll()          const    = 0;
    virtual std::optional<T>     findById(int id)   const    = 0;
    virtual bool                 update(const T& entity)     = 0;
    virtual bool                 remove(int id)              = 0;
};
