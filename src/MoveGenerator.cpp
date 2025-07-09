#include "MoveGenerator.hpp"
#include <cassert>

// pathMask[sq][dir][dist] repräsentiert alle Zwischenfelder
static uint64_t pathMask[49][4][8];
static bool     pathMaskInit = false;
static const int dirOffset[4] = { +7, +1, -7, -1 };

static void ensurePathMask() {
    if (pathMaskInit) return;
    for (int sq = 0; sq < 49; ++sq) {
        int r = sq / 7, f = sq % 7;
        for (int d = 0; d < 4; ++d) {
            int dr = (d == 0 ? +1 : d == 2 ? -1 : 0);
            int df = (d == 1 ? +1 : d == 3 ? -1 : 0);
            for (int n = 1; n < 8; ++n) {
                uint64_t m = 0;
                int rr = r, ff = f;
                for (int s = 1; s < n; ++s) {
                    rr += dr; ff += df;
                    if (rr < 0 || rr > 6 || ff < 0 || ff > 6) { m = 0; break; }
                    m |= 1ULL << (rr*7 + ff);
                }
                pathMask[sq][d][n] = m;
            }
        }
    }
    pathMaskInit = true;
}

static inline int sqIdx(char file, char rank) { return (rank - '1')*7 + (file - 'A'); }
static inline char fileC(int sq) { return char('A' + (sq % 7)); }
static inline char rankC(int sq) { return char('1' + (sq / 7)); }

void MoveGenerator::generate(const Board& board, std::vector<Move>& out) {
    ensurePathMask();
    out.clear();
    int us   = int(board.side), them = 1 - us;
    uint64_t occAll = board.occAll;
    uint64_t ownS   = board.stackBB[us],
             ownG   = board.guardBB[us];

    // --- Guards: 1-Step Moves ---
    {
        uint64_t g = ownG;
        while (g) {
            int sq = __builtin_ctzll(g);
            g &= g - 1;
            int f = sq % 7;
            for (int d = 0; d < 4; ++d) {
                int dest = sq + dirOffset[d];
                if (dest < 0 || dest >= 49) continue;
                if ((d == 1 && f == 6) || (d == 3 && f == 0)) continue;
                if ((ownG | ownS) & (1ULL << dest)) continue;
                out.push_back({uint8_t(sq), uint8_t(dest), 0});
            }
        }
    }

    // --- Stacks: Full-Move h-Steine & Splits k-Steine ---
    {
        uint64_t s = ownS;
        while (s) {
            int sq = __builtin_ctzll(s);
            s &= s - 1;
            int h = board.height[sq];
            if (h <= 0) continue;
            int f = sq % 7, r = sq / 7;

            // Full-Move: alle h-Steine ziehen
            for (int d = 0; d < 4; ++d) {
                int dest = sq + dirOffset[d]*h;
                if (dest < 0 || dest >= 49) continue;
                if ((d == 1 && f+h > 6) || (d == 3 && f-h < 0)
                 || (d == 0 && r+h > 6) || (d == 2 && r-h < 0)) continue;
                if (occAll & pathMask[sq][d][h]) continue;
                uint64_t dm = 1ULL << dest;
                // keine eigene Guard blockiert
                if (board.guardBB[us] & dm) continue;
                // Capture nur erlaubt, wenn eigener h >= gegnerischer dh
                bool theirStack = board.stackBB[them] & dm;
                int dh = board.height[dest];
                if (theirStack && h < dh) continue;
                out.push_back({uint8_t(sq), uint8_t(dest), uint8_t(h)});
            }

            // Split-Moves: k = 1 .. h-1
            for (int d = 0; d < 4; ++d) {
                for (int k = 1; k < h; ++k) {
                    int dest = sq + dirOffset[d]*k;
                    if (dest < 0 || dest >= 49) continue;
                    if ((d == 1 && f+k > 6) || (d == 3 && f-k < 0)
                     || (d == 0 && r+k > 6) || (d == 2 && r-k < 0)) continue;
                    if (occAll & pathMask[sq][d][k]) continue;
                    uint64_t dm = 1ULL << dest;
                    if (board.guardBB[us] & dm) continue;
                    bool theirStack = board.stackBB[them] & dm;
                    int dh = board.height[dest];
                    // Capture-Split nur erlaubt, wenn k >= dh
                    if (theirStack && k < dh) continue;
                    out.push_back({uint8_t(sq), uint8_t(dest), uint8_t(k)});
                }
            }
        }
    }
}

bool MoveGenerator::parse(const std::string& s, Move& mv) {
    if (s.size() < 5 || s[2] != '-') return false;
    char f1 = s[0], r1 = s[1], f2 = s[3], r2 = s[4];
    if (f1<'A'||f1>'G'||r1<'1'||r1>'7'||f2<'A'||f2>'G'||r2<'1'||r2>'7')
        return false;
    int from = sqIdx(f1,r1), to = sqIdx(f2,r2), cnt = 0;
    if (s.size() > 5) {
        if (s[5] != '-') return false;
        cnt = std::stoi(s.substr(6));
        if (cnt <= 0) return false;
    }
    mv.from = uint8_t(from);
    mv.to = uint8_t(to);
    mv.splitCount = uint8_t(cnt);
    return true;
}

std::string MoveGenerator::toString(const Move& mv) {
    std::string o;
    o += fileC(mv.from);
    o += rankC(mv.from);
    o += '-';
    o += fileC(mv.to);
    o += rankC(mv.to);
    if (mv.splitCount > 0) {
        o += '-';
        o += char('0' + mv.splitCount);
    }
    return o;
}
