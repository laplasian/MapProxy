#ifndef AUDITOR_H
#define AUDITOR_H
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>

class Auditor {
public:
    virtual ~Auditor() = default;

    enum AccessType {
        Prohibited = 1 << 0, Readable = 1 << 1, Writeable = 1 << 2, Owned = 1 << 3, Addable = 1 << 4, Removable = 1 << 5,
        ReadWrite = Readable | Writeable,
        FullAccess = Readable | Writeable | Addable | Removable,
        OwnerReadonly = Readable | Owned,
        OwnerReadWrite = Readable | Writeable | Owned,
    };

    [[nodiscard]] const virtual std::pair < AccessType, int > check_read(const std::string& key) const = 0;
    [[nodiscard]] const virtual AccessType check_write(const std::string& key, int data) { return check_read(key).first;};
};

class FullAccessAuditor final : public Auditor {
public:
    FullAccessAuditor() = default;
    [[nodiscard]] const std::pair < AccessType, int > check_read(const std::string& key) const override {return {FullAccess, 0};};
};

class ReaderAuditor final : public Auditor {
public:
    [[nodiscard]] const std::pair < AccessType, int > check_read(const std::string& key) const override {return {Readable, 0};};
};

class OwnMapAuditor final : public Auditor { // Имеет права чтения и/или записи для spoofed, для остальных ключей только чтение
public:
    explicit OwnMapAuditor(const std::map<std::string, int> & spoofed = {}, const bool writeable_spoofed = true): spoofed(spoofed), writeable_spoofed(writeable_spoofed) {};
    [[nodiscard]] const std::pair < AccessType, int > check_read(const std::string& key) const override {
        const auto item = spoofed.find(key);
        if (item == spoofed.end()) return  {Readable, 0}; // Выдаёт только Readable, так как данным ключом key не владеет
        return  {writeable_spoofed ? OwnerReadWrite : OwnerReadonly, item->second};
    };
    [[nodiscard]] const AccessType check_write(const std::string& key, int data) override {
        if (spoofed.find(key) == spoofed.end()) return Readable; // Выдаёт только Readable, так как данным ключом key не владеет
        if (writeable_spoofed) spoofed[key] = data;
        return writeable_spoofed ? OwnerReadWrite : OwnerReadonly;
    };
private:
    std::map<std::string, int> spoofed;
    bool writeable_spoofed;
};

static bool find(const std::vector<std::string>& vec, const std::string& key) { return std::find(vec.begin(), vec.end(), key) != vec.end(); };

class ProxyAuditor final : public Auditor {
public:
    ProxyAuditor(const std::vector<std::string>& write_map, const std::vector<std::string>& prohibited_map ):
        write_map_(write_map), prohibited_map_(prohibited_map) {};
    [[nodiscard]] const std::pair < AccessType, int > check_read(const std::string& key) const override {
        if (find(prohibited_map_, key)) return {Prohibited, 0};
        if (find(write_map_, key)) return {ReadWrite, 0};
        return {Readable, 0};
    }
private:
    std::vector<std::string> write_map_;
    std::vector<std::string> prohibited_map_;
};

class SlaveAuditor final : public Auditor {
public:
    explicit SlaveAuditor(const std::vector<std::string>& read_access_map): access_map_(read_access_map){};
    [[nodiscard]] const std::pair < AccessType, int > check_read(const std::string& key) const override {
        if (find(access_map_, key)) return {Readable, 0};
        return {Prohibited, 0};
    }
private:
    std::vector<std::string> access_map_;
};

#endif //AUDITOR_H
