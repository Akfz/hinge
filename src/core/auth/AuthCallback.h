#pragma once

#include <string>

using namespace std;

namespace hinge::core::auth {
    class AuthCallback {
        public:
            virtual ~AuthCallback() = default;

            virtual void onSuccess(string name,
                                   string uuid,
                                   string accessToken,
                                   string refreshToken) = 0;

            virtual void onFailure(string error) = 0;
    };
}
