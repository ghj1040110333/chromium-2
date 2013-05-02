// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_DRIVE_TEST_UTIL_H_
#define CHROME_BROWSER_CHROMEOS_DRIVE_TEST_UTIL_H_

#include <string>

#include "chrome/browser/google_apis/test_util.h"

namespace drive {

class FileCacheEntry;
class ChangeListLoader;

namespace test_util {

// This is a bitmask of cache states in FileCacheEntry. Used only in tests.
enum TestFileCacheState {
  TEST_CACHE_STATE_NONE       = 0,
  TEST_CACHE_STATE_PINNED     = 1 << 0,
  TEST_CACHE_STATE_PRESENT    = 1 << 1,
  TEST_CACHE_STATE_DIRTY      = 1 << 2,
  TEST_CACHE_STATE_MOUNTED    = 1 << 3,
  TEST_CACHE_STATE_PERSISTENT = 1 << 4,
};

// Converts |cache_state| which is a bit mask of TestFileCacheState, to a
// FileCacheEntry.
FileCacheEntry ToCacheEntry(int cache_state);

// Returns true if the cache state of the given two cache entries are equal.
bool CacheStatesEqual(const FileCacheEntry& a, const FileCacheEntry& b);

// Loads a test json file as root ("/drive") element from a test file stored
// under chrome/test/data/chromeos. Returns true on success.
bool LoadChangeFeed(const std::string& relative_path,
                    ChangeListLoader* change_list_loader,
                    bool is_delta_feed,
                    const std::string& root_resource_id,
                    int64 root_feed_changestamp);

// Helper to destroy objects which needs Destroy() to be called on destruction.
// Note: When using this helper, you should destruct objects before
// BrowserThread.
struct DestroyHelperForTests {
  template<typename T>
  void operator()(T* object) const {
    if (object) {
      object->Destroy();
      google_apis::test_util::RunBlockingPoolTask();  // Finish destruction.
    }
  }
};

}  // namespace test_util
}  // namespace drive

#endif  // CHROME_BROWSER_CHROMEOS_DRIVE_TEST_UTIL_H_
