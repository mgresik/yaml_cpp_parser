#include "demo_config.h"

namespace {
    template<typename T>
    std::ostream& format_vector(std::ostream& os,
                               const std::vector<T>& vec,
                               std::string_view prefix = "[",
                               std::string_view suffix = "]",
                               std::string_view delim = ", ") {
        os << prefix;
        for (std::size_t i = 0; i < vec.size(); ++i) {
            if (i > 0)
                os << delim;
            os << vec[i];
        }
        os << suffix;
        return os;
    }
}

namespace demo {
    std::ostream& operator<<(std::ostream& os, const ServerSettings& s) {
        os << "{protocol: " << s.proto << ", port: " << s.port << "}";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const LogSettings& s) {
        os << "{level: " << s.level << ", folder: '" << s.folder << "'}";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const MtlsSettings& s) {
        os << "enabled: " << (s.use_mtls_auth ? "on" : "off") << ", " << std::endl;
        os << "\ttls: {versions: ";
        format_vector(os, s.versions, "[", "]") << "}" << std::endl;
        os << "\tcertificates: {ca_cert: '" << s.ca_cert << "'";
        os << ", server-cert: '" << s.server_cert << "'";
        os << ", server-key:'" << s.server_key << "'}";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const DemoConfig& s) {
        os << "servers: " << std::endl;
        for (const auto& it : s.servers)
            os << "\t" << it << std::endl;

        os << "logging: " << std::endl;
        os << "\t" << s.logs << std::endl;

        os << "mtls_auth: " << std::endl;
        os << "\t" << s.mtls;

        return os;
    }
}
