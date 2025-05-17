#ifndef PROXIEDMAP_H
#define PROXIEDMAP_H
#include <map>
#include <string>
#include "Auditor.h"

class ProxiedMap final {
public:
    ProxiedMap(const std::map<std::string, int>& data_, const Auditor& auditor): data_(data_), auditor_(auditor) {};
    ~ProxiedMap() = default;

    int read(const std::string& key);
    void edit(const std::string& key, int data);

private:
    std::map<std::string, int> data_;
    const Auditor & auditor_;
};

#endif //PROXIEDMAP_H
