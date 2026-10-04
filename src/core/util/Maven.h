#pragma once

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

namespace hinge::core::util {
    inline string mavenToPath(const string& mavenName) {
        string name = mavenName;
        string ext = "jar";

        auto at = name.find('@');
        if (at != string::npos) {
            ext = name.substr(at + 1);
            name = name.substr(0, at);
        }

        vector<string> parts;
        stringstream ss(name);
        string tok;
        while (getline(ss, tok, ':')) parts.push_back(tok);

        if (parts.size() < 3) return name;

        string group = parts[0];
        string artifact = parts[1];
        string version = parts[2];
        string classifier = parts.size() > 3 ? parts[3] : "";

        replace(group.begin(), group.end(), '.', '/');

        string file = artifact + "-" + version;
        if (!classifier.empty()) file += "-" + classifier;
        file += "." + ext;

        return group + "/" + artifact + "/" + version + "/" + file;
    }
}
