#ifndef PROXIEDMAP_H
#define PROXIEDMAP_H
#include <map>
#include <string>
#include "Auditor.h"

class ProxiedMap final {
public:
    ProxiedMap(const std::map<std::string, int>& data_, Auditor& auditor): data_(data_), auditor_(auditor) {};
    ~ProxiedMap() = default;

    int read(const std::string& key) const;
    void edit(const std::string& key, int data);
    void add(const std::string& key, int data);
    void remove(const std::string& key);

private:
    std::map<std::string, int> data_;
    Auditor & auditor_;
};

#endif //PROXIEDMAP_H
