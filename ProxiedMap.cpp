#include "ProxiedMap.h"
#include <stdexcept>

static bool operator==(Auditor::AccessType container, Auditor::AccessType access) { // проверяет наличие бита access в container
    return (container & access) == access;
}

static bool operator!=(Auditor::AccessType container, Auditor::AccessType access) {
    return !(container == access);
}

using enum Auditor::AccessType;

int ProxiedMap::read(const std::string &key) const {
    const std::pair < Auditor::AccessType, int > info = auditor_.check_read(key);
    if (info.first != Readable) throw std::runtime_error("Prohibited");

    return info.first == Owned ? info.second : data_.at(key);
}

void ProxiedMap::edit(const std::string &key, int data) {
    Auditor::AccessType info = auditor_.check_write(key, data);

    if (info != Writeable) throw std::runtime_error("Prohibited");
    if (info == Owned) return; // так как check_write сделал это за нас

    if (data_.find(key) == data_.end()) return;
    data_[key]= data;
}

void ProxiedMap::add(const std::string &key, int data) {
    Auditor::AccessType info = auditor_.check_add(key, data);

    if (info!=Addable) throw std::runtime_error("Prohibited");
    if (info == Owned) return; // так как check_add сделал это за нас

    data_[key] = data;
}

void ProxiedMap::remove(const std::string &key) {
    Auditor::AccessType info = auditor_.check_remove(key);

    if (info!=Auditor::Removable) throw std::runtime_error("Prohibited");
    if (info == Owned) return;  // так как check_remove сделал это за нас

    data_.erase(key);
}
