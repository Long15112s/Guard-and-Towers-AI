#include "Board.hpp"
#include <iostream>
#include <sstream>
#include <cctype>

Board::Board() {
    height.fill(0);
    guardBB[0]=guardBB[1]=0;
    stackBB[0]=stackBB[1]=0;
    occAll=0;
    side=Color::Red;
}

bool Board::loadFEN(const std::string& fen) {
    std::istringstream in(fen);
    std::string placement, sideStr;
    if (!(in>>placement>>sideStr)) return false;

    // Vorab‑Checks via FEN‑Utils
    if (hasSoldierWithoutHeight(placement)) return false;
    auto pad = padPlacement(placement);
    if (!validateFENPlacement(pad)) return false;

    // reset
    height.fill(0);
    guardBB[0]=guardBB[1]=0;
    stackBB[0]=stackBB[1]=0;
    occAll=0;

    // parse rows
    int rank=6; size_t pos=0;
    while (true) {
        auto slash = pad.find('/',pos);
        std::string row = (slash==std::string::npos ? pad.substr(pos)
                                                    : pad.substr(pos,slash-pos));
        if (!parsePieceRow(row,rank--)) return false;
        if (slash==std::string::npos) break;
        pos = slash+1;
    }
    // side
    if      (sideStr=="r") side=Color::Red;
    else if (sideStr=="b") side=Color::Blue;
    else return false;
    return true;
}

bool Board::parsePieceRow(const std::string& row,int rank) {
    int file=0;
    for (size_t i=0;i<row.size();++i) {
        char c=row[i];
        if (std::isdigit(c)) {
            file += c-'0';
        } else {
            if (file>=7) return false;
            int sq=rank*7+file;
            if ((c=='r'||c=='b') && i+1<row.size() && std::isdigit(row[i+1])) {
                height[sq]=row[++i]-'0';
                int col = (c=='r'?0:1);
                stackBB[col] |= 1ULL<<sq;
                occAll       |= 1ULL<<sq;
            }
            else if ((c=='R'||c=='B') && i+1<row.size() && row[i+1]=='G') {
                int col=(c=='R'?0:1);
                guardBB[col]|=1ULL<<sq;
                occAll     |=1ULL<<sq;
                height[sq]=255;
                ++i;
            }
            else return false;
            ++file;
        }
    }
    return file==7;
}

void Board::print() const {
    for (int r=6;r>=0;--r) {
        std::cout<<r+1<<" ";
        for (int f=0;f<7;++f) {
            int sq=r*7+f; uint64_t m=1ULL<<sq;
            if (guardBB[0]&m)      std::cout<<"RG ";
            else if (guardBB[1]&m) std::cout<<"BG ";
            else if (height[sq]>0) {
                bool red=stackBB[0]&m;
                std::cout<<(red?'r':'b')<<int(height[sq])<<" ";
            }
            else std::cout<<"-- ";
        }
        std::cout<<"\n";
    }
    std::cout<<"   A  B  C  D  E  F  G\n\n";
}

bool Board::makeMove(const Move& mv) {
    int us=int(side), them=1-us;
    int from=mv.from, to=mv.to;
    uint64_t fm=1ULL<<from, tm=1ULL<<to;

    if (mv.splitCount==0) {
        // Guard‑Move
        if (!(guardBB[us]&fm)) return false;
        // capture Gegner falls da
        if (guardBB[them]&tm) guardBB[them]&=~tm, occAll&=~tm;
        if (stackBB[them]&tm) stackBB[them]&=~tm, height[to]=0, occAll&=~tm;
        // ziehen
        guardBB[us]&=~fm; guardBB[us]|=tm;
        occAll&=~fm; occAll|=tm;
    }
    else {
        // Stack‑Move
        int srcH=height[from], destH=height[to], mvn=mv.splitCount;
        if (srcH<=0||mvn<1||mvn>srcH) return false;
        // Quelle reduzieren
        if (mvn==srcH) { stackBB[us]&=~fm; height[from]=0; occAll&=~fm; }
        else           height[from]=srcH-mvn;
        // capture
        bool capSt=false;
        if (guardBB[them]&tm) guardBB[them]&=~tm, occAll&=~tm;
        if (stackBB[them]&tm) { stackBB[them]&=~tm; height[to]=0; occAll&=~tm; capSt=true; }
        // Landung
        int finalH = capSt ? mvn : ((stackBB[us]&tm)? destH+mvn : mvn);
        if (stackBB[us]&tm) height[to]=finalH;
        else { stackBB[us]|=tm; height[to]=finalH; occAll|=tm; }
    }

    side = (side==Color::Red?Color::Blue:Color::Red);
    return true;
}

bool Board::isGameOver() const {
    return (guardBB[0]==0 || guardBB[1]==0);
}

Color Board::winner() const {
    if (guardBB[0]==0 && guardBB[1]!=0) return Color::Blue;
    if (guardBB[1]==0 && guardBB[0]!=0) return Color::Red;
    return Color::None;
}

std::string Board::toFEN() const {
    std::ostringstream o;
    for (int rank=6;rank>=0;--rank) {
        int empty=0;
        for (int f=0;f<7;++f) {
            int sq=rank*7+f; uint64_t m=1ULL<<sq;
            bool gR=guardBB[0]&m, gB=guardBB[1]&m,
                 sR=stackBB[0]&m, sB=stackBB[1]&m;
            if (!gR&&!gB&&!sR&&!sB) { empty++; }
            else {
                if (empty>0){ o<<empty; empty=0; }
                if (gR) o<<"RG"; else if (gB) o<<"BG";
                else if (sR) o<<'r'<<int(height[sq]);
                else         o<<'b'<<int(height[sq]);
            }
        }
        if (empty>0) o<<empty;
        if (rank>0)  o<<'/';
    }
    o<<' '<<(side==Color::Red?'r':'b');
    return o.str();
}
