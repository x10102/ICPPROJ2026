# ICP Projekt 2026

Zadání: Nástroj pro vizuální editaci, generování kódu a monitorování běhu interpretovaných Petriho sítí 

# Spuštění

- `make` nebo `make run` sestaví a spustí editor
- `make gui` pouze sestaví editor
- `make pack` vytvoří archiv k odevzdání
- `make doxygen` vytvoří dokumentaci pod `/doc`
- `make clean` vyčistí sestavené soubory

# Poznámky
- Před sestavením interpretu je třeba vybrat adresář, kde se nachází jeho zdrojový kód. (Pomocí Soubor > vybrat zdrojový adresář)
- Hodnoty vstupů a výstupů se zobrazují na terminálu pod záložkou "Interpret"
- Příkaz "Krok" provede jednu iteraci odpálení přechodů, tzn. odpálí se pouze přechody, u kterých jsou aktuálně splněny podmínky, i pokud se důsledkem toho naplní podmínky pro další přechody
- Příkaz "Pokračovat" provádí další iterace, dokud jsou po posledním odpálení splněny podmínky pro jakýkoliv další přechod

# Popis celkové implementace

## Komunikace interpretu a editoru

Editor a interpret jsou oddělené procesy. Kód interpretu není určen k přeložení před spuštěním editoru a Makefile ho také nepřekládá.
Poté, co je v editoru vytvořena požadovaná síť, vygeneruje interpret kód sítě. Následně vytvoří kopii souboru `src/program.cpp` nazvanou `src/program.generated.cpp` a na specifické místo ve zdrojovém kódu (označené speciálním komentářem) kód vloží. Interpret následně přeloží spuštěním `make` s parametrem `program-generated` a spustí ho. 

Editor a interpret mezi sebou komunikují pomocí UDP socketů, interpret čeká na příkazy na portu 6768 a editor čeká na aktualizace stavu od interpretu na portu 6767. Očekává se, že oba procesy běží na stejném stroji, tzn. že datagramy se posílají přes lokální loopback rozhraní, není tedy implementována žádná ochrana proti výpadkům nebo zpoždění sítě. Každý datagram obsahuje prostý text ve formátu JSON, který je generován / zpracováván knihovnou [picojson](https://github.com/kazuho/picojson). Nečitelné nebo neplatné datagramy jsou z obou stran ignorovány. 

## Ukládání a načítání sítí

Editor ukládá specifikace sítí opět ve formátu JSON. Data míst a přechodů obsahují kromě specifikace navíc jejich pozici na obrazovce, aby se po načtení mohl editor nacházet přesně ve stejném stavu, v jakém byl uložen.

## Inskripční jazyk

Akce míst a přechodů mohou být zadány libovolně v C++ kódu, za podmínky, že jej lze vložit do těla lambda funkce. Podmínky pro odpálení přechodu mohou být zadány jako jeden C++ výraz s návratovou hodnotou typu `bool`, který lze vložit za přikaz `return`. Jsou k dispozici veškeré pomocné funkce specifikované v zadání.

## Interaktivní režim interpretu

Přeložený interpret umožňuje vstup do interaktivního režimu pomocí spuštění s parametrem `--interactive`. V interaktivním režimu lze skrz textové rozhraní krokovat, sledovat stav sítě, generovat vstupní události nebo síť v aktuálním stavu připojit k editoru. Přesné instrukce k použití jsou po spuštění interaktivního režimu vypsány na konzoli.

# Implementovaná funkcionalita

## Interpret

- Vytváření míst, přechodů a hran
- Akce míst, akce přechodů
- Zpožděné přechody
- Podmínka vstupní události
- Podpora inskripčního jazyka
- Interaktivní režim
- Komunikace s editorem (částečně, chybí odesílání aktuálních časovačů a hodnot proměnných. Časovače se zobrazují v terminálu, ale po jejich ukončení se GUI neaktualizuje)

## Editor

- Editace sítí a nastavení parametrů míst/přechodů/hran
- Import specifikace sítě
- Export specifikace sítě
- Nastavení názvu a popisu
- Definice proměnných
- Generování kódu a sestavení interpretu
- Tmavý režim
- Zobrazení aktuálního počtu tokenů v místech
- Zobrazení aktuálního výstupu interpretu

# Rozdíly ve vylepšené verzi
- Načtení sítě vymaže aktuální pracovní plochu
- Načtení sítě ukončí aktuálně běžící interpret
- GUI se aktualizuje s novým stavem po odpálení zpožděných přechodů
- Tlačítko "Pokračovat" v GUI nechá simulaci běžet samovolně až do konce