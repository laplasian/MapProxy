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
        Readonly, ReadWrite, Owner, Prohibited
    };

    [[nodiscard]] const virtual std::pair < AccessType, int > read(const std::string& key) const = 0;
};

class FullAccess final : public Auditor {
public:
    FullAccess() = default;
    [[nodiscard]] const std::pair < AccessType, int > read(const std::string& key) const override {return access_;};
private:
    static constexpr std::pair < AccessType, int > access_ {ReadWrite, 0};
};

class Reader final : public Auditor {
public:
    [[nodiscard]] const std::pair < AccessType, int > read(const std::string& key) const override {return access_;};
private:
    static constexpr std::pair < AccessType, int > access_ {Readonly, 0};
};

class OwnMap final : public Auditor {
public:
    explicit OwnMap(const std::map<std::string, int> & spoofed = {}): spoofed(spoofed) {};
    [[nodiscard]] const std::pair < AccessType, int > read(const std::string& key) const override {
        const auto item = spoofed.find(key);
        if (item == spoofed.end())
        {
            return access_;
        }
        return  {Owner, item->second};
    };
private:
    std::map<std::string, int> spoofed;
    static constexpr std::pair < AccessType, int > access_ {Readonly, 0};
};


class Proxy final : public Auditor {
public:
    Proxy(const std::vector<std::string>& write_map, const std::vector<std::string>& prohibited_map ):
        write_map_(write_map), prohibited_map_(prohibited_map) {};
    [[nodiscard]] const std::pair < AccessType, int > read(const std::string& key) const override {
        for (auto &key_: prohibited_map_) {
            if (key == key_) return access_prohibited;
        }
        for (auto &key_: write_map_) {
            if (key == key_) return access_readwrite;
        }
        return access_readonly;
    }
private:
    std::vector<std::string> write_map_;
    std::vector<std::string> prohibited_map_;
    static constexpr std::pair < AccessType, int > access_readonly {Readonly, 0};
    static constexpr std::pair < AccessType, int > access_readwrite {ReadWrite, 0};
    static constexpr std::pair < AccessType, int > access_prohibited {Prohibited, 0};
};

class Slave final : public Auditor {
public:
    explicit Slave(const std::vector<std::string>& read_access_map): access_map_(read_access_map){};
    [[nodiscard]] const std::pair < AccessType, int > read(const std::string& key) const override {
        for (auto &key_: access_map_) {
            if (key == key_) return access_readonly;
        }
        return access_prohibited;
    }
private:
    std::vector<std::string> access_map_;
    static constexpr std::pair < AccessType, int > access_readonly {Readonly, 0};
    static constexpr std::pair < AccessType, int > access_prohibited {Prohibited, 0};
};

#endif //AUDITOR_H
