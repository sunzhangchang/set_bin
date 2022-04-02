#include <set>
#include <map>
#include <cctype>
#include <string>
#include <fstream>
#include <iostream>
#include <optional>
#include <filesystem>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define IS_WINDOWS "Windows"

#else

#ifdef IS_WINDOWS
#undef IS_WINDOWS
#endif

#endif

using namespace std::string_literals;

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

const string config_path = "./set_bin.cfg";

const std::set<string> executable{
#ifdef IS_WINDOWS
    ".exe"s,
    ".bat"s,
    ".cmd"s,
    ".com"s,
    ".ps1"s
#endif
};

string unquote(string str) {
    if (str.at(0) == '\"' && *str.rbegin() == '\"') {
        return str.substr(1, str.size() - 2);
    }
    return str;
}

string quote(string str) {
    return "\""s + str + "\"";
}

std::filesystem::path operator + (const std::filesystem::path & self, std::filesystem::path other) {
    return unquote(unquote(self.string()) + "\\" + unquote(other.string()));
}

std::filesystem::path operator += (std::filesystem::path & self, std::filesystem::path other) {
    return self = self + other;
}

void set_bin(std::filesystem::path path, std::filesystem::path sys_path) {
    cerr << sys_path << endl;

    string cd = "cd "s + sys_path.string();
    string path_filename = path.filename().string();
    string bat_name = path_filename.erase(path_filename.rfind(path.extension().string()), path_filename.size()) + ".bat";

    // cerr << (sys_path + bat_name).string() << endl;

    std::filesystem::path bat_path{sys_path + bat_name};

    // cerr << bat_path << endl;

    std::ofstream fout{bat_path};

    if (!fout) {
        cerr << "open(create) file "s + quote(bat_path.string()) << endl;
    }

    string output;

    char str[1024];
    sprintf(
        str,
        R"(@echo off
set params=
:param
set str=%1
if "%%str%%"=="" (
    goto end
)
set params=%%params%% %%str%%
shift /0
goto param
:end
if "%%params%%"=="" (
    goto eof
)
:intercept_left
if "%%params:~0,1%%"==" " set "params=%%params:~1%%" & goto intercept_left
:intercept_right
if "%%params:~-1%%"==" " set "params=%%params:~0,-1%%" & goto intercept_right
:eof
%s %%params%%
@echo on)",
quote(path.string()).c_str()
);

    cout << str << endl;

    fout << str << endl;

    // fout << "@echo off" << endl;
    // fout << quote(path.string()) + " %~1" << endl;
    // fout << "@echo in" << endl;
}

std::optional<std::map<string, string>> read_cfg(std::filesystem::path path) {
    if (!std::filesystem::exists(path)) {
        return {};
    }
    std::ifstream fin{path};
    if (!fin) {
        return {};
    }
    std::map<string, string> cfg;
    string key, value;
    bool is_comment = false, is_value = false, is_set_key = true;
    while (!fin.eof()) {
        char c = fin.get();
        switch (c) {
            case '#':
                is_comment = true;
                break;

            case '=':
                is_value = true;
                is_set_key = false;
                break;

            case '\n': case '\r':
                is_value = false;
                break;

            default:
                if (is_value) {
                    if (!is_set_key) {
                        cfg.emplace(key, ""s);
                        is_set_key = true;
                    }
                    value += c;
                } else {
                    if (!value.empty()) {
                        cfg[key] = value;
                        value.clear();
                        key.clear();
                    }
                    key += c;
                }
                break;
        }
    }
    return cfg;
}

bool write_cfg(std::filesystem::path path, const std::map<string, string> & cfg) {
    std::ofstream fout{path};
    if (!fout) {
        return false;
    }
    for (const auto & it : cfg) {
        cout << it.first << '=' << it.second << endl;
        fout << it.first << "=" << it.second << endl;
    }
    return true;
}

int main(int argc, char ** argv) {
#ifdef IS_WINDOWS
    std::filesystem::path path, sys_path;
    std::pair<string, int> kv;
    auto get_path = [&path]() {
        cout << "bin path: ";
        string tmp;
        std::getline(cin, tmp);
        path = tmp;
    };
    auto get_sys_path = [&sys_path]() {
        auto cfg = read_cfg(config_path);
        if (!cfg || cfg.value()["sys_path"s].empty()) {
            cout << "sys path: ";
            string tmp;
            std::getline(cin, tmp);
            sys_path = tmp;
        } else {
            sys_path = cfg.value()["sys_path"s];
        }
    };
    if (argc == 1) {
        get_path();
        get_sys_path();
    } else if (argc == 2) {
        path = argv[1];
        get_sys_path();
    } else {
        path = argv[1];
        sys_path = argv[2];
    }

    cout << "bin path: " << path << endl;
    cout << "sys path: " << sys_path << endl;

    bool is_exists_path = std::filesystem::exists(path);
    if (!is_exists_path) {
        cerr << ("file "s) + quote(path.string()) + (" does not exist") << endl;
    }

    bool is_exists_sys_path = std::filesystem::exists(sys_path);
    if (!is_exists_path) {
        cerr << ("dir "s) + quote(path.string()) + (" does not exist") << endl;
    }

    if ((!is_exists_path) || (!is_exists_sys_path)) {
        exit(0x01);
    }

    write_cfg(config_path, {{"sys_path"s, sys_path.string()}});

    if (!path.has_extension()) {
        cerr << ("file "s) + quote(path.string()) + (" does not have extensions") << endl;
        exit(0x02);
    }

    auto ext = path.extension();
    // cerr << ext << endl;

    if (!executable.count(ext.string())) {
        cerr << ("file \""s) + path.string() + ("\" is not executable") << endl;
        exit(0x03);
    }

    set_bin(path, sys_path);

#endif
    return 0;
}
