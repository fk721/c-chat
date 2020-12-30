#ifndef COLORS_H
#define COLORS_H

#include <ncurses.h>

/// Custom colors
#define COLOR_GREY          100
#define COLOR_BLUISH        101

#define WOB                 1
#define BOW                 2
#define CYAN_ON_BLACK       3
#define CYAN_ON_WHITE       4
#define RED_ON_BLACK        5
#define RED_ON_WHITE        6
#define GREEN_ON_BLACK      7
#define GREEN_ON_WHITE      8
#define MAGENTA_ON_BLACK    9
#define MAGENTA_ON_WHITE    10
#define WHITE_ON_GREY       11
#define BLACK_ON_GREY       12
#define WHITE_ON_BLUISH     13
#define BLACK_ON_BLUISH     14
#define GREY_ON_BLACK       15
#define BLUISH_ON_BLACK     16


#define INIT_COLOR_PAIRS \
    init_color(COLOR_GREY, 430, 430, 430); \
    init_color(COLOR_BLUISH, 175, 352, 352); \
    init_pair(WOB, COLOR_WHITE, COLOR_BLACK); \
    init_pair(BOW, COLOR_BLACK, COLOR_WHITE); \
    init_pair(CYAN_ON_BLACK, COLOR_CYAN, COLOR_BLACK); \
    init_pair(CYAN_ON_WHITE, COLOR_CYAN, COLOR_WHITE); \
    init_pair(RED_ON_BLACK, COLOR_RED, COLOR_BLACK); \
    init_pair(RED_ON_WHITE, COLOR_RED, COLOR_WHITE); \
    init_pair(GREEN_ON_BLACK, COLOR_GREEN, COLOR_BLACK); \
    init_pair(GREEN_ON_WHITE, COLOR_GREEN, COLOR_WHITE); \
    init_pair(MAGENTA_ON_BLACK, COLOR_MAGENTA, COLOR_BLACK); \
    init_pair(MAGENTA_ON_WHITE, COLOR_MAGENTA, COLOR_WHITE); \
    init_pair(WHITE_ON_GREY, COLOR_WHITE, COLOR_GREY); \
    init_pair(BLACK_ON_GREY, COLOR_BLACK, COLOR_GREY); \
    init_pair(BLACK_ON_BLUISH, COLOR_BLACK, COLOR_BLUISH); \
    init_pair(WHITE_ON_BLUISH, COLOR_WHITE, COLOR_BLUISH); \
    init_pair(GREY_ON_BLACK, COLOR_GREY, COLOR_BLACK); \
    init_pair(BLUISH_ON_BLACK, COLOR_BLUISH, COLOR_BLACK); 

#endif