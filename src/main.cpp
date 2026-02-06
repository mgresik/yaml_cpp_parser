#include <demo_config.h>
#include <demo_config_manager.h>

#include <iostream>

const std::string g_yaml_cfg_string{
    "servers:\n"
    "    - protocol: socks5\n"
    "      port: 1080\n"
    "    - protocol: http(s)\n"
    "      port: 2080\n\n"
    "logging:\n"
    "    level: debug\n"
    "    folder: './log'\n\n"
    "mtls_auth:\n"
    "    enabled: on\n"
    "    tls:\n"
    "        versions: [1.2, 1.3]\n"
    "    certificates:\n"
    "        ca_cert: ca.pem\n"
    "        server_cert: server-cert.pem\n"
    "        private_key: server-key.pem\n"
};
const std::string g_yaml_cfg_file_path{"config.yml"};
const std::string g_yaml_cfg_out_file_path{"config_out.yml"};

int main()
{
    demo::DemoConfigManager cfg;

    // Чтение YAML-настроек из строки
    demo::DemoConfig settingsFromString;
    std::cout << "=== Read from string ===\n";
    cfg.load_from_string(g_yaml_cfg_string, settingsFromString);
    if (cfg.is_loaded())
        std::cout << settingsFromString << std::endl;

    // Чтение YAML-настроек из файла
    demo::DemoConfig settingsFromFile;
    std::cout << "=== Read from file ===\n";
    cfg.load_from_file(g_yaml_cfg_file_path, settingsFromFile);
    if (cfg.is_loaded())
        std::cout << settingsFromFile << std::endl;

    std::cout << std::endl;

    // Запись YAML-настроек в файл
    if (cfg.save_to_file(g_yaml_cfg_out_file_path, settingsFromString))
        std::cout << "Saving config to " << g_yaml_cfg_out_file_path << " was successful";
    std::cout << std::endl;

    // Запись YAML-настроек в строку
    std::string yaml_cfg_string;
    if (cfg.save_to_string(yaml_cfg_string, settingsFromString))
        std::cout << "Saving config to yaml string was successful";
    std::cout << std::endl;

    // Сравниваем сформированную строку с эталоном
    if (yaml_cfg_string == g_yaml_cfg_string)
        std::cout << "The resulting YAML string is equivalent to the original one:\n\n";

    std::cout << yaml_cfg_string;

    return 0;
}