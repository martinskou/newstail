#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <time.h>
#include <iomanip>

#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
#include <unistd.h> // for STDOUT_FILENO

#include "boost/date_time/gregorian/gregorian.hpp"

#include "framework/utils.hpp"

//#include "rapidxml/rapidxml.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>


namespace rapidxml = boost::property_tree::detail::rapidxml;


/*
 * Config classes
 */


class Feed {
public:
    std::string tag;
    std::string url;
    term::Color color;
};

class Config {
public:
    int refresh;
    int boldnewsfor;
    term::Color timecolor;
    std::vector<Feed> feeds;

    void Load(boost::property_tree::ptree pt) {

        refresh=utils::s2i(pt.get_child("RefreshSecs").data(),60*10);
        boldnewsfor=utils::s2i(pt.get_child("BoldForSecs").data(),60*5);

        for (auto x :  pt.get_child("Feeds")) {
            Feed f;
            boost::property_tree::ptree t = x.second.get_child("Tag");
            boost::property_tree::ptree u = x.second.get_child("URL");
            boost::property_tree::ptree c = x.second.get_child("Color");

            f.tag = t.data();
            f.url = u.data();
            auto cp = c.begin();
            f.color.r = utils::s2i(cp->second.data(), 0);
            cp++;
            f.color.g = utils::s2i(cp->second.data(), 0);
            cp++;
            f.color.b = utils::s2i(cp->second.data(), 0);

            feeds.emplace_back(f);
        }

        auto c=pt.get_child("Time").get_child("Color");
        auto cp = c.begin();
        timecolor.r = utils::s2i(cp->second.data(), 0);
        cp++;
        timecolor.g = utils::s2i(cp->second.data(), 0);
        cp++;
        timecolor.b = utils::s2i(cp->second.data(), 0);

    }
};

/*
 * RSS parsing
 */

class item {
public:
    std::string title;
    std::string link;
    long timestamp;
    std::string tag = "?";
    term::Color color;
    long itimestamp;

    int age() {
        return  std::time(0)-itimestamp;
    }
    bool operator<(const item &i) const { return timestamp < i.timestamp; }
};

bool itemContains(std::vector<item> &items ,std::string title, std::string url) {
    for (auto i : items) {
        if (i.title==title && i.link==url)
            return true;
    }
    return false;
}

void parse_children(std::vector<item> &items, rapidxml::xml_node<> *root, std::string tag, term::Color color) {
    for (rapidxml::xml_node<> *node = root->first_node(); node; node = node->next_sibling()) {

//        std::cout << node->name() << std::endl;

        if (std::string(node->name()) == "item") {

            rapidxml::xml_node<> *n_t = node->first_node("title");
            rapidxml::xml_node<> *n_l = node->first_node("link");
            rapidxml::xml_node<> *n_d1 = node->first_node("dc:date");
            rapidxml::xml_node<> *n_d2 = node->first_node("pubDate");


            if (n_t && n_l && (n_d1 || n_d2)) {
                std::string t = n_t->value();
                std::string l = n_l->value();

                if (t.size() == 0) {
                    n_t = n_t->first_node(); // check for data in CDATA child element
                    if (n_t) {
                        t = n_t->value();
                    } else {
                        t = "?";
                    }
                }

                if (itemContains(items,t,l)) break;

                std::time_t tt;
                if (n_d2) {
                    std::string d = n_d2->value();

                    std::string format("%a, %d %b %Y %H:%M:%S %Z");

                    std::tm tm;
                    strptime(d.c_str(), format.c_str(), &tm);
                    tt = mktime(&tm);

                    std::tm *ptm = std::localtime(&tt);
                    char buffer[32];
                    std::strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", ptm);
                }

                if (n_d1) {
                    std::string d = n_d1->value();
                    // 2020-06-11T18:45:00Z
                    std::string format("%Y-%m-%dT%H:%M:%S%Z");

                    std::tm tm;
                    strptime(d.c_str(), format.c_str(), &tm);
                    tt = mktime(&tm);

                    std::tm *ptm = std::localtime(&tt);
                    char buffer[32];
                    std::strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", ptm);
                }

                items.emplace_back(item{t, l, tt, tag, color, std::time(0)});

            }
        } else {
            //       std::cout << "<" << node->name() << ">" << std::endl;

        }

        parse_children(items, node, tag, color);

    }
}

void parse_rss(std::vector<item> &items, std::string xml, std::string tag, term::Color color) {

//    std::cout << xml << std::endl;

    std::vector<char> xml_copy(xml.begin(), xml.end());
    xml_copy.push_back('\0');

    rapidxml::xml_document<> doc;

    try {
        doc.parse<rapidxml::parse_default | rapidxml::parse_trim_whitespace>(&xml_copy[0]);
        rapidxml::xml_node<> *root_node = doc.first_node();
        parse_children(items, root_node, tag, color);
    } catch (const std::exception &e) {
        std::cout << "Exception error: " << e.what() << std::endl;
        std::cout << "Where: " << tag << std::endl;
    }

}

/*
 * Output functions
 */

void print_middle(struct winsize size, std::string txt) {
    int l = txt.size();
    int y = size.ws_row / 2;
    int x = size.ws_col / 2 - l / 2;
    term::Move(x, y);
    std::cout << txt << std::endl;
}


void print_highlight(struct winsize size, std::string txt) {
    int l = txt.size();

    std::string pad=std::string(((size.ws_col-l) / 2)-2,'=');
    txt=pad+" ["+txt+"] "+pad;

    std::cout << txt << std::endl;
}


void print_link(std::string url, std::string txt) {
    std::cout << "\e]8;;" << url << "\e\\" << txt << "\e]8;;\e\\";
}

std::string trim(std::string txt, int maxlen) {
    if ((int)txt.size()>maxlen) {
        return txt.substr(0,maxlen);
    }
    return txt;
}

void print_item(struct winsize size, int line, item i,Config config) {

    int top_offset=2;

    std::tm *ptm = std::localtime(&i.timestamp);
    char st[32];
    std::strftime(st, 32, "%H:%M", ptm);

    term::Move(1, line + top_offset);
    std::cout << i.color << std::setw(3) << i.tag << term::Normal() << ": ";

    term::Move(6, line + top_offset);
//    print_link(i.link, trim(i.title+" "+utils::i2s(i.age()) ,size.ws_col-13));
    print_link(i.link, trim(i.title, size.ws_col-13));

    term::Move(size.ws_col - 5, line + top_offset);
    term::IsBold(i.age()<config.boldnewsfor);
    std::cout << config.timecolor << st << term::Normal() << std::endl;
    term::IsBold(false);
}


void reload(std::vector<item> &items, Config config) {
    struct winsize size = term::GetSize();

    for (auto f : config.feeds) {
        std::string xml = utils::fetch(f.url);
        if (xml.size() != 0) {
            parse_rss(items, xml, f.tag, f.color);
        }
    }

    std::sort(items.begin(), items.end());

    term::Clear();

    int max_posts = size.ws_row - 2;
    int min_post = std::max(0, (int) items.size() - max_posts);

    for (int i = min_post; i < (int)items.size(); i++) {
        print_item(size, i - min_post, items[i],config);
    }

    std::string lt = utils::get_now("%H:%M:%S");

    term::Move(1, 1);
    print_highlight(size,"Updated: "+lt+" / "+utils::i2s(items.size())+" items "+" / "+utils::i2s(config.feeds.size())+" feeds");

}


int main(int argc, char *argv[]) {

    setlocale(LC_CTYPE, "");

    boost::property_tree::ptree ptconfig;
    boost::property_tree::read_json("config.json", ptconfig);
    write_json(std::cout, ptconfig);

    Config c;
    c.Load(ptconfig);

    term::Clear();

    struct winsize size = term::GetSize();
    print_middle(size, "Loading news....");
    term::ShowCursor(false);

    std::vector<item> items{};
    while (true) {
        reload(items, c);
        sleep(c.refresh);
    }

    term::Reset();

}