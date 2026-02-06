#ifndef DEMO_CONFIG_MANAGER_H
#define DEMO_CONFIG_MANAGER_H

#include <string>

namespace demo {
    struct DemoConfig;

    class DemoConfigManager {
    public:
        bool load_from_file(const std::string& path, DemoConfig& cfg);
        bool load_from_string(const std::string& str, DemoConfig& cfg);

        bool is_loaded() const { return is_loaded_; }

        bool save_to_file(const std::string& path, const DemoConfig& cfg);
        bool save_to_string(std::string& out_str, const DemoConfig& cfg);

    private:
        bool is_loaded_{false};
    };
}

#endif // DEMO_CONFIG_MANAGER_H