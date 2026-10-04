#include "core/auth/MicrosoftAuthService.h"

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <openssl/evp.h>
#include <openssl/sha.h>

#include <array>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <future>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>

using namespace std;

namespace hinge::core::auth {
    namespace {
        string base64UrlEncode(const unsigned char* data, size_t len) {
            string buf(4 * ((len + 2) / 3), '\0');
            int written = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(buf.data()),
                                          data, static_cast<int>(len));
            buf.resize(written);

            for (char& c : buf) {
                if (c == '+') c = '-';
                else if (c == '/') c = '_';
            }
            while (!buf.empty() && buf.back() == '=') buf.pop_back();
            return buf;
        }

        void openInBrowser(const string& url) {
            #ifdef _WIN32
                system(("start \"\" \"" + url + "\"").c_str());
            #elif __APPLE__
                system(("open \"" + url + "\"").c_str());
            #else
                system(("xdg-open \"" + url + "\"").c_str());
            #endif
        }
    }

    void MSAuthService::authenticate(bool usePrism, AuthCallback& callback) {
        clientId_ = string(usePrism ? PRISM_CLIENT_ID : MAIN_CLIENT_ID);

        const int port = 4242;
        const string redirectUri = "http://127.0.0.1:" + to_string(port);

        promise<string> codePromise;
        auto codeFuture = codePromise.get_future();
        atomic<bool> handled{false};

        currentCodeVerifier_ = generateCodeVerifier();
        string codeChallenge = generateCodeChallenge(currentCodeVerifier_);

        httplib::Server oauthServer;
        oauthServer.Get("/", [&](const httplib::Request& req, httplib::Response& res) {
            string code = req.get_param_value("code");
            res.set_content(responseHtml_.empty() ? generateHtml() : responseHtml_,
                            "text/html; charset=utf-8");

            if (!handled.exchange(true)) {
                if (!code.empty()) codePromise.set_value(code);
                else codePromise.set_exception(
                    make_exception_ptr(runtime_error("Authorization code not found")));
            }
            oauthServer.stop();
        });

        thread serverThread([&] { oauthServer.listen("127.0.0.1", port); });

        string loginUrl =
            "https://login.live.com/oauth20_authorize.srf"
            "?client_id=" + clientId_ +
            "&response_type=code"
            "&redirect_uri=" + redirectUri +
            "&scope=XboxLive.SignIn+XboxLive.offline_access"
            "&prompt=select_account"
            "&code_challenge=" + codeChallenge +
            "&code_challenge_method=S256"
            "&ui_locales=ru-RU";

        openInBrowser(loginUrl);

        if (codeFuture.wait_for(chrono::minutes(3)) == future_status::timeout) {
            oauthServer.stop();
            serverThread.join();
            callback.onFailure("Authorization timed out");
            return;
        }

        string authCode;
        try {
            authCode = codeFuture.get();
        } catch (const exception& e) {
            oauthServer.stop();
            serverThread.join();
            callback.onFailure(string("Auth failed: ") + e.what());
            return;
        }

        serverThread.join();
        performMinecraftLogin(authCode, redirectUri, callback);
    }

    void MSAuthService::performMinecraftLogin(const string& authCode,
                                              const string& redirectUri,
                                              AuthCallback& callback) {
        try {
            string body =
                "client_id=" + clientId_ +
                "&code=" + authCode +
                "&grant_type=authorization_code"
                "&redirect_uri=" + redirectUri +
                "&code_verifier=" + currentCodeVerifier_;

            httplib::SSLClient cli("login.live.com");
            cli.enable_server_certificate_verification(false);
            cli.set_connection_timeout(10);
            cli.set_read_timeout(30);

            auto res = cli.Post("/oauth20_token.srf",
                {{"Content-Type", "application/x-www-form-urlencoded"}},
                body, "application/x-www-form-urlencoded");

            if (!res) {
                callback.onFailure("HTTP error: " + httplib::to_string(res.error()));
                return;
            }

            auto data = nlohmann::json::parse(res->body);

            if (res->status != 200 || data.contains("error")) {
                string err = data.value("error_description", string("OAuth error"));
                callback.onFailure("OAuth verification failed: " + err);
                return;
            }

            string msAccessToken = data.value("access_token", string());
            string refreshToken  = data.value("refresh_token", string());

            performXboxLogin(msAccessToken, refreshToken, callback);
        } catch (const exception& e) {
            callback.onFailure(string("Token exchange failed: ") + e.what());
        }
    }

    void MSAuthService::performXboxLogin(const string& msAccessToken,
                                         const string& refreshToken,
                                         AuthCallback& callback) {
        using nlohmann::json;

        try {
            json xblBody = {
                {"Properties", {
                    {"AuthMethod", "RPS"},
                    {"SiteName",   "user.auth.xboxlive.com"},
                    {"RpsTicket",  "d=" + msAccessToken}
                }},
                {"RelyingParty", "http://auth.xboxlive.com"},
                {"TokenType",    "JWT"}
            };

            httplib::SSLClient xbl("user.auth.xboxlive.com");
            xbl.enable_server_certificate_verification(false);
            auto xblRes = xbl.Post("/user/authenticate",
                {{"Content-Type", "application/json"}, {"Accept", "application/json"}},
                xblBody.dump(), "application/json");

            if (!xblRes || xblRes->status != 200) {
                callback.onFailure("XBL authentication failed");
                return;
            }

            auto xblData = json::parse(xblRes->body);
            string xblToken = xblData.value("Token", string());
            string userHash = xblData["DisplayClaims"]["xui"][0]["uhs"].get<string>();

            json xstsBody = {
                {"Properties", {
                    {"SandboxId",   "RETAIL"},
                    {"UserTokens",  json::array({xblToken})}
                }},
                {"RelyingParty", "rp://api.minecraftservices.com/"},
                {"TokenType",    "JWT"}
            };

            httplib::SSLClient xsts("xsts.auth.xboxlive.com");
            xsts.enable_server_certificate_verification(false);
            auto xstsRes = xsts.Post("/xsts/authorize",
                {{"Content-Type", "application/json"}, {"Accept", "application/json"}},
                xstsBody.dump(), "application/json");

            if (!xstsRes || xstsRes->status != 200) {
                callback.onFailure("XSTS authentication failed");
                return;
            }

            auto xstsData = json::parse(xstsRes->body);
            string xstsToken = xstsData.value("Token", string());

            json mcBody = {
                {"identityToken", "XBL3.0 x=" + userHash + ";" + xstsToken}
            };

            httplib::SSLClient mcs("api.minecraftservices.com");
            mcs.enable_server_certificate_verification(false);
            auto mcRes = mcs.Post("/authentication/login_with_xbox",
                {{"Content-Type", "application/json"}, {"Accept", "application/json"}},
                mcBody.dump(), "application/json");

            if (!mcRes || mcRes->status != 200) {
                callback.onFailure("Minecraft Services authentication failed");
                return;
            }

            auto mcData = json::parse(mcRes->body);
            string mcAccessToken = mcData.value("access_token", string());

            httplib::SSLClient profile("api.minecraftservices.com");
            profile.enable_server_certificate_verification(false);
            auto profileRes = profile.Get("/minecraft/profile",
                {{"Authorization", "Bearer " + mcAccessToken}});

            if (!profileRes || profileRes->status != 200) {
                callback.onFailure("Minecraft license not found");
                return;
            }

            auto profileData = json::parse(profileRes->body);
            string name    = profileData.value("name", string());
            string rawUuid = profileData.value("id",   string());

            string uuid = rawUuid;
            if (rawUuid.size() == 32) {
                uuid = rawUuid.substr(0, 8)  + "-" + rawUuid.substr(8, 4)  + "-" +
                       rawUuid.substr(12, 4) + "-" + rawUuid.substr(16, 4) + "-" +
                       rawUuid.substr(20, 12);
            }

            callback.onSuccess(name, uuid, mcAccessToken, refreshToken);
        } catch (const exception& e) {
            callback.onFailure(string("Network error during login: ") + e.what());
        }
    }

    string MSAuthService::generateCodeVerifier() {
        array<unsigned char, 32> bytes{};
        random_device rd;
        for (auto& b : bytes) b = static_cast<unsigned char>(rd());
        return base64UrlEncode(bytes.data(), bytes.size());
    }

    string MSAuthService::generateCodeChallenge(const string& verifier) {
        array<unsigned char, SHA256_DIGEST_LENGTH> hash{};
        SHA256(reinterpret_cast<const unsigned char*>(verifier.data()),
               verifier.size(), hash.data());
        return base64UrlEncode(hash.data(), hash.size());
    }

    string MSAuthService::generateHtml() {
        return R"(
<html>
<body style="font-family: sans-serif; text-align: center; padding-top: 60px; background: #2b2b2b; color: white;">
    <h1 style="color: #2ecc71;">Авторизация успешна!</h1>
    <p style="font-size: 16px;">Вы можете закрыть эту вкладку браузера и вернуться в лаунчер Hinge.</p>
</body>
</html>)";
    }
}
