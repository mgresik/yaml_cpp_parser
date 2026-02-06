#include "demo_config_manager.h"
#include "demo_config.h"

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <fstream>
#include <stack>
#include <vector>
#include <numeric>

namespace {
// Вспомогательный класс
// Поддерживает текущий путь в Yaml-документе
// В случае ошибки, будет содержать актуальный путь к месту ошибки
class PathTracker {
public:
    PathTracker(const std::string& delim) : delim_{delim} {}

    // Нужно вызывать по мере "погружения" в Yaml-дерево
    void enter(const std::string& node_name) { 
        path_.push(node_name); 
    }

    // Нужно вызывать по мере "всплытия" в Yaml-дереве
    void leave() { path_.pop(); }

    // Альтернатива вызову подряд leave() и enter()
    void leave_and_enter(std::string_view node_name) { 
        path_.top() = node_name; 
    }

    // Возвращает имя текущего узла (без полного пути)
    const std::string& current_node() const { return path_.top(); }

    // Из внутреннего стека создает и возвращает текущий путь в виде строки
    std::string path() const {
        // Получение копии стека (чтоб не нарушить инварианты класса)
        auto path = path_;

        // Опустошение стека в вектор
        std::vector<std::string> vec;
        vec.reserve(path.size());
        while (!path.empty()) {
            vec.push_back(path.top());
            path.pop();
        }

        // Восстановление порядка узлов в Yaml-пути
        std::reverse(vec.begin(), vec.end());

        // Лямбдя для склеивания названий узлов в yaml-путь
        auto acc_fn = [&](const std::string& acc, const std::string& str) {
            return acc + (str.empty() ? "" : delim_ + str);
        };

        return std::accumulate(vec.begin(), vec.end(), std::string(), acc_fn);
    }

private:
    // Символ разделителя в пути, важно, чтобы не был ключевым словом YAML
    std::string delim_;

    // Стек отлично подходит для организации поддержки текущего пути в Yaml-дереве
    std::stack<std::string> path_;
};

    using namespace demo;

    // Вспомогательная функция
    // Формирует для пользователя доступную информацию об ошибке
    template <typename T>
    std::string gen_save_err_msg(std::string_view source, const T& ex)
    {
        std::ostringstream oss;
        oss << "An error occurred while saving settings to [" << source << "]\n";
        oss << "Error details: " << ex.what();
        return oss.str();
    };


    // Вспомогательная функция
    // Формирует для пользователя доступную информацию об ошибке
    template <typename T>
    std::string gen_err_msg(std::string_view source, std::string_view path, const T& ex)
    {
        std::ostringstream oss;
        oss << "An error occurred while loading settings from [" << source << "]\n";
        oss << "Yaml document path: [" << path << "]\n";
        oss << "Error details: " << ex.what();
        return oss.str();
    };

    // Вспомогательная функция
    // Формирует в виде строки данные о месте ошибки
    std::string mark_to_string(const YAML::Mark& mark) {
        if (!mark.is_null()) {
            std::ostringstream oss;
            oss << "(line: " << std::to_string(mark.line) << ", ";
            oss << "column: " << std::to_string(mark.column) << ", ";
            oss << "position: " << std::to_string(mark.pos) << ")";
            return oss.str();
        }
        return {};
    }

    // Генерирует исключение об отсутствии узла
    // Обогащает сообщение об ошибки информацией о месте ошибки (если доступно)
    void throw_field_not_found(std::string_view field, YAML::Mark mark)
    {
        std::ostringstream oss;
        oss << "Required <" << field << "> field not found"
            << mark_to_string(mark);
        throw std::runtime_error(oss.str());
    }

    // Генерирует исключение об отсутствии ненулевого значения у узла
    // Обогащает сообщение об ошибки информацией о месте ошибки (если доступно)
    void throw_field_empty(std::string_view field, YAML::Mark mark)
    {
        std::ostringstream oss;
        oss << "Required <" << field << "> field is empty"
            << mark_to_string(mark);
        throw std::runtime_error(oss.str());
    }

    // Генерирует исключение об отсутствии узла
    void throw_node_not_found(std::string_view path) {
        std::ostringstream oss;
        oss << "Required section: [" << path << "] does not exists";
        throw std::runtime_error(oss.str());
    }

    // Генерирует исключение об отсутствии ненулевого значения у узла
    void throw_node_empty(std::string_view path) {
        std::ostringstream oss;
        oss << "Required section: [" << path << "] is empty";
        throw std::runtime_error(oss.str());
    }

    // Функция проверяет существует ли узел и содержит ли он ненулевое значение
    // Если обе проверки не проходят, то генерируется соответсвующее исключение
    void check_node(const YAML::Node& parent, PathTracker& path, const std::string& node_name)
    {
        path.enter(node_name);

        const auto& node = parent[node_name];

        if (!node.IsDefined())
            throw_node_not_found(node_name);

        if (node.IsNull())
            throw_node_empty(node_name);

        path.leave();
    }

    // Функция проверяет существует ли узел и содержит ли он ненулевое значение.
    // Если обе проверки не прошли, то генерируется соответствующее исключение.
    // При успешности проверок, попытка считать значение узла в поле field.
    template <typename T>
    void check_and_get(const YAML::Node& root, PathTracker& path, const std::string& node_name, T& field)
    {
        path.enter(node_name);

        const auto& node = root[node_name];

        if (!node.IsDefined())
            throw_field_not_found(path.current_node(), root.Mark());

        if (node.IsNull())
            throw_field_empty(path.current_node(), root.Mark());

        field = node.as<T>();

        path.leave();
    }

    // Функция проверки одного и более узлов Yaml-дерева перед обработкой 
    // их значений
    void check_nodes(const YAML::Node& root,
                     PathTracker& path,
                     const std::vector<std::string>& sections)
    {
        for (const auto& section : sections)
            check_node(root, path, section);
    }

    // Обработка секции конфига /servers
    void read_servers_section(const YAML::Node& root, PathTracker& path, DemoConfig& cfg)
    {
        path.enter("servers");
        for (const auto& node : root) {
            ServerSettings settings;
            check_and_get(node, path, "protocol", settings.proto);
            check_and_get(node, path, "port", settings.port);
            cfg.servers.emplace_back(std::move(settings));
        }
        path.leave();
    }

    // Обработка секции конфига /logging
    void read_logging_section(const YAML::Node& root, PathTracker& path, DemoConfig& cfg)
    {
        path.enter("logging");
        check_and_get(root, path, "level", cfg.logs.level);
        check_and_get(root, path, "folder", cfg.logs.folder);
        path.leave();
    }

    // Обработка секции конфига /mtls_auth/tls/versions
    void read_mtls_auth_tls(const YAML::Node& root, PathTracker& path, DemoConfig& cfg)
    {
        path.enter("tls");
        check_and_get(root, path, "versions", cfg.mtls.versions);
        path.leave();
    }

    // Обработка секции конфига /mtls_auth/certificates
    void read_mtls_auth_certificates(const YAML::Node& root, PathTracker& path, DemoConfig& cfg)
    {
        path.enter("certificates");
        check_and_get(root, path, "ca_cert", cfg.mtls.ca_cert);
        check_and_get(root, path, "server_cert", cfg.mtls.server_cert);
        check_and_get(root, path, "private_key", cfg.mtls.server_key);
        path.leave();
    }

    // Обработка секции конфига /mtls_auth
    void read_mtls_auth_section(const YAML::Node& root, PathTracker& path, DemoConfig& cfg)
    {
        path.enter("mtls_auth");
        check_nodes(root, path, {"enabled"});
        check_and_get(root, path, "enabled", cfg.mtls.use_mtls_auth);

        if (cfg.mtls.use_mtls_auth) {
            check_nodes(root, path, {"tls", "certificates"});
            read_mtls_auth_tls(root["tls"], path, cfg);
            read_mtls_auth_certificates(root["certificates"], path, cfg);
        }
    }

    // Обработка конфига начиная с корня yaml-дерева /
    bool load_from_yaml_node(const YAML::Node& root, PathTracker& path, DemoConfig& cfg)
    {
        check_nodes(root, path, {"servers", "logging", "mtls_auth"});

        read_servers_section(root["servers"], path, cfg);
        read_logging_section(root["logging"], path, cfg);
        read_mtls_auth_section(root["mtls_auth"], path, cfg);

        return true;
    }

    template <typename F>
    bool load_node(std::string_view source, DemoConfig& cfg, F load_yaml_fn)
    {
        // Названия узлов конфига этого примера не содержат символ '/' 
        // Поэтому можно его задать в качестве разделителя в Yaml-пути.
        PathTracker path("/");

        try {
            const YAML::Node root_node = load_yaml_fn();

            DemoConfig settings{};
            load_from_yaml_node(root_node, path, settings);
            cfg = settings;

            return true;
        }
        catch (const YAML::BadFile& ex) {
            std::cerr << gen_err_msg(source, path.path(), ex);
        }
        catch (const YAML::ParserException& ex) {
            std::cerr << gen_err_msg(source, path.path(), ex);
        }
        catch (const YAML::BadConversion& ex) {
            std::cerr << gen_err_msg(source, path.path(), ex);
        }
        catch (const YAML::Exception& ex) {
            std::cerr << gen_err_msg(source, path.path(), ex);
        }
        catch (const std::exception& ex) {
            std::cerr << gen_err_msg(source, path.path(), ex);
        }
        catch (...) {
            std::cerr << gen_err_msg(source, path.path(), 
                                     std::runtime_error{"Unknown error"});
        }

        std::cerr << std::endl;

        return false;
    }

    /* Запись секции servers
    servers:
        - protocol: socks5
          port: 1080
        - protocol: http(s)
          port: 2080
    */
    void write_to_yaml_stream(YAML::Emitter& out, const std::vector<ServerSettings>& servers) 
    {
        using namespace YAML;
        out << Key << "servers";
        out.SetLocalIndent(2);
        out << BeginSeq;
            for (const auto& [proto, port] : servers)
                out << BeginMap << "protocol" << proto << "port" << port << EndMap;
        out << EndSeq;
        out << Newline << Newline;
    }

    /* Запись секции logging
    logging:
        level: debug
        folder: './log'
    */
    void write_to_yaml_stream(YAML::Emitter& out, const LogSettings& logs)
    {
        using namespace YAML;
        out << Key << "logging" << BeginMap;
            out << "level" << logs.level;
            out << "folder" << SingleQuoted << logs.folder;
            out << EndMap;
        out << Newline << Newline;
    }


    /* Запись секции mtls_auth
    mtls_auth:
        enabled: on
        tls:
            versions: [1.2, 1.3]
        certificates:
            ca_cert: ca.pem
            server_cert: server-cert.pem
            private_key: server-key.pem
    */
    void write_to_yaml_stream(YAML::Emitter& out, const MtlsSettings& mtls)
    {
        using namespace YAML;
        out << Key << "mtls_auth" << BeginMap;
            // OnOffBool - включает запись on|off вместо true|false
            out << "enabled" << OnOffBool << mtls.use_mtls_auth;
            out << "tls";
            out << BeginMap << "versions" << Flow << FloatPrecision(3);
                // Автоматически сработает перегрузка для std::vector<float>
                // Поэтому можно явно не указывать BeginSec|EndSeq
                out << mtls.versions;
            out << EndMap;
            out << "certificates" << BeginMap;
                out << "ca_cert" << mtls.ca_cert;
                out << "server_cert" << mtls.server_cert;
                out << "private_key" << mtls.server_key;
            out << EndMap;
        out << EndMap;
        out << Newline;
    }

    // Функция из структуры с конфигом формирует и возвращает строку
    // с YAML представлением конфига
    std::string to_string(const DemoConfig& cfg)
    {
        YAML::Emitter out;
        out.SetIndent(4); // Задаем отступы 4 символа

        out << YAML::BeginMap;
        write_to_yaml_stream(out, cfg.servers);
        write_to_yaml_stream(out, cfg.logs);
        write_to_yaml_stream(out, cfg.mtls);
        out << YAML::EndMap;

        // Если были какие-либо ошибки при записи, информируем пользователя
        if (!out.good())
            throw std::runtime_error(out.GetLastError());

        return out.c_str();
    }

    template <typename F>
    bool save_to_yaml(std::string_view source, F save_yaml_fn)
    {
        try {
            return save_yaml_fn();
        }
        catch (const YAML::Exception& ex) {
            std::cerr << gen_save_err_msg(source, ex);
        }
        catch (const std::exception& ex) {
            std::cerr << gen_save_err_msg(source, ex);
        }
        catch (...) {
            std::cerr << gen_save_err_msg(source, std::runtime_error{"Unknown error"});
        }

        return false;
    }
}

namespace demo {
    bool DemoConfigManager::load_from_file(const std::string& path, DemoConfig& cfg)
    {
        return is_loaded_ = load_node(path, cfg, [&path]() { return YAML::LoadFile(path); });
    }

    bool DemoConfigManager::load_from_string(const std::string& str, DemoConfig& cfg)
    {
        return is_loaded_ = load_node("string", cfg, [&str]() { return YAML::Load(str); });
    }

    bool DemoConfigManager::save_to_file(const std::string& path, const DemoConfig& cfg)
    {
        auto save_yaml_fn = [&path, &cfg]() {
            std::ofstream ofs(path, std::ios::trunc);
            if (!ofs.is_open())
                throw std::runtime_error{"File open error"};
            ofs << to_string(cfg);
            if (!ofs.good())
                throw std::runtime_error{"File write error"};
            return true;
        };

        return save_to_yaml(path, save_yaml_fn);
    }

    bool DemoConfigManager::save_to_string(std::string& out_str, const DemoConfig& cfg)
    {
        auto save_yaml_fn = [&out_str, &cfg]() {
            out_str = to_string(cfg);
            return true;
        };

        return save_to_yaml(out_str, save_yaml_fn);
    }
}