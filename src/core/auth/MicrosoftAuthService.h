#pragma once

#include "core/auth/AuthCallback.h"
#include <string>
#include <string_view>

using namespace std;

namespace hinge::core::auth {
    class MSAuthService {
        public:
            static MSAuthService& instance() {
                static MSAuthService inst;
                return inst;
            }

            MSAuthService(const MSAuthService&)            = delete;
            MSAuthService& operator=(const MSAuthService&) = delete;
            MSAuthService(MSAuthService&&)                 = delete;
            MSAuthService& operator=(MSAuthService&&)      = delete;

            void authenticate(bool usePrism, AuthCallback& callback);
        private:
            MSAuthService() = default;
            ~MSAuthService() = default;

            void performMinecraftLogin(const string& authCode,
                                       const string& redirectUri,
                                       AuthCallback& callback);
            void performXboxLogin(const string& msAccessToken,
                                  const string& refreshToken,
                                  AuthCallback& callback);

            string generateCodeVerifier();
            string generateCodeChallenge(const string& verifier);
            string generateHtml();

            inline static constexpr string_view MAIN_CLIENT_ID  = "eb0e83bb-b472-4b6b-97e2-4c886d97f88c";
            inline static constexpr string_view PRISM_CLIENT_ID = "c36a9fb6-4f2a-41ff-90bd-ae7cc92031eb";

            string clientId_;
            string currentCodeVerifier_;
            string responseHtml_;
    };
}
