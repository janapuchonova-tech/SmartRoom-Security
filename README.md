# SmartRoom-Security
SmartRoom Security je IoT projekt založený na mikrokontroléri **ESP32-S3**, ktorého cieľom je monitorovanie a základné zabezpečenie miestnosti. Projekt demonštruje praktické využitie Internetu vecí v oblasti zberu dát, ich spracovania, automatickej reakcie systému a vizualizácie údajov prostredníctvom webovej aplikácie.
Systém sleduje dianie v miestnosti pomocou senzorov a umožňuje používateľovi vzdialený prehľad o aktuálnom stave priestoru bez potreby inštalácie samostatnej aplikácie.

---

## Funkcie projektu
- detekcia pohybu v miestnosti pomocou PIR senzora  
- meranie intenzity osvetlenia pomocou fotorezistora  
- vizuálna a zvuková signalizácia alarmu (LED, buzzer)  
- webová aplikácia dostupná cez prehliadač  
- grafické zobrazenie intenzity svetla v čase  
- zobrazenie aktuálneho dátumu a času  
- získavanie vonkajšej teploty z verejného WEB API  

---

## Použitý hardvér
- ESP32-S3 vývojová doska  
- PIR senzor pohybu  
- fotorezistor (LDR)  
- rezistor (napäťový delič pre LDR)  
- LED dióda  
- aktívny bzučiak  
- nepájivé kontaktné pole (breadboard)  
- prepojovacie vodiče  

---

## Použitý softvér a technológie
- Arduino IDE  
- programovací jazyk C/C++ (Arduino)  
- Wi-Fi komunikácia  
- WebServer na ESP32  
- HTML, CSS a JavaScript  
- Open-Meteo WEB API  
- knižnica ArduinoJson  

---

## Využitie projektu
Projekt SmartRoom Security je vhodný najmä pre:
- demonštráciu princípov Internetu vecí (IoT)  
- edukačné účely v oblasti informatiky a automatizácie  
- základné zabezpečenie miestnosti alebo kancelárie  
- rozšírenie o ďalšie senzory (teplota, vlhkosť, hluk)  
- základ pre pokročilejšie smart-home riešenia  

Projekt je navrhnutý modulárne, čo umožňuje jeho ďalšie rozširovanie a úpravy podľa potrieb používateľa.

---

## Autor
Jana Puchoňová
