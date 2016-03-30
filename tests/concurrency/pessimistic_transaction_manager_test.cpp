//===----------------------------------------------------------------------===//
//
//                         PelotonDB
//
// pessimistic_transaction_manager_test.cpp
//
// Identification: tests/concurrency/pessimistic_transaction_manager_test.cpp
//
// Copyright (c) 2015, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "harness.h"
#include "concurrency/transaction_tests_util.h"

namespace peloton {

namespace test {

//===--------------------------------------------------------------------===//
// Transaction Tests
//===--------------------------------------------------------------------===//

class PessimisticTransactionManagerTests : public PelotonTest {};

TEST_F(PessimisticTransactionManagerTests, Test) {
  concurrency::TransactionManagerFactory::Configure(CONCURRENCY_TYPE_2PL);
  EXPECT_TRUE(true);
}

}  // End test namespace
}  // End peloton namespace
