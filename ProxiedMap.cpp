#include "ProxiedMap.h"

#include <stdexcept>


int ProxiedMap::read(const std::string &key) const {
    const Auditor::AccessType info = auditor_.check(key);
    if (info==Auditor::Prohibited)
    {
        throw std::runtime_error("Prohibited");
    }
    if (info == Auditor::AccessType::Owner)
    {
        try {
            const Spoofer& spoofer = dynamic_cast<const Spoofer&>(auditor_);
            if (spoofer.spoofed.find(key) != spoofer.spoofed.end()) return spoofer.spoofed.at(key);
        } catch (const std::bad_cast&) {
            throw std::runtime_error("Auditor return Owner flag, but he isn`t Spoofer");
        }
    }
    return data_.at(key);
}

void ProxiedMap::edit(const std::string &key, int data) {
    const Auditor::AccessType info = auditor_.check(key);
    if (info==Auditor::Prohibited)
    {
        throw std::runtime_error("Prohibited");
    }
    if (info == Auditor::AccessType::Owner | Auditor::AccessType::Write)
    {
        try {
            const auto& spoofer = dynamic_cast<const Spoofer&>(auditor_);
            auto it = spoofer.spoofed.find(key);
            if (it != spoofer.spoofed.end()) {
                it->second = data;
            };
        } catch (const std::bad_cast&) {
            throw std::runtime_error("Auditor return Owner flag, but he isn`t Spoofer");
        }
    }
    auto it = data_.find(key);
    if (it != data_.end()) {
        data_[key] = data;
    };
}

void ProxiedMap::add(const std::string &key, int data) {
    const Auditor::AccessType info = auditor_.check(key);
    if (info==Auditor::Prohibited)
    {
        throw std::runtime_error("Prohibited");
    }
    if (info == Auditor::AccessType::Owner) {

    }
}

void ProxiedMap::remove(const std::string &key) {
}
