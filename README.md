# ICPPROJ2026
diddy projekt kamo nevim

# co delat
- header guardy - `#define _JMENO_H`
- tridy - `PascalCase`
- funkce - `snake_case`
- konstanty - `PROSTE_VELKY_PISMENA`

# debug veci
pro loggovani importujte `debug.hpp`
- `LOG_D("neco")` pouzijte pro dulezity debug info, aktivuje se kdyz definujete makro `DEBUG`. (`make program-dbg`)
- `LOG_T("neco")` pouzijte pro vypisovani promennych v loopu, loggovani ze se spustila nejaka funkce atd, proste veci co delaji extremni log spam. Aktivuje se kdyz definujete makro `TRACE`. (`make program-trace`)