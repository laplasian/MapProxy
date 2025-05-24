#ifndef AUDITOR_H
#define AUDITOR_H
#include <map>
#include <string>
#include <utility>
#include <vector>

class Auditor {
public:
    virtual ~Auditor() = default;

    enum AccessType {
        Prohibited = 0, Readonly = 0x1, Write = 0x2, Owner = 0x4, Addable = 0x8, Removable = 0x10,
        ReadWrite = Readonly | Write,
        FullAccess = Readonly | Write | Addable | Removable,
        OwnerReadonly = Readonly | Readonly,
    };
    [[nodiscard]] virtual AccessType check(const std::string& key) const = 0;
};

class Spoofer : public Auditor {
protected:
    friend class ProxiedMap;
    mutable std::map<std::string, int> spoofed;
};

class FullAccess final : public Auditor {
public:
    FullAccess() = default;
    [[nodiscard]] AccessType check(const std::string& key) const override {return AccessType::FullAccess;};
};

class Reader final : public Auditor {
public:
    [[nodiscard]] AccessType check(const std::string& key) const override {return AccessType::Readonly;};
};

class OwnMap final : public Spoofer {
public:
    explicit OwnMap(const std::map<std::string, int> & spoofed_ = {}) { spoofed = spoofed_ ;};
    [[nodiscard]] AccessType check(const std::string& key) const override {
        return AccessType::OwnerReadonly;
    };
};


class Proxy final : public Auditor {
public:
    Proxy(const std::vector<std::string>& write_map, const std::vector<std::string>& prohibited_map ):
        write_map_(write_map), prohibited_map_(prohibited_map) {};
    [[nodiscard]] AccessType check(const std::string& key) const override {
        for (auto &key_: prohibited_map_) {
            if (key == key_) return Prohibited;
        }
        for (auto &key_: write_map_) {
            if (key == key_) return ReadWrite;
        }
        return Readonly;
    }
private:
    std::vector<std::string> write_map_;
    std::vector<std::string> prohibited_map_;
};

class Slave final : public Auditor {
public:
    explicit Slave(const std::vector<std::string>& read_access_map): access_map_(read_access_map){};
    [[nodiscard]] AccessType check(const std::string& key) const override {
        for (auto &key_: access_map_) {
            if (key == key_) return Readonly;
        }
        return Prohibited;
    }
private:
    std::vector<std::string> access_map_;
};

#endif //AUDITOR_H
