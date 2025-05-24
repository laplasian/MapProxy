#include <gtest/gtest.h>
#include "Auditor.h"
#include "ProxiedMap.h"
#include <map>

TEST(AuditorTest, BasicTest) {
    OwnMap g1({{"key", 1}});
    FullAccess g2;
    Reader g3;
    Slave g4({"key"});
    Proxy g5({"write"}, {"prohibited"});

    EXPECT_EQ(g1.check_read(""), ( std::pair{Auditor::Readonly, 0}) );
    EXPECT_EQ(g1.check_read("key"), (std::pair{Auditor::Owner, 1}));

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

    OwnMap OldData({{ "Pavel", 5000 }, { "Lev", 0 }, { "Artem", -10 }});
    FullAccess Government;
    Reader TaxInspector;

    Slave Pavel({"Pavel"});
    Slave Lev({"Lev"});
    Slave Artem({"Artem"});

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

    FullAccess Boss;
    ProxiedMap Map(bank_account$, Boss);

    EXPECT_EQ(Map.read("Employee"), 10);

    Map.edit("Employee", Map.read("Employee") + 1000);

    EXPECT_EQ(Map.read("Employee"), 1010);

}

TEST(ProxiedMapTest, NoAccessThrow)
{
    std::map<std::string, int> bank_account$;
    bank_account$.insert({ "Employee", 10 });

    Reader Employee;
    ProxiedMap Map(bank_account$, Employee);
    EXPECT_THROW(Map.edit("Employee", 1000000000000), std::runtime_error);
}

TEST(ProxiedMapTest, NoKeyThrow)
{
    std::map<std::string, int> people;
    people.insert({ "Person1", 56 });

    Reader Funeral;
    ProxiedMap Map(people, Funeral);
    EXPECT_THROW(Map.read("Person2"), std::out_of_range);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
