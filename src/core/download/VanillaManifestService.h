#pragma once

#include "core/data/Minecraft.h"
#include <filesystem>
#include <string>

using namespace std;

namespace hinge::core::download {
    class VanillaManifestService {
        public:
            static data::VersionDetails fetchAndSave(const string& versionId,
                                                     const filesystem::path& gameDir);

            static data::VersionDetails loadLocal(const filesystem::path& gameDir,
                                                  const string& id);

            static data::VersionManifest fetchManifest();
    };
}
