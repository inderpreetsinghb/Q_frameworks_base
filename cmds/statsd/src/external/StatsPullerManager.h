/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <android/os/IStatsCompanionService.h>
#include <binder/IServiceManager.h>
#include <utils/RefBase.h>
#include <utils/String16.h>
#include <utils/String8.h>
#include <utils/threads.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "PullDataReceiver.h"
#include "StatsPuller.h"
#include "logd/LogEvent.h"

namespace android {
namespace os {
namespace statsd {

class StatsPullerManager : public virtual RefBase {
public:
    static StatsPullerManager& GetInstance();

    void RegisterReceiver(int pullCode, sp<PullDataReceiver> receiver, long intervalMs);

    void UnRegisterReceiver(int pullCode, sp<PullDataReceiver> receiver);

    // We return a vector of shared_ptr since LogEvent's copy constructor is not available.
    vector<std::shared_ptr<LogEvent>> Pull(const int pullCode, const uint64_t timestampSec);

    // Translate metric name to pullCodes.
    // return -1 if no valid pullCode is found
    int GetPullCode(std::string metricName);

    void OnAlarmFired();

private:
    StatsPullerManager();

    sp<IStatsCompanionService> mStatsCompanionService = nullptr;

    sp<IStatsCompanionService> get_stats_companion_service();

    std::unordered_map<int, std::unique_ptr<StatsPuller>> mPullers;



      // internal state of a bucket.
      typedef struct {
        // pull_interval_sec : last_pull_time_sec
        std::pair<uint64_t, uint64_t> timeInfo;
        sp<PullDataReceiver> receiver;
      } ReceiverInfo;

    std::map<int, std::vector<ReceiverInfo>> mReceivers;

    Mutex mReceiversLock;

    long mCurrentPullingInterval;

    // for value metrics, it is important for the buckets to be aligned to multiple of smallest
    // bucket size. All pulled metrics start pulling based on this time, so that they can be
    // correctly attributed to the correct buckets. Pulled data attach a timestamp which is the
    // request time.
    const long mPullStartTimeMs;

    long get_pull_start_time_ms();

    LogEvent parse_pulled_data(String16 data);

    static const std::unordered_map<std::string, int> kPullCodes;
};

}  // namespace statsd
}  // namespace os
}  // namespace android