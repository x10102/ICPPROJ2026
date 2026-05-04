# ICPPROJ2026
diddy projekt kamo nevim

# co delat
- header guardy - `#define _JMENO_H`
- tridy - `PascalCase`
- funkce - `snake_case`
- konstanty - `PROSTE_VELKY_PISMENA`

# debug veci
pro loggovani importujte `debug.hpp`
- `LOG_I("neco")` logguje vzdycky
- `LOG_D("neco")` pouzijte pro dulezity debug info, aktivuje se kdyz definujete makro `DEBUG`. (`make program-dbg`)
- `LOG_T("neco")` pouzijte pro vypisovani promennych v loopu, loggovani ze se spustila nejaka funkce atd, proste veci co delaji extremni log spam. Aktivuje se kdyz definujete makro `TRACE`. (`make program-trace`)

vsechny logging veci funguji jako printf, to protoze volaji printf. takze pokud vypisujete c++ string tak na nej musite zavolat `.c_str()` napriklad, delat tohle pres iostream je borderline nemozny bohuzel. 

# gui spusteni
```bash
cd gui
cmake -DCMAKE_PREFIX_PATH=~/Qt/6.x.x/gcc_64 -B build
cmake --build build
./build/petri-editor
```

Za "6.x.x" doplnte svou verzi Qt. Testovano aktualne na 6.10.2
