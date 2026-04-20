#include "Reader.h"
#include "Session.h"

#include <algorithm>

namespace aidl::android::se {
using aidl::android::se::omapi::SecureElementSession;
    SecureElementReader::SecureElementReader(std::shared_ptr<SecureElementService> service,
                                             ::android::sp<Terminal> terminal)
        : mService(service),
          mTerminal(terminal) {}

    std::vector<uint8_t> SecureElementReader::getAtr() {
        LOG(INFO) << __PRETTY_FUNCTION__;
        return mTerminal->getAtr();
    }

    ::ndk::ScopedAStatus SecureElementReader::isSecureElementPresent(bool* isTrue) {
        LOG(INFO) << __PRETTY_FUNCTION__;
        *isTrue = mTerminal->isSecureElementPresent();
        return ::ndk::ScopedAStatus::ok();
    }

    ::ndk::ScopedAStatus SecureElementReader::closeSessions() {
        LOG(INFO) << __PRETTY_FUNCTION__;
        std::lock_guard<std::mutex> lock(mLock);
        for (auto& cSession : mSessions) {
                cSession->close();
        }
        mSessions.clear();
        return ::ndk::ScopedAStatus::ok();
    }

    void SecureElementReader::removeSession(SecureElementSession* session) {
        LOG(INFO) << __PRETTY_FUNCTION__;
        if (!session) {
            LOG(ERROR) << "Session is empty, failed to remove";
            return;
        }
        std::lock_guard<std::mutex> lock(mLock);
        mSessions.erase(
            std::remove_if(
                mSessions.begin(),
                mSessions.end(),
                [&session](const std::shared_ptr<aidl::android::se::omapi::SecureElementSession>& ptr) {
                    return ptr.get() == session;
                }
            ),
            mSessions.end()
        );
        if (mSessions.empty()) {
            mTerminal->mDefaultApplicationSelectedOnBasicChannel = true;
        }
    }

    ::ndk::ScopedAStatus SecureElementReader::openSession(std::shared_ptr<ISecureElementSession>* session) {
        LOG(INFO) << __PRETTY_FUNCTION__;
        if (!mTerminal->isSecureElementPresent()) {
            LOG(ERROR) << "Secure Element is not present";
            return ::ndk::ScopedAStatus::fromExceptionCodeWithMessage(
                    EX_ILLEGAL_STATE, "Secure Element is not present");
        }
        std::lock_guard<std::mutex> lock(mLock);
        auto nSession = ndk::SharedRefBase::make<SecureElementSession>(this->ref<SecureElementReader>());
        mSessions.push_back(nSession);
        *session = std::static_pointer_cast<ISecureElementSession>(nSession);
        return ::ndk::ScopedAStatus::ok();
    }

    ::android::sp<Terminal> SecureElementReader::getTerminal() {
        LOG(INFO) << __PRETTY_FUNCTION__;
        return mTerminal;
    }

    ::ndk::ScopedAStatus SecureElementReader::reset(bool* isReset) {
        LOG(INFO) << __PRETTY_FUNCTION__;
        *isReset = mTerminal->reset();
        return ::ndk::ScopedAStatus::ok();
    }
}