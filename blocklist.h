#pragma once
#include <unordered_set>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

class Blocklist {
    private:
        std::unordered_set<std::string> domains_;

        static void normalize(std::string& s);
    public:
        bool load(const std::string& path);
        bool isBlocked(std::string domain) const;
};

bool Blocklist::load(const std::string& path){
    std::ifstream file(path);
    if (!file.is_open()){
        std::cerr << "Cannot open blocklist: " << path << std::endl;
        return false;
    }

    std::string line;
    size_t count = 0;
    while (std::getline(file, line)){
        auto hash = line.find('#');
        if (hash != std::string::npos) line = line.substr(0, hash);
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::string first, second;
        ss >> first;
        ss >> second;

        std::string& domain = second.empty() ? first : second;
        normalize(domain);
        if (!domain.empty()){
            domains_.insert(domain);
            ++count;
        }
    }

    std::cout << "Blocklist loaded: " << count << " entries from " << path << std::endl;
    return true;
}

