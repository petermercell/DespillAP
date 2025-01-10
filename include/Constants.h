#ifndef CONSTANTS_H
#define CONSTANTS_H


namespace Constants {
    static const char *const RESPILL_MATH_TYPES[] = {
        "Rec 709",
        "Ccir 601",
        "Rec 2020",
        "Average",
        "Max",
        0};

    static const char *const COLOR_TYPES[] = {
        "Red",
        "Green",
        "Blue",
        "Pick",
        0};

    static const char *const OUTPUT_TYPES[] = {
        "Despill",
        "Spill",
        0};

    static const char *const DESPILL_MATH_TYPES[] = {
        "Average",
        "Max",
        "Min",
        "Custom",
        0};
}

#endif // CONSTANTS_H