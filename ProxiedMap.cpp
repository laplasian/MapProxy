#include "ProxiedMap.h"
#include <stdexcept>

bool operator==(Auditor::AccessType container, Auditor::AccessType access) {
    return (container & access) == access;
}

bool operator!=(Auditor::AccessType container, Auditor::AccessType access) {
    return !(container == access);
}

int ProxiedMap::read(const std::string &key) const {
    const std::pair < Auditor::AccessType, int > info = auditor_.check_read(key);
    if (info.first==Auditor::Prohibited) throw std::runtime_error("Prohibited");

    if (info.first == Auditor::AccessType::Owner) return info.second;

    return data_.at(key);
}

void ProxiedMap::edit(const std::string &key, int data) {
    Auditor::AccessType info = auditor_.check_write(key, data);

    if (info==Auditor::Prohibited || info!=Auditor::Write) throw std::runtime_error("Prohibited");
    if (info == (Auditor::AccessType::Owner | Auditor::AccessType::Write)) return;

    auto item = data_.find(key);
    if (item == data_.end()) return;
    data_[key]= data;
}

void ProxiedMap::add(const std::string &key, int data) {
    Auditor::AccessType info = auditor_.check_read(key).first;

    if (info==Auditor::Prohibited || info!=Auditor::Addable) throw std::runtime_error("Prohibited");

    data_[key] = data;
}

void ProxiedMap::remove(const std::string &key) {
    Auditor::AccessType info = auditor_.check_read(key).first;

    if (info==Auditor::Prohibited || info!=Auditor::Removable) throw std::runtime_error("Prohibited");

    data_.erase(key);
}
