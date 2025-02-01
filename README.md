GPS in the field GPS dla krótkofalowca w polu pracującego, informuje go o godzinie UTC, którą wpisuje do dziennika, lokator QRA w razie pytania od korespondentów i do wpisania na kartę QSL
Ograniczenia - niestety wyświetlane jest maksymalnie 12 satelit pomimo podłączenoa np. M10 które widzi ponad 30...
Odczytywane są tylko ramki:<br>
GPGSV: Dotyczy satelitów GPS.<br>
Nie odczytuje:<br>
GLGSV: Dotyczy satelitów GLONASS.<br>
GBGSV: Dotyczy satelitów BeiDou.<br>
GAGSV: Dotyczy satelitów Galileo.<br>

# GPS_Field

## Opis projektu
GPS_Display to program napisany dla platformy Arduino, który bazuje na kodzie testowym dla modułów GPS Adafruit wykorzystujących sterownik MTK3329/MTK3339. Projekt został zoptymalizowany do pracy z wyświetlaczem OLED i dodano informacje o lokalizatorze QRA. Program wyświetla czas lokalny w formacie 12/24 godzinnym oraz współrzędne GPS.

## Sprzęt
- Arduino Nano z wyświetlaczem OLED 64 x 128 (I2C)
- Moduł GPS GY-GPS6MV2 (e-bay) z chipsetem NEO-6M-0-001

## Połączenia między Nano, OLED i modułem GPS
- OLED I2C: A4-SDA, A5-SCL, 5Volt-Vcc
- GPS: 5Volt-Vcc, RX0-TX.
  !!!>> Odłączyć podczas programowania <<!!!

## Instalacja
1. Podłącz Arduino Nano do komputera za pomocą kabla USB.
2. Otwórz Arduino IDE i wgraj bibliotekę Adafruit GPS oraz U8glib.
3. Skopiuj kod z pliku `GPS_Display.ino` do Arduino IDE.
4. Sprawdź połączenia między Arduino, OLED i modułem GPS zgodnie z powyższymi instrukcjami.
5. Wgraj kod do Arduino Nano.

## Modyfikacje
Plik pobrany z kilku miejsc w internecie (zlepek pomysłów, jeśli rościsz prawa do części kodu napisz):
- 26.01.2025 V.2.3 - Dodano kilka funkcji, optymalizacja kodu.
- 29.01.2025 Dalsza optymalizacja i opis działania w kodzie.
- 01.02.2025 V.2.4 - Zmiana biblioteki na TINY++ i drobne poprawki

## Funkcje programu
- Wyświetlanie czasu lokalnego w formacie 12/24 godzinnym.
- Wyświetlanie współrzędnych GPS, jeśli brak informacji wyświetla się odpowiedni komunikat.
- Wyświetlanie liczby satelitów.
- Wyświetlanie lokalizatora QRA, jeśli brak informacji z GPS to nic się nie wyświetla.
