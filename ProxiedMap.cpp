#include "ProxiedMap.h"

#include <stdexcept>


int ProxiedMap::read(const std::string &key) {
    const std::pair < Auditor::AccessType, int > info = auditor_.read(key);
    if (info.first == Auditor::AccessType::Owner)
    {
        return info.second;
    }
    if (info.first==Auditor::Prohibited)
    {
        throw std::runtime_error("Prohibited");
    }
    const auto item = data_.find(key);
    if (item == data_.end())
    {
        throw std::runtime_error("No such key");
    }
    return item->second;
}

void ProxiedMap::edit(const std::string &key, int data) {
    std::pair < Auditor::AccessType, int > info = auditor_.read(key);
    if (info.first == Auditor::AccessType::Owner)
    {
        info.second = data;
        return;
    }
    if (info.first==Auditor::Prohibited || info.first==Auditor::Readonly) {
        throw std::runtime_error("Prohibited");
    }
    auto item = data_.find(key);
    if (item == data_.end())
    {
        throw std::runtime_error("No such key");
    }
    item->second = data;
}
