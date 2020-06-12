#pragma once

#include <regex>

#include <boost/algorithm/string.hpp>

// using namespace std;

namespace utils {
    std::string fetch_https(std::string, std::string);
    std::string fetch_http(std::string, std::string);
    std::string fetch(std::string);

    bool exists_file(std::string filename);

    std::string read_file(std::string filename);

    void write_file(std::string filename, std::string data);

    std::vector<std::string> read_lines(std::string filename);

    std::string get_uuid();

    bool url_decode(const std::string &in, std::string &out);

    std::vector<std::string> split(const std::string &str, const char primary);

    std::map<std::string, std::string>
    double_split(const std::string &in, const char primary, const char secondary);

    std::map<std::string, std::string> url_to_map(const std::string &in);

    void url_to_map(std::map<std::string, std::string> *mp, const std::string &in);

    int s2i(std::string, int = 0);

    std::string i2s(int, std::string = "");

    double s2d(std::string, double = 0);

    std::string d2s(double, std::string = "");

    long s2l(std::string, long = 0);

    std::string l2s(long, std::string = "");

    std::string password(long);

    std::string join(std::vector<std::string>, std::string);

    long timestamp();

    std::vector<std::string> split2(const std::string &input, const std::string &reText);

    inline bool ends_with(std::string const &value, std::string const &ending) {
        if (ending.size() > value.size())
            return false;
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }

    std::vector<std::tuple<bool, std::string>> tokenize(std::string, std::regex r);

    bool sendmail(std::string from, std::string to, std::string subject, std::string message);

    std::string exec(const char *cmd);
    std::string get_now(std::string);



} // namespace utils

namespace term {

    class Color {
    public:
        int r;
        int g;
        int b;
        bool bg = false;

        friend std::ostream &operator<<(std::ostream &os, const Color &c) {
            if (c.bg) {
                return os << "\033[48:2;" << c.r << ";" << c.g << ";" << c.b << "m";
            } else {
                return os << "\033[38:2;" << c.r << ";" << c.g << ";" << c.b << "m";
            }
        }
    };

    winsize GetSize();
    std::string Normal();

    void IsBold(bool);
    void ShowCursor(bool);
    void Move(int,int);
    void Reset();
    void Clear();

} // namespace color
