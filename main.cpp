#include "Service.h"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include <csignal>
#include <cstdlib>

using aidl::android::se::omapi::SecureElementService;

namespace {
void handleTerminationSignal(int sig) {
    LOG(INFO) << "Caught signal " << sig << ", exiting";
    _exit(0);
}
}  // namespace

int main() {
    std::signal(SIGTERM, handleTerminationSignal);
    std::signal(SIGINT, handleTerminationSignal);

    ABinderProcess_setThreadPoolMaxThreadCount(0);
    std::shared_ptr<SecureElementService> hal = ndk::SharedRefBase::make<SecureElementService>();

    const std::string instance = std::string(SecureElementService::descriptor) + "/default";
    auto status = AServiceManager_addService(hal->asBinder().get(), instance.c_str());
    CHECK_EQ(status, STATUS_OK) << "Failed to add service " << instance << " " << status;
    LOG(INFO) << "SecureElementService AIDL service(omapi) running...";
    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should not reach
}
