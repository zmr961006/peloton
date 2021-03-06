//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// backend_stats_context.cpp
//
// Identification: src/statistics/backend_stats_context.cpp
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <map>

#include "common/types.h"
#include "catalog/catalog.h"
#include "index/index.h"
#include "statistics/backend_stats_context.h"
#include "statistics/stats_aggregator.h"
#include "statistics/counter_metric.h"
#include "storage/database.h"
#include "storage/tile_group.h"

namespace peloton {
namespace stats {

BackendStatsContext& BackendStatsContext::GetInstance() {

  // Each thread gets a backend stats context
  thread_local static BackendStatsContext stats_context(
      LATENCY_MAX_HISTORY_THREAD, true);

  return stats_context;
}

BackendStatsContext::BackendStatsContext(size_t max_latency_history,
                                         bool regiser_to_aggregator)
    : txn_latencies_(LATENCY_METRIC, max_latency_history) {
  std::thread::id this_id = std::this_thread::get_id();
  thread_id_ = this_id;

  is_registered_to_aggregator_ = regiser_to_aggregator;

  // Register to the global aggregator
  if (regiser_to_aggregator == true)
    StatsAggregator::GetInstance().RegisterContext(thread_id_, this);
}

BackendStatsContext::~BackendStatsContext() {
  if (is_registered_to_aggregator_ == true)
    StatsAggregator::GetInstance().UnregisterContext(thread_id_);
}

//===--------------------------------------------------------------------===//
// ACCESSORS
//===--------------------------------------------------------------------===//

// Returns the table metric with the given database ID and table ID
TableMetric* BackendStatsContext::GetTableMetric(oid_t database_id,
                                                 oid_t table_id) {
  TableMetric::TableKey table_key = TableMetric::GetKey(database_id, table_id);
  if (table_metrics_.find(table_key) == table_metrics_.end()) {
    table_metrics_[table_key] = std::unique_ptr<TableMetric>(
        new TableMetric{TABLE_METRIC, database_id, table_id});
  }
  return table_metrics_[table_key].get();
}

// Returns the database metric with the given database ID
DatabaseMetric* BackendStatsContext::GetDatabaseMetric(oid_t database_id) {
  if (database_metrics_.find(database_id) == database_metrics_.end()) {
    database_metrics_[database_id] = std::unique_ptr<DatabaseMetric>(
        new DatabaseMetric{DATABASE_METRIC, database_id});
  }
  return database_metrics_[database_id].get();
}

// Returns the index metric with the given database ID, table ID, and
// index ID
IndexMetric* BackendStatsContext::GetIndexMetric(oid_t database_id,
                                                 oid_t table_id,
                                                 oid_t index_id) {
  IndexMetric::IndexKey index_key =
      IndexMetric::GetKey(database_id, table_id, index_id);
  if (index_metrics_.find(index_key) == index_metrics_.end()) {
    index_metrics_[index_key] = std::unique_ptr<IndexMetric>(
        new IndexMetric{INDEX_METRIC, database_id, table_id, index_id});
  }
  return index_metrics_[index_key].get();
}

LatencyMetric& BackendStatsContext::GetTxnLatencyMetric() {
  return txn_latencies_;
}

void BackendStatsContext::IncrementTableReads(oid_t tile_group_id) {
  oid_t table_id =
      catalog::Manager::GetInstance().GetTileGroup(tile_group_id)->GetTableId();
  oid_t database_id = catalog::Manager::GetInstance()
                          .GetTileGroup(tile_group_id)
                          ->GetDatabaseId();
  auto table_metric = GetTableMetric(database_id, table_id);
  PL_ASSERT(table_metric != nullptr);
  table_metric->GetTableAccess().IncrementReads();
}

void BackendStatsContext::IncrementTableInserts(oid_t tile_group_id) {
  oid_t table_id =
      catalog::Manager::GetInstance().GetTileGroup(tile_group_id)->GetTableId();
  oid_t database_id = catalog::Manager::GetInstance()
                          .GetTileGroup(tile_group_id)
                          ->GetDatabaseId();
  auto table_metric = GetTableMetric(database_id, table_id);
  PL_ASSERT(table_metric != nullptr);
  table_metric->GetTableAccess().IncrementInserts();
}

void BackendStatsContext::IncrementTableUpdates(oid_t tile_group_id) {
  oid_t table_id =
      catalog::Manager::GetInstance().GetTileGroup(tile_group_id)->GetTableId();
  oid_t database_id = catalog::Manager::GetInstance()
                          .GetTileGroup(tile_group_id)
                          ->GetDatabaseId();
  auto table_metric = GetTableMetric(database_id, table_id);
  PL_ASSERT(table_metric != nullptr);
  table_metric->GetTableAccess().IncrementUpdates();
}

void BackendStatsContext::IncrementTableDeletes(oid_t tile_group_id) {
  oid_t table_id =
      catalog::Manager::GetInstance().GetTileGroup(tile_group_id)->GetTableId();
  oid_t database_id = catalog::Manager::GetInstance()
                          .GetTileGroup(tile_group_id)
                          ->GetDatabaseId();
  auto table_metric = GetTableMetric(database_id, table_id);
  PL_ASSERT(table_metric != nullptr);
  table_metric->GetTableAccess().IncrementDeletes();
}

void BackendStatsContext::IncrementIndexReads(size_t read_count,
                                              index::IndexMetadata* metadata) {
  oid_t index_id = metadata->GetOid();
  oid_t table_id = metadata->GetTableOid();
  oid_t database_id = metadata->GetDatabaseOid();
  auto index_metric = GetIndexMetric(database_id, table_id, index_id);
  PL_ASSERT(index_metric != nullptr);
  index_metric->GetIndexAccess().IncrementReads(read_count);
}

void BackendStatsContext::IncrementIndexInserts(
    index::IndexMetadata* metadata) {
  oid_t index_id = metadata->GetOid();
  oid_t table_id = metadata->GetTableOid();
  oid_t database_id = metadata->GetDatabaseOid();
  auto index_metric = GetIndexMetric(database_id, table_id, index_id);
  PL_ASSERT(index_metric != nullptr);
  index_metric->GetIndexAccess().IncrementInserts();
}

void BackendStatsContext::IncrementTableUpdates(
    index::IndexMetadata* metadata) {
  oid_t index_id = metadata->GetOid();
  oid_t table_id = metadata->GetTableOid();
  oid_t database_id = metadata->GetDatabaseOid();
  auto index_metric = GetIndexMetric(database_id, table_id, index_id);
  PL_ASSERT(index_metric != nullptr);
  index_metric->GetIndexAccess().IncrementUpdates();
}

void BackendStatsContext::IncrementIndexDeletes(
    size_t delete_count, index::IndexMetadata* metadata) {
  oid_t index_id = metadata->GetOid();
  oid_t table_id = metadata->GetTableOid();
  oid_t database_id = metadata->GetDatabaseOid();
  auto index_metric = GetIndexMetric(database_id, table_id, index_id);
  PL_ASSERT(index_metric != nullptr);
  index_metric->GetIndexAccess().IncrementDeletes(delete_count);
}

void BackendStatsContext::IncrementTxnCommitted(oid_t database_id) {
  auto database_metric = GetDatabaseMetric(database_id);
  PL_ASSERT(database_metric != nullptr);
  database_metric->IncrementTxnCommitted();
}

void BackendStatsContext::IncrementTxnAborted(oid_t database_id) {
  auto database_metric = GetDatabaseMetric(database_id);
  PL_ASSERT(database_metric != nullptr);
  database_metric->IncrementTxnAborted();
}

//===--------------------------------------------------------------------===//
// HELPER FUNCTIONS
//===--------------------------------------------------------------------===//

bool BackendStatsContext::operator==(const BackendStatsContext& other) {
  return database_metrics_ == other.database_metrics_ &&
         table_metrics_ == other.table_metrics_ &&
         index_metrics_ == other.index_metrics_;
}

bool BackendStatsContext::operator!=(const BackendStatsContext& other) {
  return !(*this == other);
}

void BackendStatsContext::Aggregate(BackendStatsContext& source) {
  // Aggregate all global metrics
  txn_latencies_.Aggregate(source.txn_latencies_);
  txn_latencies_.ComputeLatencies();

  // Aggregate all per-database metrics
  //  for (auto& database_item : database_metrics_) {
  //    auto worker_db_metric = source.GetDatabaseMetric(database_item.first);
  //    if (worker_db_metric != nullptr) {
  //      database_item.second->Aggregate(*worker_db_metric);
  //    }
  //  }
  for (auto& database_item : source.database_metrics_) {
    GetDatabaseMetric(database_item.first)->Aggregate(*database_item.second);
  }

  // Aggregate all per-table metrics
  //  for (auto& table_item : table_metrics_) {
  //    auto worker_table_metric = source.GetTableMetric(
  //        table_item.second->GetDatabaseId(),
  // table_item.second->GetTableId());
  //    if (worker_table_metric != nullptr) {
  //      table_item.second->Aggregate(*worker_table_metric);
  //    }
  //  }
  for (auto& table_item : source.table_metrics_) {
    GetTableMetric(table_item.second->GetDatabaseId(),
                   table_item.second->GetTableId())
        ->Aggregate(*table_item.second);
  }

  // Aggregate all per-index metrics
  //  for (auto& index_item : source.index_metrics_) {
  //    auto worker_index_metric = GetIndexMetric(
  //        index_item.second->GetDatabaseId(), index_item.second->GetTableId(),
  //        index_item.second->GetIndexId());
  //    if (worker_index_metric != nullptr) {
  //      index_item.second->Aggregate(*worker_index_metric);
  //    }
  //  }
  for (auto& index_item : source.index_metrics_) {
    GetIndexMetric(
        index_item.second->GetDatabaseId(), index_item.second->GetTableId(),
        index_item.second->GetIndexId())->Aggregate(*index_item.second);
  }
}

void BackendStatsContext::Reset() {
  txn_latencies_.Reset();

  for (auto& database_item : database_metrics_) {
    database_item.second->Reset();
  }
  for (auto& table_item : table_metrics_) {
    table_item.second->Reset();
  }
  for (auto& index_item : index_metrics_) {
    index_item.second->Reset();
  }

  oid_t num_databases = catalog::Catalog::GetInstance()->GetDatabaseCount();
  for (oid_t i = 0; i < num_databases; ++i) {
    auto database = catalog::Catalog::GetInstance()->GetDatabaseWithOffset(i);
    oid_t database_id = database->GetOid();

    // Reset database metrics
    if (database_metrics_.find(database_id) == database_metrics_.end()) {
      database_metrics_[database_id] = std::unique_ptr<DatabaseMetric>(
          new DatabaseMetric{DATABASE_METRIC, database_id});
    }

    // Reset table metrics
    oid_t num_tables = database->GetTableCount();
    for (oid_t j = 0; j < num_tables; ++j) {
      auto table = database->GetTable(j);
      oid_t table_id = table->GetOid();
      TableMetric::TableKey table_key =
          TableMetric::GetKey(database_id, table_id);

      if (table_metrics_.find(table_key) == table_metrics_.end()) {
        table_metrics_[table_key] = std::unique_ptr<TableMetric>(
            new TableMetric{TABLE_METRIC, database_id, table_id});
      }

      // Reset indexes metrics
      oid_t num_indexes = table->GetIndexCount();
      for (oid_t k = 0; k < num_indexes; ++k) {
        auto index = table->GetIndex(k);
        oid_t index_id = index->GetOid();
        IndexMetric::IndexKey index_key =
            IndexMetric::GetKey(database_id, table_id, index_id);

        if (index_metrics_.find(index_key) == index_metrics_.end()) {
          index_metrics_[index_key] = std::unique_ptr<IndexMetric>(
              new IndexMetric{INDEX_METRIC, database_id, table_id, index_id});
        }
      }
    }
  }
}

std::string BackendStatsContext::ToString() const {
  std::stringstream ss;

  ss << txn_latencies_.GetInfo() << std::endl;

  for (auto& database_item : database_metrics_) {
    oid_t database_id = database_item.second->GetDatabaseId();
    ss << database_item.second->GetInfo();

    for (auto& table_item : table_metrics_) {
      if (table_item.second->GetDatabaseId() == database_id) {
        ss << table_item.second->GetInfo();

        oid_t table_id = table_item.second->GetTableId();
        for (auto& index_item : index_metrics_) {
          if (index_item.second->GetDatabaseId() == database_id &&
              index_item.second->GetTableId() == table_id) {
            ss << index_item.second->GetInfo();
          }
        }
        if (!index_metrics_.empty()) {
          ss << std::endl;
        }
      }
      if (!table_metrics_.empty()) {
        ss << std::endl;
      }
    }
    if (!database_metrics_.empty()) {
      ss << std::endl;
    }
  }

  return ss.str();
}

}  // namespace stats
}  // namespace peloton
