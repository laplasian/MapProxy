#include <gtest/gtest.h>
#include "Auditor.h"
#include "ProxiedMap.h"
#include <map>

// TEST AUDITORS (ctor, check_read, check_write)

TEST(FullAccessAuditor, BasicTest) {
    FullAccessAuditor auditor;
    EXPECT_EQ(auditor.check_read("any_key"), (std::pair{Auditor::AccessType::FullAccess, 0}) );
    EXPECT_EQ(auditor.check_write("any_key", 10), Auditor::AccessType::FullAccess);
}

TEST(ReaderAuditor, BasicTest) {
    ReaderAuditor auditor;
    EXPECT_EQ(auditor.check_read("any_key"), (std::pair{Auditor::AccessType::Readable, 0}));
    EXPECT_EQ(auditor.check_write("any_key", 10), Auditor::AccessType::Readable);
}

TEST(OwnMapAuditor, BasicTest) {
    OwnMapAuditor auditor1({{"key1", 10}, {"key2", 20}}, true);
    OwnMapAuditor auditor2({{"key1", 10}, {"key2", 20}}, false);

    EXPECT_EQ(auditor1.check_read("key1"), (std::pair{Auditor::AccessType::OwnerReadWrite, 10}));
    EXPECT_EQ(auditor1.check_read("key2"), (std::pair{Auditor::AccessType::OwnerReadWrite, 20}));
    EXPECT_EQ(auditor1.check_read("key3"), (std::pair{Auditor::AccessType::Readable, 0}));
    EXPECT_EQ(auditor1.check_write("key1", 0), Auditor::AccessType::OwnerReadWrite);
    EXPECT_EQ(auditor1.check_write("key3", 0), Auditor::AccessType::Readable);

    EXPECT_EQ(auditor2.check_read("key1"), (std::pair{Auditor::AccessType::OwnerReadonly, 10}));
    EXPECT_EQ(auditor2.check_read("key2"), (std::pair{Auditor::AccessType::OwnerReadonly, 20}));
    EXPECT_EQ(auditor2.check_read("key3"), (std::pair{Auditor::AccessType::Readable, 0}));
    EXPECT_EQ(auditor2.check_write("key1", 0), Auditor::AccessType::OwnerReadonly);
    EXPECT_EQ(auditor2.check_write("key3", 0), Auditor::AccessType::Readable);
}

TEST(ProxyAuditor, BasicTest) {
    ProxyAuditor auditor({"key_writable"}, {"key_prohibited"});

    EXPECT_EQ(auditor.check_read("key_writable"), (std::pair{Auditor::AccessType::ReadWrite, 0}));
    EXPECT_EQ(auditor.check_read("key_prohibited"), (std::pair{Auditor::AccessType::Prohibited, 0}));
    EXPECT_EQ(auditor.check_read("key_other"), (std::pair{Auditor::AccessType::Readable, 0}));

    EXPECT_EQ(auditor.check_write("key_writable", 10), Auditor::AccessType::ReadWrite);
    EXPECT_EQ(auditor.check_write("key_prohibited", 10), Auditor::AccessType::Prohibited);
    EXPECT_EQ(auditor.check_write("key_other", 10), Auditor::AccessType::Readable);
}

TEST(SlaveAuditor, BasicTest) {
    SlaveAuditor auditor({"key_readable"});

    EXPECT_EQ(auditor.check_read("key_readable"), (std::pair{Auditor::AccessType::Readable, 0}));
    EXPECT_EQ(auditor.check_read("key_other"), (std::pair{Auditor::AccessType::Prohibited, 0}));
    EXPECT_EQ(auditor.check_write("any_key", 10), Auditor::AccessType::Prohibited);
}

// TEST PROXIED MAP (ctor, read, edit, add, remove and throws)

TEST(ProxiedMap, FullAccessAuditor) {
    std::map<std::string, int> map;
    map.insert({ "1", 10 });
    map.insert({ "2", 200 });
    map.insert({ "3", 3000 });

    FullAccessAuditor god;
    ProxiedMap proxied_map(map, god);

    EXPECT_EQ(proxied_map.read("1"), 10);
    EXPECT_EQ(proxied_map.read("2"), 200);
    EXPECT_EQ(proxied_map.read("3"), 3000);

    proxied_map.edit("1", -10);
    proxied_map.edit("2", -200);
    proxied_map.edit("3", -3000);

    EXPECT_EQ(proxied_map.read("1"), -10);
    EXPECT_EQ(proxied_map.read("2"), -200);
    EXPECT_EQ(proxied_map.read("3"), -3000);

    proxied_map.remove("1");
    EXPECT_THROW(proxied_map.read("1"), std::out_of_range);

    proxied_map.add("1", 100);
    EXPECT_EQ(proxied_map.read("1"), 100);
}

TEST(ProxiedMap, ReaderAuditor) {
    std::map<std::string, int> map;
    map.insert({ "1", 10 });
    map.insert({ "2", 200 });
    map.insert({ "3", 3000 });

    ReaderAuditor reader;
    ProxiedMap proxied_map(map, reader);

    EXPECT_EQ(proxied_map.read("1"), 10);
    EXPECT_EQ(proxied_map.read("2"), 200);
    EXPECT_EQ(proxied_map.read("3"), 3000);
    EXPECT_THROW(proxied_map.read("4"), std::out_of_range);

    EXPECT_THROW(proxied_map.edit("1", -10), std::runtime_error);
    EXPECT_THROW(proxied_map.add("1", -10), std::runtime_error);
    EXPECT_THROW(proxied_map.remove("1"), std::runtime_error);
}

TEST(ProxiedMap, OwnMapAuditor) {
    std::map<std::string, int> map;
    map.insert({ "1", 10 });
    map.insert({ "2", 200 });
    map.insert({ "3", 3000 });

    OwnMapAuditor own_map_auditor_readonly({{"1", -10}}, false);
    OwnMapAuditor own_map_auditor_readwrite({{"2", -200}}, true);

    ProxiedMap proxied_map_readonly(map, own_map_auditor_readonly);
    ProxiedMap proxied_map_readwrite(map, own_map_auditor_readwrite);

    EXPECT_EQ(proxied_map_readonly.read("1"), -10);
    EXPECT_EQ(proxied_map_readonly.read("2"), 200);
    EXPECT_EQ(proxied_map_readonly.read("3"), 3000);
    EXPECT_THROW(proxied_map_readonly.read("4"), std::out_of_range);

    EXPECT_THROW(proxied_map_readonly.edit("1", 0), std::runtime_error);
    EXPECT_THROW(proxied_map_readonly.edit("2", 0), std::runtime_error);
    EXPECT_THROW(proxied_map_readonly.add("1", 0), std::runtime_error);
    EXPECT_THROW(proxied_map_readonly.remove("1"), std::runtime_error);
    EXPECT_THROW(proxied_map_readonly.remove("2"), std::runtime_error);

    EXPECT_EQ(proxied_map_readwrite.read("1"), 10);
    EXPECT_EQ(proxied_map_readwrite.read("2"), -200);
    EXPECT_EQ(proxied_map_readwrite.read("3"), 3000);
    EXPECT_THROW(proxied_map_readwrite.read("4"), std::out_of_range);

    proxied_map_readwrite.edit("2", 150);
    EXPECT_EQ(proxied_map_readwrite.read("2"), 150);

    EXPECT_THROW(proxied_map_readwrite.edit("1", 0), std::runtime_error);
    EXPECT_THROW(proxied_map_readwrite.add("1", 0), std::runtime_error);
    EXPECT_THROW(proxied_map_readwrite.remove("1"), std::runtime_error);
    EXPECT_THROW(proxied_map_readwrite.remove("2"), std::runtime_error);
}

TEST(ProxiedMap, ProxyAuditor) {
    std::map<std::string, int> map;
    map.insert({ "key_writable", 10 });
    map.insert({ "key_prohibited", 200 });
    map.insert({ "other", 3000 });

    ProxyAuditor auditor({"key_writable"}, {"key_prohibited"});
    ProxiedMap proxied_map(map, auditor);

    EXPECT_EQ(proxied_map.read("key_writable"), 10);
    EXPECT_EQ(proxied_map.read("other"), 3000);
    EXPECT_THROW(proxied_map.read("key_prohibited"), std::runtime_error);

    proxied_map.edit("key_writable", -10);
    EXPECT_EQ(proxied_map.read("key_writable"), -10);

    EXPECT_THROW(proxied_map.edit("other", 0), std::runtime_error);
    EXPECT_THROW(proxied_map.add("any_key", 0), std::runtime_error);
    EXPECT_THROW(proxied_map.remove("any_key"), std::runtime_error);
    EXPECT_THROW(proxied_map.remove("any_key"), std::runtime_error);
}

TEST(ProxiedMap, SlaveAuditor) {
    std::map<std::string, int> map;
    map.insert({ "key_asses", 10 });
    map.insert({ "other", 3000 });

    SlaveAuditor auditor({"key_asses"});
    ProxiedMap proxied_map(map, auditor);

    EXPECT_EQ(proxied_map.read("key_asses"), 10);
    EXPECT_THROW(proxied_map.read("other"), std::runtime_error);

    EXPECT_THROW(proxied_map.edit("key_asses", 0), std::runtime_error);
    EXPECT_THROW(proxied_map.edit("other", 0), std::runtime_error);
    EXPECT_THROW(proxied_map.add("any_key", 0), std::runtime_error);
    EXPECT_THROW(proxied_map.remove("any_key"), std::runtime_error);
    EXPECT_THROW(proxied_map.remove("any_key"), std::runtime_error);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
