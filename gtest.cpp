#include <gtest/gtest.h>
#include "Auditor.h"
#include "ProxiedMap.h"
#include <map>

TEST(AuditorTest, BasicTest) {
    OwnMapAuditor g1({{"key", 1}});
    FullAccessAuditor g2;
    ReaderAuditor g3;
    SlaveAuditor g4({"key"});
    ProxyAuditor g5({"write"}, {"prohibited"});

    EXPECT_EQ(g1.check_read(""), ( std::pair{Auditor::Readonly, 0}) );
    EXPECT_EQ(g1.check_read("key"), (std::pair{Auditor::OwnerReadWrite, 1}));

    EXPECT_EQ(g2.check_read(""), ( std::pair{Auditor::FullAccess, 0}) );

    EXPECT_EQ(g3.check_read(""), ( std::pair{Auditor::Readonly, 0}) );

    EXPECT_EQ(g4.check_read(""), ( std::pair{Auditor::Prohibited, 0}) );
    EXPECT_EQ(g4.check_read("key"), ( std::pair{Auditor::Readonly, 0}) );

    EXPECT_EQ(g5.check_read("write"), ( std::pair{Auditor::ReadWrite, 0}) );
    EXPECT_EQ(g5.check_read("prohibited"), ( std::pair{Auditor::Prohibited, 0}) );
    EXPECT_EQ(g5.check_read(""), ( std::pair{Auditor::Readonly, 0}) );

}

TEST(ProxiedMapTest, BankSystem)
{
    std::map<std::string, int> bank_account$;
    bank_account$.insert({ "Pavel", 1000000 });
    bank_account$.insert({ "Lev", -100 });
    bank_account$.insert({ "Artem", 1 });

    OwnMapAuditor OldData({{ "Pavel", 5000 }, { "Lev", 0 }, { "Artem", -10 }});
    FullAccessAuditor Government;
    ReaderAuditor TaxInspector;

    SlaveAuditor Pavel({"Pavel"});
    SlaveAuditor Lev({"Lev"});
    SlaveAuditor Artem({"Artem"});

    ProxiedMap Map1(bank_account$, Pavel);
    ProxiedMap Map2(bank_account$, Lev);
    ProxiedMap Map3(bank_account$, Artem);
    ProxiedMap Map4(bank_account$, OldData);
    ProxiedMap Map5(bank_account$, Government);
    ProxiedMap Map6(bank_account$, TaxInspector);

    EXPECT_EQ(Map1.read("Pavel"), 1000000);
    EXPECT_EQ(Map2.read("Lev"), -100);
    EXPECT_EQ(Map3.read("Artem"), 1);

    EXPECT_EQ(Map4.read("Pavel"), 5000);
    EXPECT_EQ(Map4.read("Lev"), 0);
    EXPECT_EQ(Map4.read("Artem"), -10);

    EXPECT_EQ(Map5.read("Pavel"), 1000000);
    EXPECT_EQ(Map5.read("Lev"), -100);
    EXPECT_EQ(Map5.read("Artem"), 1);

    EXPECT_EQ(Map6.read("Pavel"), 1000000);
    EXPECT_EQ(Map6.read("Lev"), -100);
    EXPECT_EQ(Map6.read("Artem"), 1);
}

TEST(ProxiedMapTest, EditTest)
{
    std::map<std::string, int> bank_account$;
    bank_account$.insert({ "Employee", 10 });

    FullAccessAuditor Boss;
    ProxiedMap Map(bank_account$, Boss);

    EXPECT_EQ(Map.read("Employee"), 10);

    Map.edit("Employee", Map.read("Employee") + 1000);

    EXPECT_EQ(Map.read("Employee"), 1010);

}

TEST(ProxiedMapAddTest, FullAccessPermitsNewKey) {
    std::map<std::string, int> data;
    FullAccessAuditor auditor;
    ProxiedMap m(data, auditor);
    EXPECT_NO_THROW(m.add("Key1", 42));
    EXPECT_EQ(m.read("Key1"), 42);
}

TEST(ProxiedMapAddTest, FullAccessPermitsOverwrite) {
    std::map<std::string, int> data;
    data.insert({"Key1", 7});
    FullAccessAuditor auditor;
    ProxiedMap m(data, auditor);
    EXPECT_NO_THROW(m.add("Key1", 100));
    EXPECT_EQ(m.read("Key1"), 100);
}

TEST(ProxiedMapAddTest, OwnMapThrows) {
    std::map<std::string, int> data;
    OwnMapAuditor auditor;
    ProxiedMap m(data, auditor);
    EXPECT_THROW(m.add("Key1", 1), std::runtime_error);
}

TEST(ProxiedMapAddTest, ReaderThrows) {
    std::map<std::string, int> data;
    ReaderAuditor auditor;
    ProxiedMap m(data, auditor);
    EXPECT_THROW(m.add("Key1", 1), std::runtime_error);
}

TEST(ProxiedMapAddTest, SlaveThrows) {
    std::map<std::string, int> data;
    SlaveAuditor auditor({"Key1"});
    ProxiedMap m(data, auditor);
    EXPECT_THROW(m.add("Key1", 1), std::runtime_error);
}

TEST(ProxiedMapAddTest, ProxyThrows) {
    std::map<std::string, int> data;
    ProxyAuditor auditor({"Key1"}, {});
    ProxiedMap m(data, auditor);
    EXPECT_THROW(m.add("Key1", 1), std::runtime_error);
}

TEST(ProxiedMapRemoveTest, FullAccessPermitsExistingKey) {
    std::map<std::string, int> data;
    data.insert({"Key1", 7});
    FullAccessAuditor auditor;
    ProxiedMap m(data, auditor);
    EXPECT_NO_THROW(m.remove("Key1"));
    EXPECT_THROW(m.read("Key1"), std::out_of_range);
}

TEST(ProxiedMapRemoveTest, FullAccessPermitsNonExistingKey) {
    std::map<std::string, int> data;
    FullAccessAuditor auditor;
    ProxiedMap m(data, auditor);
    EXPECT_NO_THROW(m.remove("Key2"));
    EXPECT_THROW(m.read("Key2"), std::out_of_range);
}

TEST(ProxiedMapRemoveTest, OwnMapThrows) {
    std::map<std::string, int> data;
    data.insert({"Key1", 7});
    OwnMapAuditor auditor;
    ProxiedMap m(data, auditor);
    EXPECT_THROW(m.remove("Key1"), std::runtime_error);
}

TEST(ProxiedMapRemoveTest, ReaderThrows) {
    std::map<std::string, int> data;
    data.insert({"Key1", 7});
    ReaderAuditor auditor;
    ProxiedMap m(data, auditor);
    EXPECT_THROW(m.remove("Key1"), std::runtime_error);
}

TEST(ProxiedMapRemoveTest, SlaveThrows) {
    std::map<std::string, int> data;
    data.insert({"Key1", 7});
    SlaveAuditor auditor({"Key1"});
    ProxiedMap m(data, auditor);
    EXPECT_THROW(m.remove("Key1"), std::runtime_error);
}

TEST(ProxiedMapRemoveTest, ProxyThrows) {
    std::map<std::string, int> data;
    data.insert({"Key1", 7});
    ProxyAuditor auditor({}, {"Key1"});
    ProxiedMap m(data, auditor);
    EXPECT_THROW(m.remove("Key1"), std::runtime_error);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
