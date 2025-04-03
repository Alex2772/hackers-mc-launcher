#include <gmock/gmock.h>
#include "Launcher.h"


TEST(Common, PopulateUuid) {
    // This is a real username. And real uuid.
    Account account {
        .username = "Alex2772",
    };
    *account.populateUuid();
    EXPECT_EQ(account.uuid.toRawString(), "b2d966668241409284decb1a5141112b");
}