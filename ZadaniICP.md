# Zadání: Nástroj pro vizuální editaci, generování kódu a monitorování běhu interpretovaných Petriho sítí
(event-driven SFC/Grafcet s multiset sémantikou)

## Interpretovaná Petriho síť

Interpretovanou Petriho sítí zde chápeme jako (časovanou) Petriho síť rozšířenou o:

- vstupní události (eventy)
- interní proměnné a akce
- strážní podmínky přechodů
- generování výstupních událostí
- plánování zpožděných odpálení přechodů (timeouty)

Síť je event-driven (reaktivní/proaktivní): neprovádí se cyklické skenování kroků v pevném taktu, ale systém čeká na externí událost nebo na timeout některého naplánovaného přechodu a teprve poté provede maximální množinu nezávislých odpálení.

Síť pracuje s multiset sémantikou míst:

- Každé místo (krok) obsahuje nezáporný počet tokenů (0..N)
- Tokeny jsou nerozlišené (neadresné). V základní verzi nemají vlastní datovou hodnotu
- Smyslem je umožnit souběhy a agregace (např. více současně probíhajících "instancí" stejného kroku) a soupeření o zdroje

Pro jednoduchost předpokládáme, že vstupy a výstupy jsou pouze typu String. Interní proměnné mohou být libovolného typu, který dovolí inskripční jazyk (standardně C/C++).

Umístění nějaké hodnoty do některého vstupu (uživatelskou interakcí nebo předání zprávy z jiného systému) je vstupní událost. Hodnota uložená na vstupu (tzv. poslední známá hodnota příslušného vstupu) je vždy k dispozici pro čtení a je použitelná ve strážích přechodů i v akcích přechodů.

Výstupní události se generují odesláním nějaké hodnoty do některého výstupu v rámci akce přechodu (případně akce místa, viz dále).

## Prvky sítě

### Místa (places / kroky) obsahují:

- Identifikátor (jméno)
- Počáteční počet tokenů (integer >= 0)
- Volitelná akce místa (place action), která se provede při "vstupu tokenu" do místa (tj. při každém přidání tokenu), nebo alternativně při změně počtu tokenů (toto musí být v zadání přesně definováno a dodrženo implementací; doporučeno: akce se provádí pro každý přidaný token)

### Přechody (transitions) obsahují:

- Identifikátor (jméno)
- Vstupní hrany z míst do přechodu s vahou w >= 1 (kolik tokenů se spotřebuje z daného místa)
- Výstupní hrany z přechodu do míst s vahou w >= 1 (kolik tokenů se vyrobí v daném místě)
- Podmínka odpálení (enabling condition) složená ze 3 částí, přičemž každá může chybět:

```
input_event_name [ bool_expr_in_C_C++ ] @ delay_in_ms
```

kde:
- `input_event_name` je název vstupní události, která musí nastat, aby byl přechod kandidátem (pokud chybí, jde o "interní" přechod)
- `bool_expr_in_C_C++` je booleovský výraz nad proměnnými a last-known hodnotami vstupů
- `delay_in_ms` je zpoždění odpálení přechodu; pokud je uvedeno, odpálení se naplánuje a proběhne až po timeoutu

### Akce přechodu:

- Kód v inskripčním jazyce (ideálně podmnožina C/C++)
- Akce se provede při skutečném odpálení přechodu (tj. po spotřebování tokenů a před umístěním výstupních tokenů)

## Inskripční jazyk

Základní verze: podmnožina C/C++ interpretovaná/kompilovaná v rámci generovaného interpretu.

Musí být k dispozici minimálně tyto vestavěné funkce:

- `valueof("in_name")` -> `const char*` (poslední známá hodnota vstupu)
- `defined("in_name")` -> `bool` (zda byl vstup někdy nastaven, případně zda byl nastaven od posledního zpracování – zvolte a specifikujte)
- `output("out_name", x)` -> `void` (vygeneruje výstupní událost; x se konvertuje na string)
- `tokens("place")` -> `int` (aktuální počet tokenů v místě)
- `elapsed("place")` -> `int` (doba od poslední změny tokenů v místě, nebo od poslední aktivace – zvolte a specifikujte; doporučeno: od poslední změny počtu tokenů)
- `elapsed("transition")` -> `int` (doba nepřetržité proveditelnosti časovaného neboli zpožděného přechodu)
- `now()` -> `int64` (čas v ms od startu)
- Další pomocné funkce dle potřeby

Pozn.: Pokud zavedete place actions, doporučuje se doplnit:
`on_enter("place")` / `on_exit("place")` nebo jednoduše akce místa "on_add_token".
V základní verzi postačí akce na přechodech.

## Nástroj pro vizuální editaci, generování kódu a monitorování

Umožní vizuálně specifikovat síť (místa jako kroužky nebo elipsy, přechody jako čtverečky nebo obdélníky, orientované hrany s vahami jako šipky), vstupy, výstupy a interní proměnné. Podmínky přechodů a akce přechodů (a případně akce míst) lze specifikovat vhodným inskripčním jazykem (ideálně podmnožinou C nebo C++).

Umožní kompletní specifikaci sítě uložit do souboru v čitelné textové podobě, kterou lze případně i ručně editovat. Takto uloženou síť lze načíst a dále editovat. Z této textové podoby může být i generován kód (detaily viz dále).

Umožní jedním kliknutím vygenerovat kód interpretu, přeložit ho, spustit a následně se k němu připojit např. přes UDP sockety a kdykoli injektovat do interpretu vstupy a průběžně přijímat, logovat a v GUI zobrazovat veškeré události v reálném čase.
(Spuštění a komunikace ale může být řešena i jinak - např. sdílenou knihovnou, případně interpret nepřekládat, ale použít vestavěný interpret vhodného jazyka.)

### Za běhu zobrazuje:

- Aktuální "marking" sítě: počty tokenů ve všech místech (nejlépe přímo v diagramu, vše naráz)
- Seznam právě povolených přechodů (enabled) a seznam naplánovaných (čekajících) timeoutů (pending timers), nejlépe přímo v diagramu (např. barevně zvýraznit)
- Poslední známé hodnoty všech vstupů, výstupů a interních proměnných
- Log událostí: datum a čas, popis události (externí vstup / odpálení přechodu / plánování timeoutu / timeout), změny tokenů, změny proměnných (alespoň ty, které lze zachytit), vygenerované výstupy

### Za běhu umožní:

- Asynchronně (tj. kdykoli za běhu) pohodlně injektovat aktualizované hodnoty vstupů, zadané uživatelem

### Při startu:

- Pokusí se připojit k běžící síti, pokud takovou najde
- Zjistí si její jméno, načte odpovídající zdrojový soubor, zobrazí síť a začne ji monitorovat, tj. průběžně zobrazovat kompletní stav
- Běžící síť může na pokyn uživatele požádat o ukončení

## Sémantika odpálení a "nezávislost"

### Přechod je povolen (enabled), pokud:

- Ve všech vstupních místech je dostatek tokenů dle vah hran (multiset enabling)
- Je splněna jeho událostní část (pokud má input_event_name, musí přijatá událost odpovídat)
- Je splněna booleovská stráž (pokud existuje)

### Při odpálení (fire) přechodu:

- Ze vstupních míst se odebere příslušný počet tokenů dle vah
- Provede se akce přechodu
- Do výstupních míst se přidá příslušný počet tokenů dle vah
- Volitelně se provedou place actions pro každé přidání tokenu

### Nezávislé odpálení:

- Dva přechody jsou v daném okamžiku nezávislé, pokud jejich odpálení nekonkuruje - nesoupeří o stejné tokeny (tj. součet požadovaných tokenů z každého místa nepřekročí dostupný počet)
- V jedné "reakci" systému (viz smyčka níže) se odpálí maximální množina nezávislých přechodů
- Pokud existuje konflikt (více kandidátů soutěží o stejné tokeny), musí být zvoleno deterministické pravidlo výběru (např. pevné pořadí přechodů v souboru, nebo priority přechodů). Toto pravidlo musí být jasně definováno a dodrženo

### Zpožděné přechody:

- Pokud je přechod povolen a má uvedeno @delay_in_ms, pak se neodpálí okamžitě, ale vytvoří se časovač (naplánované odpálení)
- Po vypršení časovače se přechod pokusí odpálit
- Pokud po doběhnutí časovače už přechod není povolen (např. tokeny byly spotřebovány jiným přechodem):
  - doporučeno: timeout pouze vyvolá "pokus o odpálení"; pokud není povolen, timeout se zahodí

### Odstraňování časovačů:

- Pokud přechod přestal dávat smysl (např. jeho vstupní místa už nemají dost tokenů), základní verze může časovač ponechat a při timeoutu pouze zkontrolovat povolení
- Alternativně lze implementovat aktivní čištění časovačů (pokročilejší; volitelné)

## Algoritmus provádění interpretované event-driven časované Petriho sítě (hlavní smyčka)

Dokud není požadavek na ukončení:

1. **Zpracuj "reakci bez čekání":**
   - Dokud existuje množina nezávislých proveditelných okamžitých přechodů:
     - Vyber deterministicky maximální množinu nezávislých přechodů (dle pravidla výběru)
     - Odpál přechody (v definovaném pořadí), proveď jejich akce, uprav marking
     - Zaloguj všechny změny (odpálení, tokeny, výstupy)
     - Pokud se odpálením některého přechodu objeví nově povolené přechody, pokračuj ve smyčce (stabilizace, mikrokroky až do ustálení)

2. **Naplánuj zpožděné přechody:**
   - Pro každý přechod, který je v aktuálním markingu povolen a splňuje podmínku (událost/stráž) a má @delay_in_ms:
     - Pokud pro něj už neběží časovač se stejnými parametry, založ/aktualizuj časovač
   - (Je nutné definovat, zda se časovač zakládá jen po konkrétní události, nebo i bez události. Doporučeno: přechod s input_event_name se plánuje pouze jako reakci na tuto událost; přechod bez input_event_name se může plánovat kdykoli je povolen)

3. **Pasivně čekej:**
   - Pasivně čekej na:
     - externí vstupní událost (nastavení vstupu)
     - timeout některého časovače
     - příkaz pro ukončení

4. **Po probuzení:**
   - Jde-li o timeout časovače:
     - Zjisti, který přechod timeoutem vypršel, odstraň jeho časovač
     - Pokud je přechod právě teď povolen (marking + stráž), odpál ho
     - Jinak pouze zaloguj "timeout ignored"
   - Jde-li o externí vstup obsahující vstupní událost:
     - Ulož poslední známou hodnotu příslušného vstupu
     - Aktivuj kandidátní přechody navázané na tuto událost
     - Okamžité odpál dle kroku (1), zpožděné naplánuj dle kroku (2)
   - Jde-li o externí vstup obsahující příkaz pro ukončení:
     - nastav požadavek na ukončení na TRUE

## Textový formát sítě (příklad struktury)

Soubor musí být čitelný a snadno editovatelný ručně. Minimálně musí obsahovat:

- jméno sítě
- komentář
- vstupy, výstupy
- proměnné (včetně inicializace)
- místa (včetně počátečních tokenů a případných akcí)
- přechody (každý má vstupní a výstupní hrany s váhami, podmínky, akce)

Doporučený formát (orientační; můžete navrhnout vlastní, ale musí být jednoznačný):

```
Jméno sítě:
...
Komentář:
...
Vstupy:
...
Výstupy:
...
Proměnné:
...
Místa (počáteční tokeny, volitelně akce):
    P1 (1) : { ... }
    P2 (0) : { ... }
Přechody (první blok = arcs, druhý blok = podmínky a akce):
T1 :
    in:  P1*1, P2*2
    out: P3*1
    when:  start [ ... ] @ 1000
    do: { ... }
```

## Příklad sítě (jednoduchá verze)

```
Jméno sítě:
    TOF_PN_5s
Komentář:
    Timer to off, jednoduchá verze.
Vstupy:
    in
Výstupy:
    out
Proměnné:
    int timeout = 5000
Místa (počáteční tokeny, volitelně akce):
    IDLE   (1) : { output("out", 0); }
    ACTIVE (0) : { output("out", 1); }
    TIMING (0) : { }
Přechody a jejich podmínky:
T_on :
    in:  IDLE*1
    out: ACTIVE*1
    when: in [ atoi(valueof("in")) == 1 ]
    do: { }
T_off_start :
    in:  ACTIVE*1
    out: TIMING*1
    when: in [ atoi(valueof("in")) == 0 ]
    do: { }

T_cancel_off :
    in:  TIMING*1
    out: ACTIVE*1
    when: in [ atoi(valueof("in")) == 1 ]
    do: { }

T_timeout_off :
    in:  TIMING*1
    out: IDLE*1
    when: @ timeout
    do: { }
```

Pozn.: Akce míst IDLE/ACTIVE zde slouží jako ekvivalent Moore výstupu; při přidání tokenu do místa se odešle výstup.
Pozn.: Pokud neimplementujete place actions, pak výstupy řešte v akcích přechodů (např. v T_on poslat out=1, v T_timeout_off poslat out=0, atd.).

## Příklad sítě (rozšířená verze)

```
Jméno sítě:
    TOF_PN
Komentář:
    Timer to off, umí nastavit timeout a na požádání sdělit zbývající čas timeru.
Vstupy:
    in
    set_to
    req_rt
Výstupy:
    out
    rt
Proměnné:
    int timeout = 5000
Místa (počáteční tokeny, volitelně akce):
    IDLE   (1) : { output("out", 0); output("rt", 0); }
    ACTIVE (0) : { output("out", 1); output("rt", timeout); }
    TIMING (0) : { output("rt", timeout - elapsed("TIMING")); }
Přechody a jejich podmínky:

T_on :
    in:  IDLE*1
    out: ACTIVE*1 
    when: in [ atoi(valueof("in")) == 1 ]
    do: { if (defined("set_to")) { timeout = atoi(valueof("set_to")); } }
T_off_start :
    in:  ACTIVE*1
    out: TIMING*1
    when: in [ atoi(valueof("in")) == 0 ]
    do: { if (defined("set_to")) { timeout = atoi(valueof("set_to")); } }

T_cancel_off :
    in:  TIMING*1
    out: ACTIVE*1
    when: in [ atoi(valueof("in")) == 1 ]
    do: { if (defined("set_to")) { timeout = atoi(valueof("set_to")); } }

T_timeout_off :
    in:  TIMING*1
    out: IDLE*1
    when: @ timeout
    do: { if (defined("set_to")) { timeout = atoi(valueof("set_to")); } }

T_set_idle :
    in:  IDLE*1
    out: IDLE*1
    when: set_to
    do: { timeout = atoi(valueof("set_to")); output("rt", 0); }

T_set_active :
    in:  ACTIVE*1
    out: ACTIVE*1
    when: set_to
    do: { timeout = atoi(valueof("set_to")); output("rt", timeout); }

T_set_timing :
    in:  TIMING*1
    out: TIMING*1
    when: set_to
    do: { timeout = atoi(valueof("set_to")); output("rt", timeout - elapsed("TIMING")); }

T_req_idle :
    in:  IDLE*1
    out: IDLE*1
    when: req_rt
    do: { output("rt", 0); }

T_req_active :
    in:  ACTIVE*1
    out: ACTIVE*1
    when: req_rt
    do: { output("rt", timeout); }

T_req_timing :
    in:  TIMING*1
    out: TIMING*1
    when: req_rt
    do: { output("rt", timeout - elapsed("TIMING")); }
```

Pozn.: elapsed("TIMING") vrací dobu trvání "markingu" místa TIMING (doporučeno: čas od poslední změny počtu tokenů v místě TIMING).
Pozn.: Přechody typu "P -> P" nemění počet tokenů; v takovém případě se elapsed může neresetovat (doporučeno), ale musí být definováno a dodrženo.

## Požadavky na GUI (minimální rozsah)

### Editor:

- přidání/mazání míst a přechodů
- kreslení oblouků s vahami (u každého oblouku číslo)
- editace vstupů/výstupů/proměnných
- editace podmínek a akcí
- uložení/načtení

### Runtime + monitor:

- živé zobrazení počtů tokenů v místech (v diagramu)
- log událostí a odpálení
- panel pro injektování vstupů
- přehled naplánovaných timeoutů

## Možná rozšíření pro čtyřčlenný tým

- Pracujte s množinou sítí a nechte je komunikovat vhodným způsobem, např. přes MQTT (publish/subscribe jako vstupní a výstupní události)
- Zaveďte barevné tokeny (Colored Petri Nets "light"): token nese jednoduchou hodnotu (např. int/string) a stráže/akce s ní mohou pracovat
- Doplňte verifikaci základních vlastností: detekce uváznutí (deadlock), dosažitelnost označených míst, nebo limitovaný reachability pro malé sítě
- Doplňte hierarchii (sub-nety) podobně jako hierarchické SFC: přechod může "rozbalit" podsíť a synchronizovat se návratem

---

## Obecné pokyny k realizaci projektu

- Zadání definuje podstatné vlastnosti aplikace, které musí být splněny. Předpokládá se, že detaily řešení si doplní řešitelské týmy
- Při návrhu postupujte tak, aby výsledný program byl dobře použitelný. Hodnotit se bude nejen to, zda splňuje výše uvedené požadavky, ale také JAK je splňuje
- Návrh musí oddělit vlastní model aplikace od GUI (navrhněte vhodné rozhraní)
- Návrh programu zdokumentujte ve zdrojovém kódu ve formátu pro program "doxygen"
- Používejte vhodné návrhové vzory (Design Patterns) - viz literatura
- Pro vytváření menu, dialogů a dalších komponent grafického uživatelského rozhraní (GUI) použijte toolkit Qt minimálně ve verzi 5.5 (nainstalováno v učebnách a na serveru "merlin" v adresáři /usr/local/share/Qt-5.5.1/). Pokud pro vývoj použijete verzi novější/starší než je na učebnách, předpokládáme, že zajistíte zpětnou/dopřednou kompatibilitu vašich programů. (V případě problémů, počítejte s nutností předvedení překladu/sestavení/běhu aplikace na vašem notebooku. Toto se nedoporučuje.)
- Protože toolkity jsou přenositelné na více platforem, měly by vaše programy fungovat minimálně na Linuxu i na Windows. (Nejen Java umožňuje psaní přenositelných aplikací.) Je jedno na které platformě proběhne vývoj aplikací, jen dbejte na pravidla psaní přenositelných programů. (Testovat se bude na Linuxu = server merlin, ale Makefile nesmí být závislý na konkrétním adresáři s lokální instalací toolkitu.)
- Na vytvoření GUI dialogů a menu můžete použít interaktivní editory/generátory
- Použijte C++17. (Boost raději nepoužívejte. Pokud použijete některé knihovny z Boost, tak použité soubory přibalte a zkontrolujte si, zda se to vše vejde do max. velikosti odevzdaného archivu. Existuje nástroj BCP pro výběr podmnožiny Boost.)
- Programy budou vytvořeny zadáním příkazu "make" v adresáři s rozbalenými zdrojovými kódy projektu (Pozor - bude se automaticky překládat). Musíte vytvořit soubor Makefile, který nesmí obsahovat absolutní cesty k souborům. Výsledný program vhodně pojmenujte. Pro testovací účely půjde vše spustit použitím "make run"
- Součástí projektu bude programová dokumentace vytvořená programem Doxygen (konfigurační soubor nastavte na HTML výstup se zahrnutím zdrojových textů: SOURCE_BROWSER = YES). Dokumentace se vygeneruje automaticky (= nezabalovat do archivu) při zadání "make doxygen" a bude uložena v podadresáři doc
- Součástí projektu bude také konceptuální návrh v pdf, obsahující minimálně diagram tříd bez zbytečných implementačních detailů, případně další UML diagramy dokumentující návrh. Očekává se, že implementace bude v souladu s konceptuálním návrhem

## Poznámky k odevzdávání

- Termín odevzdání je uveden v IS
- Řešení zabalené ve formátu .zip (nebo .tar.gz) odevzdá pouze vedoucí týmu do IS a to s dostatečným předstihem před uzávěrkou. (Nikdo nebude psát, že neodevzdal, protože *** a posílat to poštou.) Jméno odevzdaného souboru musí mít tvar:

```
xname01-xname02.zip
```

nebo

```
xname01-xname02-xname03.zip
```

- xname01 je jméno vedoucího (toho, kdo projekt odevzdal), xname02, resp. xname03 je druhý, resp.třetí člen týmu (ti nic neodevzdávají)
- POZOR: Velikost archivu musí odpovídat tomu, co dovolí IS. Proto si raději včas ověřte odevzdatelnost, ať nedojde k problému na poslední chvíli
- Odevzdaný archiv obsahuje minimálně tuto adresářovou strukturu:

```
./src/* - zdrojové texty
./examples/* - příklady (pro testování)
./doc/ - dokumentace (bude později generován: "make doxygen")
./README.txt - základní přehled o co jde, autoři, poznámky, ...
./Makefile - zavolá rekurzivně make na src/Makefile, případně na generování dokumentace atd.
```

- Neodevzdávejte nic, co lze vytvořit pomocí "make" (spustitelné programy ani dokumentaci, kterou lze vygenerovat použitím "make doxygen")
- Poznámka: Příkaz "make clean" všechny produkty překladu smaže, "make pack" vytvoří archiv pro odevzdání - zkontrolujte, zda obsahuje vše potřebné rozbalením v prázdném adresáři a překladem/spuštěním
- Všechny zdrojové soubory musí obsahovat na začátku komentář se jmény autorů daného modulu, popisem obsahu modulu, atd. Z komentářů by také mělo být patrné, které části byly napsány ručně a které vygenerovány automaticky
- Pokud některý modul vychází z převzatého kódu (např. z nějakého tutorialu nebo veřejně dostupného opensource projektu), musí být toto explicitně specifikováno v záhlaví modulu a také zmíněno v README
- V dokumentaci (README) musí být zřetelně specifikováno, jaká funkcionalita byla implementována kompletně, jaká s nějakými omezeními a jaká vůbec (aby se hodnotitel zbytečně nesnažil zprovoznit něco, o čem je dopředu známo, že je nefunkční). Nesrovnalost mezi dokumentací a realitou bude předmětem bodové penalizace

## Poznámky k obhajobě projektu

- Projekt musí být demonstrován a obhájen. Na termín obhajoby je nutné se přihlásit v IS

## Poznámky k hodnocení

- Projekt je hodnocen na stupnici 0-100 procent/bodů
- Po ohodnocení může dojít k přepočtu bodů na jednotlivé členy týmu podle návrhu vedoucího týmu, který je zakódován do jména odevzdaného souboru (formát jména pak je:

```
xname01-xname02-AA-BB.zip
```

- Součet AA+BB=100, POZOR: pro rovnoměrné rozdělení AA=BB=50% není třeba uvádět "-50-50" a je to preferovaná možnost
- Hodnocení zahrnuje kvalitu OO návrhu, kvalitu implementace v C++, dodržování stylu psaní programu (odsazování, kvalita komentářů, vhodné identifikátory) a především funkčnost programu. Pokud budou ve zdrojových textech chybět podstatné komentáře (zvláště jména autorů), bude projekt hodnocen 0b
- POZOR:
  - Pokud nepůjde přeložit příkazem make = 0b nebo předvedete na svém notebooku a vysvětlíte kde byl problém
  - Pokud nebudou komentáře se jmény autorů = 0b
  - Zcela nevhodně formátovaný a nečitelný kód = 0b
  - Plagiáty atd. = 0b + návštěva u disciplinární komise

---

Naposledy změněno: čtvrtek, 16. dubna 2026, 14.45
