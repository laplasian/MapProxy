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
        Prohibited = 1 << 0, Readonly = 1 << 1, Write = 1 << 2, Owner = 1 << 3, Addable = 1 << 4, Removable = 1 << 5,
        ReadWrite = Readonly | Write,
        FullAccess = Readonly | Write | Addable | Removable,
        OwnerReadonly = Readonly | Readonly,
        OwnerWrite = Write | Owner,
    };

    [[nodiscard]] const virtual std::pair < AccessType, int > check_read(const std::string& key) const = 0;
    [[nodiscard]] const virtual AccessType check_write(const std::string& key, int data) const { return check_read(key).first;} ;
};

class FullAccess final : public Auditor {
public:
    FullAccess() = default;
    [[nodiscard]] const std::pair < AccessType, int > check_read(const std::string& key) const override {return {AccessType::FullAccess, 0};};
};

class Reader final : public Auditor {
public:
    [[nodiscard]] const std::pair < AccessType, int > check_read(const std::string& key) const override {return access_;};
private:
    static constexpr std::pair < AccessType, int > access_ {Readonly, 0};
};

class OwnMap final : public Auditor {
public:
    explicit OwnMap(const std::map<std::string, int> & spoofed = {}): spoofed(spoofed) {};
    [[nodiscard]] const std::pair < AccessType, int > check_read(const std::string& key) const override {
        const auto item = spoofed.find(key);
        if (item == spoofed.end()) return  {Readonly, 0};
        return  {Owner, item->second};
    };
    [[nodiscard]] const AccessType check_write(const std::string& key, int data) const override {
        const auto item = spoofed.find(key);
        if (item == spoofed.end()) return Readonly;
        spoofed[key] = data;
        return Write;
    };
private:
    mutable std::map<std::string, int> spoofed;
};


class Proxy final : public Auditor {
public:
    Proxy(const std::vector<std::string>& write_map, const std::vector<std::string>& prohibited_map ):
        write_map_(write_map), prohibited_map_(prohibited_map) {};
    [[nodiscard]] const std::pair < AccessType, int > check_read(const std::string& key) const override {
        for (auto &key_: prohibited_map_) { if (key == key_) return {Prohibited, 0}; }
        for (auto &key_: write_map_) { if (key == key_) return {ReadWrite, 0}; }
        return {Readonly, 0};
    }

private:
    std::vector<std::string> write_map_;
    std::vector<std::string> prohibited_map_;
};

class Slave final : public Auditor {
public:
    explicit Slave(const std::vector<std::string>& read_access_map): access_map_(read_access_map){};
    [[nodiscard]] const std::pair < AccessType, int > check_read(const std::string& key) const override {
        for (auto &key_: access_map_) { if (key == key_) return {Readonly, 0}; }
        return {Prohibited, 0};
    }
private:
    std::vector<std::string> access_map_;
};

#endif //AUDITOR_H
