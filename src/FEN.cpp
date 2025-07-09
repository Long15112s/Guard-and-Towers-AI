#include "FEN.hpp"
#include <cctype>
#include <vector>

// Splittet s an jedem Sep-Zeichen
static std::vector<std::string> split(const std::string& s, char sep) {
    std::vector<std::string> out;
    size_t start = 0, pos;
    while ((pos = s.find(sep, start)) != std::string::npos) {
        out.push_back(s.substr(start, pos - start));
        start = pos + 1;
    }
    out.push_back(s.substr(start));
    return out;
}

// Zählt, wie viele Felder eine Reihe belegt (Guard=1, Stack=1, Digit=n)
static int countRowFields(const std::string& row) {
    int cnt = 0;
    for (size_t i = 0; i < row.size();) {
        if (i+1 < row.size() && (row.substr(i,2)=="RG" || row.substr(i,2)=="BG")) {
            cnt++; i+=2;
        }
        else if (std::isdigit(row[i])) {
            cnt += row[i]-'0'; i+=1;
        }
        else if (row[i]=='r'||row[i]=='b') {
            cnt++; i++;
            if (i<row.size() && std::isdigit(row[i])) i++;
        }
        else { cnt++; i++; }
    }
    return cnt;
}

bool hasSoldierWithoutHeight(const std::string& placement) {
    for (size_t i=0;i<placement.size();++i) {
        char c=placement[i];
        if ((c=='r'||c=='b') &&
            (i+1>=placement.size() || (!std::isdigit(placement[i+1]) && placement[i+1]!='/')))
            return true;
    }
    return false;
}

std::string padPlacement(const std::string& placement) {
    auto rows = split(placement,'/');
    for (auto& r: rows) {
        if (r.empty()) { r="7"; continue; }
        int miss = 7 - countRowFields(r);
        if (miss>0) r.append(miss,'1');
    }
    std::string out;
    for (size_t i=0;i<rows.size();++i){
        if (i) out.push_back('/');
        out+=rows[i];
    }
    return out;
}

bool validateFENPlacement(const std::string& placement) {
    auto rows = split(placement,'/');
    if (rows.size()!=7) return false;
    for (auto& row0: rows) {
        std::string row = row0.empty() ? "7" : row0;
        int count=0;
        for (size_t i=0;i<row.size();) {
            if (i+1<row.size() && (row.substr(i,2)=="RG"||row.substr(i,2)=="BG")) {
                count+=1; i+=2;
            }
            else if (std::isdigit(row[i])) {
                int d=row[i]-'0';
                if (d<1||d>7) return false;
                count+=d; i+=1;
            }
            else if (row[i]=='r'||row[i]=='b') {
                count+=1; i+=1;
                if (i<row.size()&&std::isdigit(row[i])) i+=1;
            }
            else return false;
            if (count>7) return false;
        }
        if (count!=7) return false;
    }
    return true;
}
