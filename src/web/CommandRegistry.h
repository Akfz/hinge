#pragma once

#include "nlohmann/json_fwd.hpp"
#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

namespace hinge::web {
    struct CommandParam {
        string name;
        string description;
        string type;
        bool required = false;
        string defaultValue;
        vector<string> options;
    };

    using CommandHandler = function<nlohmann::json(const nlohmann::json& params)>;

    struct Command {
        string id;
        string title;
        string description;
        string category;
        vector<CommandParam> params;
        CommandHandler handler;
    };

    class CommandRegistry;

    class CommandBuilder {
        public:
            CommandBuilder(CommandRegistry& reg, string id);

            CommandBuilder& title(string v);
            CommandBuilder& description(string v);
            CommandBuilder& category(string v);

            CommandBuilder& stringParam(string name, string desc, bool required = false,
            string def = "");
            CommandBuilder& intParam(string name, string desc, int def = 0);
            CommandBuilder& boolParam(string name, string desc, bool def = false);
            CommandBuilder& selectParam(string name, string desc,
            vector<string> options);

            void handler(CommandHandler h);
        private:
            CommandRegistry& reg_;
            Command cmd_;
    };

    class CommandRegistry {
        public:
            CommandBuilder add(string id);

            nlohmann::json listJson() const;
            nlohmann::json invoke(const string& id, const nlohmann::json& params);

            const vector<Command>& all() const {
                return commands_;
            }
        private:
            friend class CommandBuilder;
            void registerCommand(Command cmd);

            vector<Command> commands_;
            unordered_map<string, size_t> index_;
    };
}
