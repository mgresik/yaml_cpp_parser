#ifndef DEMO_CONFIG_H
#define DEMO_CONFIG_H

#include <string>
#include <vector>
#include <cstdint>
#include <ostream>
#include <optional>

namespace demo {
    struct ServerSettings {
        std::string proto;
        std::uint16_t port{0};
    };

    struct LogSettings {
        std::string level;
        std::string folder;
    };

    struct MtlsSettings {
        bool use_mtls_auth{false};
        std::vector<float> versions;
        std::string ca_cert;
        std::string server_cert;
        std::string server_key;
    };

    struct DemoConfig {
        std::vector<ServerSettings> servers;
        LogSettings logs;
        MtlsSettings mtls;
    };

    std::ostream& operator<<(std::ostream& os, const ServerSettings& s);
    std::ostream& operator<<(std::ostream& os, const LogSettings& s);
    std::ostream& operator<<(std::ostream& os, const MtlsSettings& s);
    std::ostream& operator<<(std::ostream& os, const DemoConfig& s);
}

#endif // DEMO_CONFIG_H