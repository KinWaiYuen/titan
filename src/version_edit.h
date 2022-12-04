#pragma once

#include <set>

#include "blob_format.h"
#include "rocksdb/slice.h"

#include <inttypes.h>

namespace rocksdb {
namespace titandb {

enum Tag {
  kNextFileNumber = 1,
  kColumnFamilyID = 10,
  kAddedBlobFile = 11,
  kDeletedBlobFile = 12,  // Deprecated, leave here for backward compatibility
  kAddedBlobFileV2 = 13,  // Comparing to kAddedBlobFile, it newly includes
                          // smallest_key and largest_key of blob file
  kUnfinalizedBlobFile = 14,
  kFinalizedBlobFile = 15
};

class VersionEdit {
 public:
  void SetNextFileNumber(uint64_t v) {
    has_next_file_number_ = true;
    next_file_number_ = v;
  }

  void SetColumnFamilyID(uint32_t v) { column_family_id_ = v; }

  void AddBlobFile(std::shared_ptr<BlobFileMeta> file) {
    added_files_.push_back(file);
  }

  void DeleteBlobFile(uint64_t file_number, SequenceNumber obsolete_sequence) {
    deleted_files_.emplace_back(std::make_pair(file_number, obsolete_sequence));
  }

  void AddGcOutputBlobFile(uint64_t file_number, SequenceNumber obsolete_sequence) {
    unfinalized_gc_output_files_.emplace_back(std::make_pair(file_number, obsolete_sequence));
  }

  void FinalizeGcOutputBlobFile(uint64_t file_number) {
    finalized_gc_output_files_.emplace_back(file_number);
  }

  void EncodeTo(std::string* dst) const;
  Status DecodeFrom(Slice* src);

  friend bool operator==(const VersionEdit& lhs, const VersionEdit& rhs);

  void Dump(bool with_keys) const;

 private:
  friend class BlobFileSet;
  friend class VersionTest;
  friend class EditCollector;

  bool has_next_file_number_{false};
  uint64_t next_file_number_{0};
  uint32_t column_family_id_{0};

  std::vector<std::shared_ptr<BlobFileMeta>> added_files_;
  std::vector<std::pair<uint64_t, SequenceNumber>> deleted_files_;
  std::vector<std::pair<uint64_t, SequenceNumber>> unfinalized_gc_output_files_;
  std::vector<uint64_t> finalized_gc_output_files_;
};

}  // namespace titandb
}  // namespace rocksdb
