#include "web/CommandRegistry.h"

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace hinge::web {
    CommandBuilder::CommandBuilder(CommandRegistry& reg, string id)
        : reg_(reg) {
        cmd_.id = std::move(id);
        cmd_.title = cmd_.id;
    }

    CommandBuilder& CommandBuilder::title(string v) {
        cmd_.title = std::move(v);
        return *this;
    }

    CommandBuilder& CommandBuilder::description(string v) {
        cmd_.description = std::move(v);
        return *this;
    }

    CommandBuilder& CommandBuilder::category(string v) {
        cmd_.category = std::move(v);
        return *this;
    }

    CommandBuilder& CommandBuilder::stringParam(string name, string desc,
                                                bool required, string def) {
        cmd_.params.push_back({std::move(name), std::move(desc), "string",
                               required, std::move(def), {}});
        return *this;
    }

    CommandBuilder& CommandBuilder::intParam(string name, string desc, int def) {
        cmd_.params.push_back({std::move(name), std::move(desc), "int",
                               false, to_string(def), {}});
        return *this;
    }

    CommandBuilder& CommandBuilder::boolParam(string name, string desc, bool def) {
        cmd_.params.push_back({std::move(name), std::move(desc), "bool",
                               false, def ? "true" : "false", {}});
        return *this;
    }

    CommandBuilder& CommandBuilder::selectParam(string name, string desc,
                                                vector<string> options) {
        CommandParam p;
        p.name = std::move(name);
        p.description = std::move(desc);
        p.type = "select";
        p.options = std::move(options);
        if (!p.options.empty()) p.defaultValue = p.options.front();
        cmd_.params.push_back(std::move(p));
        return *this;
    }

    void CommandBuilder::handler(CommandHandler h) {
        if (!h) throw runtime_error("Command '" + cmd_.id + "' has no handler");
        cmd_.handler = std::move(h);
        reg_.registerCommand(std::move(cmd_));
    }

    CommandBuilder CommandRegistry::add(string id) {
        return CommandBuilder(*this, std::move(id));
    }

    void CommandRegistry::registerCommand(Command cmd) {
        if (index_.count(cmd.id)) {
            throw runtime_error("Command '" + cmd.id + "' is already registered");
        }
        index_[cmd.id] = commands_.size();
        commands_.push_back(std::move(cmd));
    }

    nlohmann::json CommandRegistry::listJson() const {
        nlohmann::json arr = nlohmann::json::array();

        for (const auto& c : commands_) {
            nlohmann::json params = nlohmann::json::array();
            for (const auto& p : c.params) {
                nlohmann::json jp = {
                    {"name",        p.name},
                    {"description", p.description},
                    {"type",        p.type},
                    {"required",    p.required},
                    {"default",     p.defaultValue}
                };
                if (!p.options.empty()) jp["options"] = p.options;
                params.push_back(std::move(jp));
            }
            arr.push_back({
                {"id",          c.id},
                {"title",       c.title},
                {"description", c.description},
                {"category",    c.category},
                {"params",      params}
            });
        }
        return arr;
    }

    nlohmann::json CommandRegistry::invoke(const string& id, const nlohmann::json& params) {
        auto it = index_.find(id);
        if (it == index_.end()) {
            return {{"ok", false}, {"error", "Unknown command: " + id}};
        }
        try {
            auto result = commands_[it->second].handler(params);
            return {{"ok", true}, {"result", std::move(result)}};
        } catch (const exception& e) {
            return {{"ok", false}, {"error", e.what()}};
        }
    }
}
