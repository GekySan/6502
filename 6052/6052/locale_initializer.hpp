#pragma once
#include <locale>

struct LocaleInitializer {
    LocaleInitializer() {
        std::locale::global(std::locale("fr_FR.UTF-8"));
    }
};

static const LocaleInitializer locale_initializer;
