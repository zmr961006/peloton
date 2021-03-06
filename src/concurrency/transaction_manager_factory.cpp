//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// transaction_manager_factory.cpp
//
// Identification: src/concurrency/transaction_manager_factory.cpp
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//


#include "concurrency/transaction_manager_factory.h"

namespace peloton {
namespace concurrency {
ConcurrencyType TransactionManagerFactory::protocol_ =
    CONCURRENCY_TYPE_TIMESTAMP_ORDERING;
IsolationLevelType TransactionManagerFactory::isolation_level_ =
    ISOLATION_LEVEL_TYPE_FULL;
}
}
