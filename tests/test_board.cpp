#include <gtest/gtest.h>
#include "Board.hpp"
#include "MoveGenerator.hpp"
#include <set>
#include <string>

static std::set<std::string> run_test(const std::string& fen) {
    Board b;
    EXPECT_TRUE(b.loadFEN(fen));
    std::vector<Move> mv;
    MoveGenerator::generate(b, mv);
    std::set<std::string> s;
    for (auto& m : mv) s.insert(MoveGenerator::toString(m));
    return s;
}

TEST(GruppeA, Case1) {
    std::string fen = "7/2r4RG3/3r13/7/7/2b11b32/1b21BG3 b";
    std::set<std::string> expected = { "E2-C2-2", "B1-A1-1", "B1-B2-1", "B1-B3-2", "B1-C1-1", "C2-B2-1", "C2-C1-1", "C2-C3-1", "C2-D2-1", "D1-C1", "D1-D2", "D1-E1", "E2-D2-1", "E2-E1-1", "E2-E3-1", "E2-E4-2", "E2-E5-3", "E2-F2-1", "E2-G2-2" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeA, Case2) {
    std::string fen = "1r1b3RGr12/3r13/3r13/7/3r21b21/3b13/3BG3 b";
    std::set<std::string> expected = { "C7-B7-1", "C7-C4-3", "C7-C5-2", "C7-C6-1", "C7-D7-1", "D1-C1", "D1-E1", "D2-C2-1", "D2-E2-1", "F3-D3-2", "F3-E3-1", "F3-F1-2", "F3-F2-1", "F3-F4-1", "F3-F5-2", "F3-G3-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeAA, Case1) {
    std::string fen = "7/2r14/3b11RG1/3r13/7/2b11b12/3BG3 b";
    std::set<std::string> expected = { "C2-B2-1", "C2-C1-1", "C2-C3-1", "C2-D2-1", "D1-C1", "D1-D2", "D1-E1", "D5-C5-1", "D5-D4-1", "D5-D6-1", "D5-E5-1", "E2-D2-1", "E2-E1-1", "E2-E3-1", "E2-F2-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeAA, Case2) {
    std::string fen = "7/7/3r1BG2/4r1RG1/7/7/7 r";
    std::set<std::string> expected = { "D5-C5-1", "D5-D4-1", "D5-D6-1", "D5-E5-1", "E4-D4-1", "E4-E3-1", "E4-E5-1", "F4-F3", "F4-F5", "F4-G4" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeAB, Case1) {
    std::string fen = "3RG3/r11r11r11r1/1r11r11r11/7/1b11b11b11/b11b11b11b1/3BG3 r";
    std::set<std::string> expected = { "A6-A5-1", "A6-A7-1", "A6-B6-1", "B5-A5-1", "B5-B4-1", "B5-B6-1", "B5-C5-1", "C6-B6-1", "C6-C5-1", "C6-C7-1", "C6-D6-1", "D5-C5-1", "D5-D4-1", "D5-D6-1", "D5-E5-1", "D7-C7", "D7-D6", "D7-E7", "E6-D6-1", "E6-E5-1", "E6-E7-1", "E6-F6-1", "F5-E5-1", "F5-F4-1", "F5-F6-1", "F5-G5-1", "G6-F6-1", "G6-G5-1", "G6-G7-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeAB, Case2) {
    std::string fen = "7/7/7/3r2RG2/1r32r22/2b24/3BGb12 b";
    std::set<std::string> expected = { "C2-A2-2", "C2-B2-1", "C2-C1-1", "C2-C3-1", "C2-C4-2", "C2-D2-1", "C2-E2-2", "D1-C1", "D1-D2", "E1-E2-1", "E1-F1-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeAC, Case1) {
    std::string fen = "r1r11RG1r1r1/3r33/3b23/7/7/4b12/b1b11BG1b1b1 r";
    std::set<std::string> expected = { "A7-A6-1", "A7-B7-1", "B7-A7-1", "B7-B6-1", "B7-C7-1", "D6-A6-3", "D6-B6-2", "D6-C6-1", "D6-E6-1", "D6-F6-2", "D6-G6-3", "D7-C7", "D7-E7", "F7-E7-1", "F7-F6-1", "F7-G7-1", "G7-F7-1", "G7-G6-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeAC, Case2) {
    std::string fen = "7/BG6/7/RG6/6r1/7/7 r";
    std::set<std::string> expected = { "A4-A3", "A4-A5", "A4-B4", "G3-F3-1", "G3-G2-1", "G3-G4-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeAD, Case1) {
    std::string fen = "3RG3/1r25/7/3r3b42/1b1BG4/4b12/7 r";
    std::set<std::string> expected = { "B6-A6-1", "B6-B4-2", "B6-B5-1", "B6-B7-1", "B6-C6-1", "B6-D6-2", "D4-A4-3", "D4-B4-2", "D4-C4-1", "D4-D1-3", "D4-D2-2", "D4-D3-1", "D4-D5-1", "D4-D6-2", "D7-C7", "D7-D6", "D7-E7" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeAD, Case2) {
    std::string fen = "7/1b44b3/7/2BG4/3r13/2r1RG3/7 r";
    std::set<std::string> expected = { "C2-B2-1", "C2-C1-1", "C2-C3-1", "D2-D1", "D2-E2", "D3-C3-1", "D3-D4-1", "D3-E3-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeAF, Case1) {
    std::string fen = "3RG3/7/7/r16/r16/7/b42BG3 b";
    std::set<std::string> expected = { "A1-A2-1", "A1-A3-2", "A1-B1-1", "A1-C1-2", "D1-C1", "D1-D2", "D1-E1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeAI, Case1) {
    std::string fen = "RG6/3b3r32/3r21b21/7/4r22/7/6BG r";
    std::set<std::string> expected = { "A7-A6", "A7-B7", "D5-B5-2", "D5-C5-1", "D5-D3-2", "D5-D4-1", "D5-E5-1", "D5-F5-2", "E3-C3-2", "E3-D3-1", "E3-E1-2", "E3-E2-1", "E3-E4-1", "E3-E5-2", "E3-F3-1", "E3-G3-2", "E6-E3-3", "E6-E4-2", "E6-E5-1", "E6-E7-1", "E6-F6-1", "E6-G6-2" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeAI, Case2) {
    std::string fen = "2RG2b41/7/7/3r41r3b3/7/7/3BG3 b";
    std::set<std::string> expected = { "D1-C1", "D1-D2", "D1-E1", "F7-C7-3", "F7-D7-2", "F7-E7-1", "F7-F4-3", "F7-F5-2", "F7-F6-1", "F7-G7-1", "G4-G1-3", "G4-G2-2", "G4-G3-1", "G4-G5-1", "G4-G6-2", "G4-G7-3" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeAJ, Case1) {
    std::string fen = "7/r12RG2r1/7/3b33/7/2b24/3BG3 b";
    std::set<std::string> expected = { "C2-A2-2", "C2-B2-1", "C2-C1-1", "C2-C3-1", "C2-C4-2", "C2-D2-1", "C2-E2-2", "D1-C1", "D1-D2", "D1-E1", "D4-A4-3", "D4-B4-2", "D4-C4-1", "D4-D2-2", "D4-D3-1", "D4-D5-1", "D4-D6-2", "D4-E4-1", "D4-F4-2", "D4-G4-3" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeAJ, Case2) {
    std::string fen = "3RG3/r14r11/2r34/7/7/2b12b11/1b11BG3 r";
    std::set<std::string> expected = { "A6-A5-1", "A6-A7-1", "A6-B6-1", "C5-A5-2", "C5-B5-1", "C5-C2-3", "C5-C3-2", "C5-C4-1", "C5-C7-2", "C5-D5-1", "C5-E5-2", "C5-F5-3", "C5-C6-1", "D7-C7", "D7-D6", "D7-E7", "F6-E6-1", "F6-F5-1", "F6-F7-1", "F6-G6-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeAL, Case1) {
    std::string fen = "7/4RG2/3r23/1r35/3b53/3BG3/7 b";
    std::set<std::string> expected = { "D2-C2", "D2-D1", "D2-E2", "D3-A3-3", "D3-B3-2", "D3-C3-1", "D3-D4-1", "D3-D5-2", "D3-E3-1", "D3-F3-2", "D3-G3-3" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeAL, Case2) {
    std::string fen = "3RG3/3r23/7/7/7/3b13/3BG3 r";
    std::set<std::string> expected = { "D6-B6-2", "D6-C6-1", "D6-D4-2", "D6-D5-1", "D6-E6-1", "D6-F6-2", "D7-C7", "D7-E7" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeAM, Case1) {
    std::string fen = "RGBG5/7/7/7/7/7/7 r";
    std::set<std::string> expected = { "A7-A6", "A7-B7" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeB, Case1) {
    std::string fen = "3RG1r1r1/r1r1r11r12/3r13/7/3b33/2b11b12/3BG1b1b1 r";
    std::set<std::string> expected = { "A6-A5-1", "A6-A7-1", "A6-B6-1", "B6-A6-1", "B6-B5-1", "B6-B7-1", "B6-C6-1", "C6-B6-1", "C6-C5-1", "C6-C7-1", "C6-D6-1", "D5-C5-1", "D5-D4-1", "D5-D6-1", "D5-E5-1", "D7-C7", "D7-D6", "D7-E7", "E6-D6-1", "E6-E5-1", "E6-E7-1", "E6-F6-1", "F7-E7-1", "F7-F6-1", "F7-G7-1", "G7-F7-1", "G7-G6-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeB, Case2) {
    std::string fen = "7/3r33/2RG4/7/3BG3/1b25/7 b";
    std::set<std::string> expected = { "B2-A2-1", "B2-B1-1", "B2-B3-1", "B2-B4-2", "B2-C2-1", "B2-D2-2", "D3-C3", "D3-D2", "D3-D4", "D3-E3" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeB1, Case1) {
    std::string fen = "3RG1r1r1/2r11r12/7/7/3r23/2b21b12/b11BG2b1b1 b";
    std::set<std::string> expected = { "A1-A2-1", "A1-B1-1", "C1-B1", "C1-D1", "C2-A2-2", "C2-B2-1", "C2-C3-1", "C2-C4-2", "C2-D2-1", "C2-E2-2", "E2-D2-1", "E2-E1-1", "E2-E3-1", "E2-F2-1", "F1-E1-1", "F1-F2-1", "F1-G1-1", "G1-F1-1", "G1-G2-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeB1, Case2) {
    std::string fen = "3RG1r21/2b21r12/3r13/7/7/4b12/b12BG1b1b1 r";
    std::set<std::string> expected = { "D5-C5-1", "D5-D4-1", "D5-D6-1", "D5-E5-1", "D7-C7", "D7-D6", "D7-E7", "E6-D6-1", "E6-E5-1", "E6-E7-1", "E6-F6-1", "F7-E7-1", "F7-F5-2", "F7-F6-1", "F7-G7-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeB6, Case1) {
    std::string fen = "3RG1r1r1/7/7/7/7/7/3BG1b1b1 r";
    std::set<std::string> expected = { "D7-D6", "D7-C7", "D7-E7", "F7-F6-1", "F7-E7-1", "F7-G7-1", "G7-G6-1", "G7-F7-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeB6, Case2) {
    std::string fen = "3RG3/3r13/7/7/7/3b13/3BG3 b";
    std::set<std::string> expected = { "D1-C1", "D1-E1", "D2-D3-1", "D2-C2-1", "D2-E2-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeC, Case1) {
    std::string fen = "3RG1r11/3r33/r23r12/7/b32b33/7/3BG2b1 b";
    std::set<std::string> expected = { "A3-A1-2", "A3-A2-1", "A3-A4-1", "A3-A5-2", "A3-B3-1", "A3-C3-2", "A3-D3-3", "D1-C1", "D1-D2", "D1-E1", "D3-A3-3", "D3-B3-2", "D3-C3-1", "D3-D2-1", "D3-D4-1", "D3-D5-2", "D3-D6-3", "D3-E3-1", "D3-F3-2", "D3-G3-3", "G1-F1-1", "G1-G2-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeC, Case2) {
    std::string fen = "7/3RG3/7/3r23/3b13/3BG3/7 r";
    std::set<std::string> expected = { "D4-B4-2", "D4-C4-1", "D4-D3-1", "D4-D5-1", "D4-E4-1", "D4-F4-2", "D6-C6", "D6-D5", "D6-D7", "D6-E6" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeF, Case1) {
    std::string fen = "2RG4/5b21/4r22/3r23/2BG2b21/1r25/r16 b";
    std::set<std::string> expected = { "C3-B3", "C3-C2", "C3-C4", "C3-D3", "F3-D3-2", "F3-E3-1", "F3-F1-2", "F3-F2-1", "F3-F4-1", "F3-F5-2", "F3-G3-1", "F6-F4-2", "F6-D6-2", "F6-E6-1", "F6-F5-1", "F6-F7-1", "F6-G6-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeF, Case2) {
    std::string fen = "7/7/7/1r21RG3/2r1r13/2BGb43/7 b";
    std::set<std::string> expected = { "C2-B2", "C2-C1", "C2-C3", "D2-D1-1", "D2-D3-1", "D2-E2-1", "D2-F2-2", "D2-G2-3" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeG, Case1) {
    std::string fen = "4RG2/4r12/1b21b23/2r22r21/5b21/3BGb12/7 b";
    std::set<std::string> expected = { "B5-A5-1", "B5-B3-2", "B5-B4-1", "B5-B6-1", "B5-B7-2", "B5-C5-1", "B5-D5-2", "D2-C2", "D2-D1", "D2-D3", "D5-B5-2", "D5-C5-1", "D5-D3-2", "D5-D4-1", "D5-D6-1", "D5-D7-2", "D5-E5-1", "D5-F5-2", "E2-E1-1", "E2-E3-1", "E2-F2-1", "F3-D3-2", "F3-E3-1", "F3-F1-2", "F3-F2-1", "F3-G3-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeG, Case2) {
    std::string fen = "1b22RG2/7/3b2BG2/2r12b11/2r14/7/7 r";
    std::set<std::string> expected = { "C3-B3-1", "C3-C2-1", "C3-C4-1", "C3-D3-1", "C4-B4-1", "C4-C3-1", "C4-C5-1", "C4-D4-1", "E7-F7", "E7-D7", "E7-E6" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeJ, Case1) {
    std::string fen = "r14r21/1r1r1RG3/4r12/7/2b1r1b12/1b22b22/3BG3 r";
    std::set<std::string> expected = { "A7-A6-1", "A7-B7-1", "B6-A6-1", "B6-B5-1", "B6-B7-1", "B6-C6-1", "C6-B6-1", "C6-C5-1", "C6-C7-1", "D3-C3-1", "D3-D2-1", "D3-D4-1", "D3-E3-1", "D6-D5", "D6-D7", "D6-E6", "E5-D5-1", "E5-E4-1", "E5-E6-1", "E5-F5-1", "F7-D7-2", "F7-E7-1", "F7-F5-2", "F7-F6-1", "F7-G7-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeJ, Case2) {
    std::string fen = "7/7/7/2r34/1RG5/2b24/1b1BG4 b";
    std::set<std::string> expected = { "B1-A1-1", "B1-B2-1", "C1-D1", "C2-A2-2", "C2-B2-1", "C2-C3-1", "C2-D2-1", "C2-E2-2" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeK, Case1) {
    std::string fen = "b3r25/r26/RG6/7/7/7/BG6 b";
    std::set<std::string> expected = { "A1-A2", "A1-B1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeK, Case2) {
    std::string fen = "RG6/7/3r13/2r1b2r12/3r13/7/6BG b";
    std::set<std::string> expected = { "D4-C4-1", "D4-D3-1", "D4-D5-1", "D4-E4-1", "G1-F1", "G1-G2" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeL, Case1) {
    std::string fen = "r1r11RG1r1r1/2r14/4r22/7/3b23/2b14/b1b11BG1b1b1 b";
    std::set<std::string> expected = { "A1-A2-1", "A1-B1-1", "B1-A1-1", "B1-B2-1", "B1-C1-1", "C2-B2-1", "C2-C1-1", "C2-C3-1", "C2-D2-1", "D1-C1", "D1-D2", "D1-E1", "D3-B3-2", "D3-C3-1", "D3-D2-1", "D3-D4-1", "D3-D5-2", "D3-E3-1", "D3-F3-2", "F1-E1-1", "F1-F2-1", "F1-G1-1", "G1-F1-1", "G1-G2-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeL, Case2) {
    std::string fen = "7/r1r15/2BG4/7/6RG/7/7 b";
    std::set<std::string> expected = { "C5-B5", "C5-C4", "C5-C6", "C5-D5" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeM, Case1) {
    std::string fen = "3RG3/7/3r53/7/3b43/7/3BG3 b";
    std::set<std::string> expected = { "D1-C1", "D1-D2", "D1-E1", "D3-A3-3", "D3-B3-2", "D3-C3-1", "D3-D2-1", "D3-D4-1", "D3-E3-1", "D3-F3-2", "D3-G3-3" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeM, Case2) {
    std::string fen = "7/7/7/7/3b13/2b6RG3/3BGr72 r";
    std::set<std::string> expected = { "D2-C2", "D2-D1", "D2-D3", "D2-E2", "E1-D1-1", "E1-E2-1", "E1-E3-2", "E1-E4-3", "E1-E5-4", "E1-E6-5", "E1-E7-6", "E1-F1-1", "E1-G1-2" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeN, Case1) {
    std::string fen = "3RG1r11/3r33/r36/7/b32b33/7/3BG2b1 b";
    std::set<std::string> expected = { "A3-A1-2", "A3-A2-1", "A3-A4-1", "A3-B3-1", "A3-C3-2", "A3-D3-3", "D1-C1", "D1-D2", "D1-E1", "D3-A3-3", "D3-B3-2", "D3-C3-1", "D3-D2-1", "D3-D4-1", "D3-D5-2", "D3-D6-3", "D3-E3-1", "D3-F3-2", "D3-G3-3", "G1-F1-1", "G1-G2-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeN, Case2) {
    std::string fen = "6r1/3BG3/1r15/5RG1/1b25/7/7 b";
    std::set<std::string> expected = { "B3-A3-1", "B3-B1-2", "B3-B2-1", "B3-B4-1", "B3-B5-2", "B3-C3-1", "B3-D3-2", "D6-C6", "D6-D5", "D6-D7", "D6-E6" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeO, Case1) {
    std::string fen = "r1r11RG3/6r1/3r11r21/7/3b23/1b15/b12BG1b1b1 b";
    std::set<std::string> expected = { "A1-A2-1", "A1-B1-1", "B2-A2-1", "B2-B1-1", "B2-B3-1", "B2-C2-1", "D1-C1", "D1-D2", "D1-E1", "D3-B3-2", "D3-C3-1", "D3-D2-1", "D3-D4-1", "D3-D5-2", "D3-E3-1", "D3-F3-2", "F1-E1-1", "F1-F2-1", "F1-G1-1", "G1-F1-1", "G1-G2-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeO, Case2) {
    std::string fen = "3RG3/2b11b12/7/7/7/5b11/4BG2 r";
    std::set<std::string> expected = { "D7-C7", "D7-D6", "D7-E7" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeP, Case1) {
    std::string fen = "3RG3/7/7/7/4b11b1/4r4r11/3BG1b11 b";
    std::set<std::string> expected = { "D1-C1", "D1-D2", "D1-E1", "E3-D3-1", "E3-E4-1", "E3-F3-1", "F1-E1-1", "F1-F2-1", "F1-G1-1", "G3-F3-1", "G3-G2-1", "G3-G4-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeP, Case2) {
    std::string fen = "3RG3/1b32r22/7/1r24r1/7/2b14/4b2BG1 b";
    std::set<std::string> expected = { "B6-A6-1", "B6-B4-2", "B6-B5-1", "B6-B7-1", "B6-C6-1", "B6-D6-2", "B6-E6-3", "C2-B2-1", "C2-C1-1", "C2-C3-1", "C2-D2-1", "E1-E3-2", "E1-C1-2", "E1-D1-1", "E1-E2-1", "F1-F2", "F1-G1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeQ, Case1) {
    std::string fen = "3RG3/2r11r12/1r21r11r21/7/3b33/2b11b12/1b21BG3 b";
    std::set<std::string> expected = { "B1-A1-1", "B1-B2-1", "B1-B3-2", "B1-C1-1", "C2-B2-1", "C2-C1-1", "C2-C3-1", "C2-D2-1", "D1-C1", "D1-D2", "D1-E1", "D3-A3-3", "D3-B3-2", "D3-C3-1", "D3-D2-1", "D3-D4-1", "D3-D5-2", "D3-E3-1", "D3-F3-2", "D3-G3-3", "E2-D2-1", "E2-E1-1", "E2-E3-1", "E2-F2-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeQ, Case2) {
    std::string fen = "3RG3/4r12/3r43/7/3b33/3b23/3BG3 r";
    std::set<std::string> expected = { "D5-A5-3", "D5-B5-2", "D5-C5-1", "D5-D4-1", "D5-D6-1", "D5-E5-1", "D5-F5-2", "D5-G5-3", "D7-C7", "D7-D6", "D7-E7", "E6-D6-1", "E6-E5-1", "E6-E7-1", "E6-F6-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeR, Case1) {
    std::string fen = "3RG3/3r33/3b33/7/7/7/3BG3 r";
    std::set<std::string> expected = { "D6-A6-3", "D6-B6-2", "D6-C6-1", "D6-E6-1", "D6-F6-2", "D6-G6-3", "D7-C7", "D7-E7" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeR, Case2) {
    std::string fen = "1RG1r23/7/7/7/4b42/7/5BG1 b";
    std::set<std::string> expected = { "E3-A3-4", "E3-B3-3", "E3-C3-2", "E3-D3-1", "E3-E1-2", "E3-E2-1", "E3-E4-1", "E3-E5-2", "E3-E6-3", "E3-E7-4", "E3-F3-1", "E3-G3-2", "F1-E1", "F1-F2", "F1-G1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeS, Case1) {
    std::string fen = "7/1BG5/7/3r73/7/5RG1/7 r";
    std::set<std::string> expected = { "D4-A4-3", "D4-B4-2", "D4-C4-1", "D4-D1-3", "D4-D2-2", "D4-D3-1", "D4-D5-1", "D4-D6-2", "D4-D7-3", "D4-E4-1", "D4-F4-2", "D4-G4-3", "F2-E2", "F2-F1", "F2-F3", "F2-G2" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeS, Case2) {
    std::string fen = "7/1BG5/3b13/2b1r7b12/3b13/5RG1/7 r";
    std::set<std::string> expected = { "D4-C4-1", "D4-D3-1", "D4-D5-1", "D4-E4-1", "F2-E2", "F2-F1", "F2-F3", "F2-G2" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeT, Case1) {
    std::string fen = "7/5r11/2BGRG1b11/5r11/1b1b2r33/7/7 b";
    std::set<std::string> expected = { "B3-A3-1", "B3-B2-1", "B3-B4-1", "B3-C3-1", "C3-B3-1", "C3-C1-2", "C3-C2-1", "C3-C4-1", "C5-B5", "C5-C4", "C5-C6", "C5-D5", "F5-E5-1", "F5-F4-1", "F5-F6-1", "F5-G5-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeT, Case2) {
    std::string fen = "BGRG5/1b3r34/2b1r1r12/7/7/7/7 r";
    std::set<std::string> expected = { "B7-A7", "B7-B6", "B7-C7", "C6-C5-1", "C6-C7-1", "C6-D6-1", "C6-E6-2", "C6-F6-3", "D5-C5-1", "D5-D4-1", "D5-D6-1", "D5-E5-1", "E5-D5-1", "E5-E4-1", "E5-E6-1", "E5-F5-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeTrainer, Case1) {
    std::string fen = "r1r11RG1r1r1/2r11r12/3r13/7/3b13/2b11b12/b1b11BG1b1b1 r";
    std::set<std::string> expected = { "A7-A6-1", "A7-B7-1", "B7-A7-1", "B7-B6-1", "B7-C7-1", "C6-B6-1", "C6-C5-1", "C6-C7-1", "C6-D6-1", "D7-C7", "D7-D6", "D7-E7", "E6-D6-1", "E6-E5-1", "E6-E7-1", "E6-F6-1", "F7-E7-1", "F7-F6-1", "F7-G7-1", "G7-F7-1", "G7-G6-1", "D5-C5-1", "D5-D4-1", "D5-D6-1", "D5-E5-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeU, Case1) {
    std::string fen = "7/6r3/1RG5/3b43/1r25/7/2BG3r1 r";
    std::set<std::string> expected = { "B3-A3-1", "B3-B1-2", "B3-B2-1", "B3-B4-1", "B3-C3-1", "B3-D3-2", "B5-A5", "B5-B4", "B5-B6", "B5-C5", "G1-F1-1", "G1-G2-1", "G6-D6-3", "G6-E6-2", "G6-F6-1", "G6-G3-3", "G6-G4-2", "G6-G5-1", "G6-G7-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeV, Case1) {
    std::string fen = "3RG3/2r14/b2r11b22r2/1r15/7/2b14/3BG2b2 b";
    std::set<std::string> expected = { "A5-A3-2", "A5-A4-1", "A5-A6-1", "A5-A7-2", "A5-B5-1", "C2-B2-1", "C2-C1-1", "C2-C3-1", "C2-D2-1", "D1-C1", "D1-D2", "D1-E1", "D5-B5-2", "D5-C5-1", "D5-D3-2", "D5-D4-1", "D5-D6-1", "D5-D7-2", "D5-E5-1", "D5-F5-2", "G1-E1-2", "G1-F1-1", "G1-G2-1", "G1-G3-2" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeV, Case2) {
    std::string fen = "7/3RG3/7/3r23/7/3BG3/7 r";
    std::set<std::string> expected = { "D4-B4-2", "D4-C4-1", "D4-D2-2", "D4-D3-1", "D4-D5-1", "D4-E4-1", "D4-F4-2", "D6-C6", "D6-D5", "D6-D7", "D6-E6" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeW, Case1) {
    std::string fen = "3RG1r21/7/3r53/7/3b53/7/1b21BG3 r";
    std::set<std::string> expected = { "D5-A5-3", "D5-B5-2", "D5-C5-1", "D5-D4-1", "D5-D6-1", "D5-E5-1", "D5-F5-2", "D5-G5-3", "D7-C7", "D7-D6", "D7-E7", "F7-E7-1", "F7-F5-2", "F7-F6-1", "F7-G7-1" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeW, Case2) {
    std::string fen = "3RG1r21/7/3r22r3/7/3b53/7/1b21BG3 b";
    std::set<std::string> expected = { "B1-A1-1", "B1-B2-1", "B1-B3-2", "B1-C1-1", "D1-C1", "D1-D2", "D1-E1", "D3-A3-3", "D3-B3-2", "D3-C3-1", "D3-D2-1", "D3-D4-1", "D3-D5-2", "D3-E3-1", "D3-F3-2", "D3-G3-3" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeX, Case1) {
    std::string fen = "7/7/4b22/3b1RGb11/4BG2/7/r26 r";
    std::set<std::string> expected = { "A1-A2-1", "A1-A3-2", "A1-B1-1", "A1-C1-2", "E4-D4", "E4-E3", "E4-E5", "E4-F4" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeZ, Case1) {
    std::string fen = "3RG3/7/7/7/7/3b53/3BG3 b";
    std::set<std::string> expected = { "D1-C1", "D1-E1", "D2-A2-3", "D2-B2-2", "D2-C2-1", "D2-D3-1", "D2-D4-2", "D2-D5-3", "D2-D6-4", "D2-D7-5", "D2-E2-1", "D2-F2-2", "D2-G2-3" };
    EXPECT_EQ(run_test(fen), expected);
}

TEST(GruppeZ, Case2) {
    std::string fen = "3RG3/7/7/1b11b21r11/7/3BG3/7 b";
    std::set<std::string> expected = { "B4-A4-1", "B4-B3-1", "B4-B5-1", "B4-C4-1", "D2-C2", "D2-D1", "D2-D3", "D2-E2", "D4-C4-1", "D4-D3-1", "D4-D5-1", "D4-D6-2", "D4-E4-1", "D4-F4-2", "D4-B4-2" };
    EXPECT_EQ(run_test(fen), expected);
}
