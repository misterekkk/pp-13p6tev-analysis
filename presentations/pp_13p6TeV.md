---
marp: true
lang: pl
paginate: true
size: 16:9
style: |
  section {
    font-family: "Helvetica Neue", Helvetica, Arial, sans-serif;
    font-size: 24px;
    padding: 40px 56px 36px 56px;
    color: #1d2433;
    background: #ffffff;
    place-content: start !important;
  }
  h1 { font-size: 40px; color: #12355b; margin: 0 0 12px 0; }
  h2 { font-size: 30px; color: #12355b; margin: 0 0 14px 0; border-bottom: 2px solid #d6e0ea; padding-bottom: 6px; }
  section::after { font-size: 16px; color: #7a8699; }
  .row { display: flex; gap: 14px; justify-content: center; align-items: flex-start; }
  .row img { border: 1px solid #e3e8ee; }
  .row3 img { width: 370px; }
  .row2 img { width: 560px; }
  .split { display: grid; grid-template-columns: 620px 1fr; gap: 28px; align-items: center; }
  .split img { width: 620px; border: 1px solid #e3e8ee; }
  .note { font-size: 20px; line-height: 1.4; margin-top: 12px; }
  .note ul { margin: 0; padding-left: 22px; }
  .small { font-size: 18px; color: #4a5568; }
  table { font-size: 20px; border-collapse: collapse; margin: 0 auto; }
  th { background: #12355b; color: #fff; padding: 6px 14px; }
  td { padding: 5px 14px; border-bottom: 1px solid #e3e8ee; }
  section.title { place-content: safe center start !important; }
  section.title h1 { font-size: 46px; }
  section.title p { font-size: 24px; color: #4a5568; }
---

<!-- _class: title -->
<!-- _paginate: false -->

# Symulacja i analiza rozkładów cząstek<br>pp @ √s = 13.6 TeV

Pythia 8 + ROOT · 100 000 zdarzeń minimum bias

Kacper Piotrowski

---

## Simulation setup

<div class="split">
<div>

| Parametr | Wartość |
|---|---|
| Wiązki | p + p, √s = 13.6 TeV |
| Procesy | `SoftQCD:inelastic` |
| Tune | Monash 2013 (`Tune:pp = 14`) |
| Rozpady | tylko cząstki z cτ < 10 mm |
| Zdarzenia | 100 000, seed 12345 |
| Cząstki | `isFinal()`, bez cięć |

</div>
<div class="note">

Pythia 8.318, ROOT 6.40.04, CMake 4.3, Apple clang 21, macOS 26.6.

`pp_generate` zapisuje zderzenia do `events.root`, `pp_analyze` tworzy z nich `analysis.root`, a makro `draw_plots.C` rysuje wykresy.

Generacja trwa ok. 100 s, analiza ok. 3 s.

</div>
</div>

---

## Task interpretation

<div class="note">

- Bierzemy tylko zderzenia niesprężyste (`SoftQCD:inelastic`). Zderzenia elastyczne nie tworzą nowych cząstek i zaniżałyby średnie na zderzenie.
- Cząstki, które przed rozpadem przelatują ponad 1 cm (np. K⁰S, Λ), zostawiamy nierozpadnięte, tak jak liczy je ALICE.
- Wnioski opieramy na kategoriach z zadania: naładowane, π⁺, π⁻, K⁺, K⁻, p, p̄. `all` to dodatkowo wszystkie cząstki, także neutralne.
- Nie stosujemy cięć w pT i η. Wartości spoza zakresów są w binach przepełnienia i liczą się do średnich.
- Pythia podaje φ od −π do π, a zadanie wymaga zakresu 0–π, więc `hPhi_*` zawiera |φ|.

</div>

---

## Results

<div class="split">
<div>

| Kategoria | ⟨N⟩ / zdarzenie | ⟨pT⟩ [GeV] |
|---|---|---|
| naładowane | 83.92 | 0.527 |
| π⁺ / π⁻ | 34.51 / 33.77 | 0.492 / 0.494 |
| K⁺ / K⁻ | 4.25 / 4.12 | 0.679 / 0.676 |
| p / p̄ | 3.06 / 1.94 | 0.685 / 0.789 |
| wszystkie | 181.65 | 0.416 |

</div>
<div class="note">

- Wśród cząstek naładowanych 81% to piony, 10% kaony, a 6% protony.
- `all` jest ok. 2 razy większe od `ch`, bo na zderzenie przypada średnio 83 fotony z rozpadów π⁰.
- W 86% zderzeń jest co najmniej jedna naładowana cząstka w |η| < 1 (INEL>0).
- Poza zakresem wykresów jest 11% zdarzeń z N > 400 dla `all` i 27% cząstek naładowanych z |η| > 5.

</div>
</div>

---

## Multiplicity N: charged and all particles

<div class="row row2">

![](../plots/hN_ch.png) ![](../plots/hN_all.png)

</div>
<div class="note">

- Rozkład ma długi ogon do kilkuset cząstek. Liczba cząstek naładowanych jest zawsze parzysta, bo na początku ładunek wynosi +2, dlatego na wykresie `hN_ch` biny są scalone po 2.
- `all` jest ok. 2 razy większe przez fotony z π⁰, a 11% zdarzeń wychodzi poza zakres 0–400.

</div>

---

## Multiplicity N: pions

<div class="row row3">

![](../plots/hN_pip.png) ![](../plots/hN_pim.png) ![](../plots/hN_pions.png)

</div>
<div class="note">

Rozkłady π⁺ i π⁻ prawie się pokrywają. π⁺ jest trochę więcej (34.5 wobec 33.8 na zderzenie), bo zderzają się dwa dodatnio naładowane protony.

</div>

---

## Multiplicity N: kaons

<div class="row row3">

![](../plots/hN_Kp.png) ![](../plots/hN_Km.png) ![](../plots/hN_kaons.png)

</div>
<div class="note">

Kaonów jest ok. 8 razy mniej niż pionów, bo kwarki dziwne trzeba dopiero wytworzyć w zderzeniu. Rozkład jest więc wąski i zaczyna się od zera. K⁺ i K⁻ jest praktycznie tyle samo.

</div>

---

## Multiplicity N: protons

<div class="row row3">

![](../plots/hN_p.png) ![](../plots/hN_pbar.png) ![](../plots/hN_protons.png)

</div>
<div class="note">

Protonów jest wyraźnie więcej niż antyprotonów (3.06 wobec 1.94), bo dwa zderzające się protony wnoszą liczbę barionową +2, która musi zostać w stanie końcowym.

</div>

---

## Multiplicity N: species comparison

<div class="split">

![](../plots/hN_species.png)

<div class="note">

Kształty znormalizowane do jedności.

Im cięższa cząstka, tym rzadziej powstaje i tym węższy jest jej rozkład. π⁺ to ok. 41% cząstek naładowanych, więc jego rozkład wygląda podobnie, tylko przy ok. 2.4 razy mniejszych N.

</div>
</div>

---

## Transverse momentum p<sub>T</sub>: charged and all particles

<div class="row row2">

![](../plots/hPt_ch.png) ![](../plots/hPt_all.png)

</div>
<div class="note">

- Większość cząstek ma pT poniżej 1 GeV (średnio 0.53 GeV dla naładowanych), a do 10 GeV widmo spada o 5 rzędów wielkości.
- `all` ma niższy średni pT (0.42 GeV), bo π⁰ rozpada się na dwa fotony, które dzielą jego pęd.

</div>

---

## Transverse momentum p<sub>T</sub>: pions

<div class="row row3">

![](../plots/hPt_pip.png) ![](../plots/hPt_pim.png) ![](../plots/hPt_pions.png)

</div>
<div class="note">

Widma π⁺ i π⁻ są takie same w granicach statystyki. Najwięcej pionów ma pT ok. 0.3 GeV, a średnio 0.49 GeV.

</div>

---

## Transverse momentum p<sub>T</sub>: kaons

<div class="row row3">

![](../plots/hPt_Kp.png) ![](../plots/hPt_Km.png) ![](../plots/hPt_kaons.png)

</div>
<div class="note">

Kaony mają średnio większy pT niż piony (0.68 GeV). Widma K⁺ i K⁻ się pokrywają.

</div>

---

## Transverse momentum p<sub>T</sub>: protons

<div class="row row3">

![](../plots/hPt_p.png) ![](../plots/hPt_pbar.png) ![](../plots/hPt_protons.png)

</div>
<div class="note">

Nadmiar protonów jest tylko przy pT poniżej ok. 1.5 GeV, wyżej widma p i p̄ się pokrywają. Dodatkowe protony to głównie wolne resztki protonów z wiązek, dlatego średni pT protonów (0.69 GeV) jest niższy niż antyprotonów (0.79 GeV).

</div>

---

## Transverse momentum p<sub>T</sub>: species comparison

<div class="split">

![](../plots/hPt_species.png)

<div class="note">

Kształty znormalizowane do jedności.

Cięższe cząstki mają średnio większy pT: π 0.49, K 0.68, p 0.69 GeV. Powyżej kilku GeV cząstek jest mało, bo próbka SoftQCD nie zawiera osobno generowanych twardych zderzeń.

</div>
</div>

---

## Pseudorapidity η: charged and all particles

<div class="row row2">

![](../plots/hEta_ch.png) ![](../plots/hEta_all.png)

</div>
<div class="note">

- Rozkład jest symetryczny względem η = 0, bo zderzają się dwie takie same cząstki. Dołek w środku wynika z tego, że η liczymy z samego kąta lotu, bez masy cząstki. Dla cięższych i wolniejszych cząstek zaniża to ich liczbę przy η = 0.
- Ok. 27% cząstek naładowanych leci bliżej niż 0.8° od wiązki (|η| > 5) i jest poza zakresem wykresu.

</div>

---

## Pseudorapidity η: pions

<div class="row row3">

![](../plots/hEta_pip.png) ![](../plots/hEta_pim.png) ![](../plots/hEta_pions.png)

</div>
<div class="note">

Piony są lekkie i zwykle szybkie, więc η prawie nie zaniża ich liczby i dołek przy η = 0 jest ledwo widoczny. π⁺ jest wszędzie trochę więcej niż π⁻.

</div>

---

## Pseudorapidity η: kaons

<div class="row row3">

![](../plots/hEta_Kp.png) ![](../plots/hEta_Km.png) ![](../plots/hEta_kaons.png)

</div>
<div class="note">

Kaony są ok. 3.5 razy cięższe od pionów, więc dołek przy η = 0 jest już wyraźny.

</div>

---

## Pseudorapidity η: protons

<div class="row row3">

![](../plots/hEta_p.png) ![](../plots/hEta_pbar.png) ![](../plots/hEta_protons.png)

</div>
<div class="note">

Protony są najcięższe, więc dołek przy η = 0 jest najgłębszy: liczba protonów spada tam o ok. 1/3. Ponad połowa protonów (53%) leci prawie wzdłuż wiązki (|η| > 5). To głównie resztki protonów z wiązek.

</div>

---

## Pseudorapidity η: species comparison

<div class="split">

![](../plots/hEta_species.png)

<div class="note">

Kształty znormalizowane do jedności.

Im cięższa cząstka, tym głębszy dołek przy η = 0: najpłytszy dla pionów, najgłębszy dla protonów. To ten sam efekt liczenia η z samego kąta, bez masy.

</div>
</div>

---

## Azimuthal angle φ: charged and all particles

<div class="row row2">

![](../plots/hPhi_ch.png) ![](../plots/hPhi_all.png)

</div>
<div class="note">

Rozkład jest płaski, bo zderzenie nie wyróżnia żadnego kierunku wokół wiązki. Histogram zawiera |φ| (zakres 0–π z zadania), a pełny zakres jest w `hPhiFull_*`.

</div>

---

## Azimuthal angle φ: pions

<div class="row row3">

![](../plots/hPhi_pip.png) ![](../plots/hPhi_pim.png) ![](../plots/hPhi_pions.png)

</div>
<div class="note">

Rozkłady są płaskie. π⁺ jest stale o ok. 2% więcej, tak jak w krotności.

</div>

---

## Azimuthal angle φ: kaons

<div class="row row3">

![](../plots/hPhi_Kp.png) ![](../plots/hPhi_Km.png) ![](../plots/hPhi_kaons.png)

</div>
<div class="note">

Rozkłady są płaskie, a wahania między binami to zwykłe fluktuacje statystyczne.

</div>

---

## Azimuthal angle φ: protons

<div class="row row3">

![](../plots/hPhi_p.png) ![](../plots/hPhi_pbar.png) ![](../plots/hPhi_protons.png)

</div>
<div class="note">

Rozkłady są płaskie. Protony leżą wyżej, bo jest ich więcej niż antyprotonów.

</div>

---

## Azimuthal angle φ: species comparison

<div class="split">

![](../plots/hPhi_species.png)

<div class="note">

Po normalizacji wszystkie rodzaje cząstek mają ten sam płaski rozkład, więc φ nie zależy od rodzaju cząstki.

</div>
</div>

---

## Summary and limitations

<div class="note">

- Całość, od generacji do wykresów, uruchamia się kilkoma poleceniami i przy tym samym configu i seedzie daje te same wyniki.
- Wyniki zgadzają się z podstawową fizyką: zachowanie ładunku daje parzyste N_ch i nadmiar π⁺, zachowanie liczby barionowej daje nadmiar protonów, cięższe cząstki mają większy pT, a rozkład φ jest płaski.
- Ograniczenia tej analizy:
  - nie stosujemy cięć, więc część cząstek leci tam, gdzie detektor by ich nie zobaczył,
  - przy wysokim pT cząstek jest mało,
  - używamy jednego ustawienia modelu (Monash 2013).
- Porównanie z danymi ALICE jest w osobnej prezentacji.

<p class="small">Pythia 8: <a href="https://pythia.org">pythia.org</a> · ROOT: <a href="https://root.cern">root.cern</a> · Kod: github.com/misterekkk/pp-13p6tev-analysis</p>

</div>
