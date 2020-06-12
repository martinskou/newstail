#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <regex>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

//#include <boost/certify/extensions.hpp>
//#include <boost/certify/https_verification.hpp>

#include "utils.hpp"


using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>



namespace utils {

    bool exists_file(std::string filename) {
        return std::filesystem::exists(filename);
    }

    std::string read_file(std::string filename) {
        std::ifstream inf;
        inf.open(filename);
        std::stringstream buffer;
        buffer << inf.rdbuf();
        inf.close();
        return buffer.str();
    }

    void write_file(std::string filename, std::string data) {
        std::ofstream onf;
        onf.open(filename);
        onf << data;
        onf.close();
    }


    std::vector<std::string> read_lines(std::string filename) {
        std::vector<std::string> res;
        std::ifstream inputFile(filename);
        std::string line;
        while (std::getline(inputFile, line)) {
            res.push_back(line);
        }
        return res;
    }

    std::string password(long len) {
        static std::random_device dev;
        static std::mt19937 rng(dev());

//        const char *v = "23456789abcdefghjkmnpqrstuvwxyzABCDEFGHJKMNPQRSTUVWXYZ";
        const char *v = "23456789abcdefghjkmnpqrstuvwxyz";

        std::uniform_int_distribution<int> dist(0, strlen(v) - 1);

        std::string res;
        for (int i = 0; i < len; i++) {
            res += v[dist(rng)];
        }
        return res;
    }

    std::string get_uuid() {
        static std::random_device dev;
        static std::mt19937 rng(dev());

        std::uniform_int_distribution<int> dist(0, 15);

        const char *v = "0123456789abcdef";
        const bool dash[] = {0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0};

        std::string res;
        for (int i = 0; i < 16; i++) {
            if (dash[i])
                res += "-";
            res += v[dist(rng)];
            res += v[dist(rng)];
        }
        return res;
    }

    bool url_decode(const std::string &in, std::string &out) {
        out.clear();
        out.reserve(in.size());
        for (std::size_t i = 0; i < in.size(); ++i) {
            if (in[i] == '%') {
                if (i + 3 <= in.size()) {
                    int value = 0;
                    std::istringstream is(in.substr(i + 1, 2));
                    if (is >> std::hex >> value) {
                        out += static_cast<char>(value);
                        i += 2;
                    } else {
                        return false;
                    }
                } else {
                    return false;
                }
            } else if (in[i] == '+') {
                out += ' ';
            } else {
                out += in[i];
            }
        }
        return true;
    }


    std::vector<std::string> split(const std::string &str, const char primary) {
        std::vector<std::string> parts;
        boost::split(parts, str, [primary](char c) { return c == primary; });
        return parts;
    }

    std::map<std::string, std::string>
    double_split(const std::string &in, const char primary, const char secondary) {
        std::map<std::string, std::string> rm;

        std::vector<std::string> parts;
        boost::split(parts, in, [primary](char c) { return c == primary; });
        for (auto part : parts) {

            std::vector<std::string> innerparts;
            boost::split(innerparts, part,
                         [secondary](char c) { return c == secondary; });

            if (innerparts.size() > 1) {
                auto a = innerparts[0];
                //      auto b = boost::join(
                //          std::vector<std::string>(innerparts.begin() + 1,
                //          innerparts.end()),
                //          "&");
                auto b = innerparts[1];
                std::string c;
                url_decode(b, c);
                rm.insert(std::pair(a, c));
            }
        }
        return rm;
    }

    std::map<std::string, std::string> url_to_map(const std::string &in) {
        return double_split(in, '&', '=');
    }

    void url_to_map(std::map<std::string, std::string> *mp, const std::string &in) {
        auto x = double_split(in, '&', '=');
        mp->merge(x);
    }

    int s2i(std::string s, int d) {
        try {
            auto r = std::stoi(s);
            return r;
        } catch (...) {
            return d;
        }
    }

    std::string i2s(int i, std::string d) { return std::to_string(i); }

    double s2d(std::string s, double d) {
        try {
            auto r = std::stod(s);
            return r;
        } catch (...) {
            return d;
        }
    }

    std::string d2s(double i, std::string d) {
        char buf[20];
        snprintf(buf, 20, "%0.2f", i);
        return std::string(buf);
//        return std::to_string(i);
    }

    long s2l(std::string s, long d) {
        try {
            auto r = std::stol(s);
            return r;
        } catch (...) {
            return d;
        }
    }

    std::string l2s(long i, std::string d) { return std::to_string(i); }

    std::string join(std::vector<std::string> vec, std::string sep) {
        std::stringstream joinedValues;
        for (auto value : vec) {
            joinedValues << value << ",";
        }
        std::string result = joinedValues.str();
        if (result.size() > 0) {
            result.pop_back();
        }
        return result;
    }

    template<class T, class A>
    T join(const A &begin, const A &end, const T &t) {
        T result;
        for (A it = begin; it != end; it++) {
            if (!result.empty())
                result.append(t);
            result.append(*it);
        }
        return result;
    }


    long timestamp() {
        auto time = std::chrono::system_clock::now().time_since_epoch();
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(time).count();
        return secs;
    }

    std::vector<std::string> split2(const std::string &input, const std::string &reText) {
        std::regex re(reText);
        std::sregex_token_iterator first{input.begin(), input.end(), re, 0}, last;
        return {first, last};
    }


    std::vector<std::tuple<bool, std::string>> tokenize(std::string text, std::regex r) {
        std::vector<std::tuple<bool, std::string>> result;

        std::sregex_token_iterator iter(text.begin(),
                                        text.end(),
                                        r, {-1, 1}); // -1=inbetween, 0=full match, 1=sub match

        std::sregex_token_iterator end;
        bool is_token = false;

        for (; iter != end; ++iter) {
            std::string item = *iter;
            boost::algorithm::trim(item);
            result.push_back(std::make_tuple(is_token, item));
            is_token = !is_token;
        }

        return result;
    }


    std::string exec(const char *cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }

    std::string get_now(std::string fmt) {
        std::time_t tnow = std::time(0);   // get time now
        std::tm *ptm = std::localtime(&tnow);
        char st[32];
        std::strftime(st, 32, fmt.c_str(), ptm);
        return std::string(st);
    }

    bool verify_certificate(bool pverified_ok, ssl::verify_context &ctx) {
        char subject_name[256];
        X509 *cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
        X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
//        std::cout << "Verifying " << subject_name << std::endl;
        return true; // pverified_ok;
    }

    std::string fetch_https(std::string _host, std::string _target) {
        const char *host = _host.c_str();
        const char *target = _target.c_str();
        const char *port = "443";
        int version = 11;

        std::stringstream body;

        boost::asio::io_context ioc;
        ssl::context ctx{ssl::context::sslv23_client};

        ctx.set_verify_mode(ssl::context::verify_peer);
        ctx.set_verify_callback(verify_certificate);

        tcp::resolver resolver{ioc};
        ssl::stream<tcp::socket> stream{ioc, ctx};

        if (!SSL_set_tlsext_host_name(stream.native_handle(), host)) {
            boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            throw boost::system::system_error{ec};
        }

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        boost::asio::connect(stream.next_layer(), results.begin(), results.end());

        try {
            // Perform the SSL handshake
            stream.handshake(ssl::stream_base::client);


            // Set up an HTTP GET request message
            http::request<http::string_body> req{http::verb::get, target, version};
            req.set(http::field::host, host);
            req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
//            std::cout << req << std::endl;

            // Send the HTTP request to the remote host
            http::write(stream, req);

            // This buffer is used for reading and must be persisted
            boost::beast::flat_buffer buffer;

            // Declare a container to hold the response
            http::response<http::dynamic_body> res;

            // Receive the HTTP response
            http::read(stream, buffer, res);

            // Write the message to standard out
//            std::cout << res << std::endl;

            body << boost::beast::buffers_to_string(res.body().data());

//            std::cout << "size " << body.str().size() << std::endl;

        } catch (const std::exception &e) {
            std::cout << "Exception error: " << e.what() << std::endl;
        }

//        std::cout << "A" << std::endl;

        // Gracefully close the stream, HANGS SOMETIME, use async with timeout
//        boost::system::error_code ec;
//        stream.shutdown(ec);

//        std::cout << "B" << std::endl;

        /*
        if (ec == boost::asio::error::eof) {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec.assign(0, ec.category());
        }
        if (ec) {
            throw boost::system::system_error{ec};
        }
        */
//        std::cout << "done" << std::endl;
        return body.str();
    }


    std::string fetch_http(std::string _host, std::string _target) {
        const char *host = _host.c_str();
        const char *target = _target.c_str();
        const char *port = "80";
        int version = 11;

        std::stringstream body;

        // The io_context is required for all I/O
        boost::asio::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver{ioc};
        tcp::socket socket{ioc};

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        boost::asio::connect(socket, results.begin(), results.end());

        // Set up an HTTP GET request message
        http::request<http::string_body> req{http::verb::get, target, version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Send the HTTP request to the remote host
        http::write(socket, req);


        // This buffer is used for reading and must be persisted
        boost::beast::flat_buffer buffer;

        // Declare a container to hold the response
        http::response<http::dynamic_body> res;

        // Receive the HTTP response
        http::read(socket, buffer, res);


        body << boost::beast::buffers_to_string(res.body().data());

        // Gracefully close the socket
        boost::system::error_code ec;
        socket.shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes
        // so don't bother reporting it.
        //
        if (ec && ec != boost::system::errc::not_connected)
            throw boost::system::system_error{ec};

        return body.str();
    }

    std::string fetch(std::string url) {
        //        std::string xml = utils::fetch_https("jyllands-posten.dk", "/?service=rssfeed&submode=topnyheder");
        std::vector<std::string> ua = split(url, '/');

        std::string u_host = ua[2];
        std::string u_path = "/" + join(ua.begin() + 3, ua.end(), std::string("/"));

        if (ua[0].compare("https:") == 0) {
//            std::cout << "SSL" << std::endl;
            return fetch_https(u_host, u_path);
        }

        if (ua[0].compare("http:") == 0) {
//            std::cout << "NOSSL" << std::endl;
            return fetch_http(u_host, u_path);
        }

        return "";
    }

} // namespace utils

void term::Clear() {
    std::cout << "\033[2J";
    std::cout << "\033[3J";
    Move(1, 1);
}

winsize term::GetSize() {
    struct winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) < 0) {
        fprintf(stderr, "Error obtaining terminal size.\n");
        exit(EXIT_FAILURE);
    }
    return size;
}

std::string term::Normal() {
    return "\033[0m";
}

void term::Reset() {
    std::cout << "\033c";
}

// https://en.wikipedia.org/wiki/ANSI_escape_code
void term::Move(int y, int x) {
    std::cout << "\033[" << x << ";" << y << "H";
}

void term::ShowCursor(bool show) {
    if (show) {
        std::cout << "\033[?25h";
    } else {
        std::cout << "\033[?25l";
    }
}

void term::IsBold(bool bold) {
    if (bold) {
        std::cout << "\033[1m";
    } else {
        std::cout << "\033[0m";
    }
}
