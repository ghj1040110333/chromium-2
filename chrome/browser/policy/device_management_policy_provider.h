// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_POLICY_DEVICE_MANAGEMENT_POLICY_PROVIDER_H_
#define CHROME_BROWSER_POLICY_DEVICE_MANAGEMENT_POLICY_PROVIDER_H_
#pragma once

#include <string>

#include "base/file_path.h"
#include "base/scoped_ptr.h"
#include "base/time.h"
#include "base/weak_ptr.h"
#include "chrome/browser/policy/configuration_policy_provider.h"
#include "chrome/browser/policy/device_management_backend.h"
#include "chrome/browser/policy/device_token_fetcher.h"

class TokenService;

namespace policy {

class DeviceManagementBackend;
class DeviceManagementPolicyCache;

// Provides policy fetched from the device management server. With the exception
// of the Provide method, which can be called on the FILE thread, all public
// methods must be called on the UI thread.
class DeviceManagementPolicyProvider
    :  public ConfigurationPolicyProvider,
       public DeviceManagementBackend::DevicePolicyResponseDelegate,
       public base::SupportsWeakPtr<DeviceManagementPolicyProvider>,
       public DeviceTokenFetcher::Observer {
 public:
  DeviceManagementPolicyProvider(const PolicyDefinitionList* policy_list,
                                 DeviceManagementBackend* backend,
                                 TokenService* token_service,
                                 const FilePath& storage_dir);

  virtual ~DeviceManagementPolicyProvider();

  // ConfigurationPolicyProvider implementation:
  virtual bool Provide(ConfigurationPolicyStoreInterface* store);

  // DevicePolicyResponseDelegate implementation:
  virtual void HandlePolicyResponse(
      const em::DevicePolicyResponse& response);
  virtual void OnError(DeviceManagementBackend::ErrorCode code);

  // DeviceTokenFetcher::Observer implementation:
  void OnTokenSuccess();
  void OnTokenError();
  void OnNotManaged();

  // True if a policy request has been sent to the device management backend
  // server and no response or error has yet been received.
  bool IsPolicyRequestPending() const { return policy_request_pending_; }

  // Tells the provider that the passed in token service reference is about to
  // become invalid.
  void Shutdown();

 private:
  class InitializeAfterIOThreadExistsTask;
  class RefreshTask;

  friend class DeviceManagementPolicyProviderTest;

  // Called by constructors to perform shared initialization. Initialization
  // requiring the IOThread must not be performed directly in this method,
  // rather must be deferred until the IOThread is fully initialized. This is
  // the case in InitializeAfterIOThreadExists.
  void Initialize();

  // Called by a deferred task posted to the UI thread to complete the portion
  // of initialization that requires the IOThread.
  void InitializeAfterIOThreadExists();

  // Sends a request to the device manager backend to fetch policy if one isn't
  // already outstanding.
  void SendPolicyRequest();

  // Triggers policy refresh, re-requesting device token and policy information
  // as necessary.
  void RefreshTaskExecute();

  // Schedules a new RefreshTask.
  void ScheduleRefreshTask(int64 delay_in_milliseconds);

  // Calculates when the next RefreshTask shall be executed.
  int64 GetRefreshTaskDelay();

  // Provides the URL at which requests are sent to from the device management
  // backend.
  static std::string GetDeviceManagementURL();

  // Returns the path to the sub-directory in the user data directory
  // in which device management persistent state is stored.
  static FilePath GetOrCreateDeviceManagementDir(
      const FilePath& user_data_dir);

  // Give unit tests the ability to override timeout settings.
  void set_policy_refresh_rate_ms(int64 policy_refresh_rate_ms) {
    policy_refresh_rate_ms_ = policy_refresh_rate_ms;
  }
  void set_policy_refresh_max_earlier_ms(int64 policy_refresh_max_earlier_ms) {
    policy_refresh_max_earlier_ms_ = policy_refresh_max_earlier_ms;
  }
  void set_policy_refresh_error_delay_ms(int64 policy_refresh_error_delay_ms) {
    policy_refresh_error_delay_ms_ = policy_refresh_error_delay_ms;
  }
  void set_token_fetch_error_delay_ms(int64 token_fetch_error_delay_ms) {
    token_fetch_error_delay_ms_ = token_fetch_error_delay_ms;
  }

  scoped_ptr<DeviceManagementBackend> backend_;
  TokenService* token_service_;  // weak
  scoped_ptr<DeviceManagementPolicyCache> cache_;
  scoped_refptr<DeviceTokenFetcher> token_fetcher_;
  DeviceTokenFetcher::ObserverRegistrar registrar_;
  FilePath storage_dir_;
  bool policy_request_pending_;
  bool refresh_task_pending_;
  int64 policy_refresh_rate_ms_;
  int64 policy_refresh_max_earlier_ms_;
  int64 policy_refresh_error_delay_ms_;
  int64 token_fetch_error_delay_ms_;

  DISALLOW_COPY_AND_ASSIGN(DeviceManagementPolicyProvider);
};

}  // namespace policy

#endif  // CHROME_BROWSER_POLICY_DEVICE_MANAGEMENT_POLICY_PROVIDER_H_
