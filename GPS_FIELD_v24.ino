// GPS_FIELD
// Do pracy z wyświetlaczem OLED i dodano informacje o lokalizatorze QRA.
// Program wyświetla CZAS LOKALNY i w formacie 12/24 godzinnym.
//
// Sprzęt:
// - Arduino Nano z wyświetlaczem OLED 64 x 128 (I2C)
// - Moduł GPS GY-GPS6MV2 (e-bay) z chipsetem NEO-6M-0-001
//
// Połączenia między Nano, OLED i modułem GPS:
// - OLED I2C: A4-SDA, A5-SCL, 5Volt-Vcc
// - GPS: 5Volt-Vcc, RX0-TX.  !!!>>  Odłączyć podczas programowania <<!!!
// - 01-02-2025 - Dodano kilka funkcji, optymalizacja kodu - SQ1KSM

#include <TinyGPS++.h>  // Biblioteka TinyGPS++
#include <U8glib.h>     // Biblioteka do obsługi wyświetlacza OLED

// Inicjalizacja obiektów
TinyGPSPlus gps;                                // Biblioteka do obsługi GPS
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);  // Biblioteka do obsługi wyświetlacza OLED

// Stałe do obliczeń QRA
const float LONGITUDE_OFFSET = 180000000;  // Przesunięcie długości geograficznej
const float LATITUDE_OFFSET = 90000000;    // Przesunięcie szerokości geograficznej
const float GRID_SIZE_LONG = 20000000;     // Rozmiar siatki dla długości geograficznej
const float GRID_SIZE_LAT = 10000000;      // Rozmiar siatki dla szerokości geograficznej
const float SUBGRID_SIZE_LONG = 2000000;   // Rozmiar podziałki dla długości geograficznej
const float SUBGRID_SIZE_LAT = 1000000;    // Rozmiar podziałki dla szerokości geograficznej

// Przełączanie formatu czasu i offset strefy czasowej
#define HU_24_H true  // false = 12-godzinny, true = 24-godzinny
const int HOUR_OFFSET = 0;  // Offset strefy czasowej (np. 1 dla UTC+1)

void setup() {
  Serial.begin(9600);  // Inicjalizacja komunikacji szeregowej
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_helvR08);
    u8g.drawStr(35, 10, "GPS - FIELD");   // Wyświetlenie tekstu na ekranie startowym
    u8g.setFont(u8g_font_gdr20);
    u8g.drawStr(10, 40, "SQ1KSM");   // Wyświetlenie znaku
    u8g.setFont(u8g_font_helvR08);
    u8g.drawStr(55, 60, "Slawek");   // Wyświetlenie imienia
  } while (u8g.nextPage());
  delay(2000);
}

void loop() {
  // Symulacja odbioru danych GPS (możesz zastąpić tym rzeczywiste dane z modułu GPS)
  while (Serial.available() > 0) {
    char c = Serial.read();
    gps.encode(c);  // Przekazanie danych do TinyGPS++
  }

  // Aktualizacja wyświetlacza co 1 sekundę
  static uint32_t timer = millis();
  if (millis() - timer > 1000) {
    timer = millis();

    u8g.firstPage();
    do {
      displaytime();     // Wyświetlenie czasu
      displaydate();     // Wyświetlenie daty
      displaylocation(); // Wyświetlenie lokalizacji
      displaysats();     // Wyświetlenie liczby satelit
      displayQRA();      // Wyświetlenie lokalizatora QRA
    } while (u8g.nextPage());
  }
}

// Funkcja pomocnicza do wyświetlania elementów czasu (z zerami wiodącymi)
void printTimeElement(uint8_t element) {
  if (element < 10) u8g.print("0");  // Dodaj zero wiodące dla liczb jednocyfrowych
  u8g.print(element, DEC);  // Wyświetlenie liczby
}

void displaytime() {
  u8g.setFont(u8g_font_gdr20);  // Ustawienie czcionki
  u8g.setPrintPos(0, 42);       // Ustawienie pozycji kursora

  uint8_t hours = gps.time.hour() + HOUR_OFFSET;  // Dodanie offsetu strefy czasowej
  if (hours >= 24) hours -= 24;  // Korekta przekroczenia północy
  uint8_t minutes = gps.time.minute();// Pobranie minut
  uint8_t seconds = gps.time.second();// Pobranie sekund

  if (HU_24_H) {
    // Format 24-godzinny
    u8g.print("  ");  // Dodaj dwie spacje przed czasem
    printTimeElement(hours);  // Wyświetlenie godzin
  } else {
    // Format 12-godzinny
    if (hours > 12) {
      u8g.setFont(u8g_font_helvR08);
      u8g.print("PM ");  // Wyświetlenie PM dla godzin > 12
      u8g.setFont(u8g_font_gdr20);
      hours -= 12;  // Konwersja na format 12-godzinny
    } else {
      u8g.setFont(u8g_font_helvR08);
      u8g.print("AM ");  // Wyświetlenie AM dla godzin <= 12
      u8g.setFont(u8g_font_gdr20);
    }
    printTimeElement(hours);  // Wyświetlenie godzin
  }

  u8g.print(":");           // Separator
  printTimeElement(minutes);// Wyświetlenie minut
  u8g.print(":");           // Separator
  printTimeElement(seconds);// Wyświetlenie sekund
  u8g.print(" ");           // Spacja
}

void displaydate() {
  u8g.setFont(u8g_font_helvR08);  // Ustawienie czcionki
  u8g.setPrintPos(0, 64);         // Ustawienie pozycji kursora

  // Poprawne wyświetlanie daty w formacie DD-MM-YY
  u8g.print(gps.date.day() < 10 ? "0" : "");  // Dodaj zero wiodące dla dni < 10
  u8g.print(gps.date.day());      // Wyświetlenie dnia
  u8g.print('-');                 // Separator
  u8g.print(gps.date.month() < 10 ? "0" : "");  // Dodaj zero wiodące dla miesięcy < 10
  u8g.print(gps.date.month());    // Wyświetlenie miesiąca
  u8g.print('-');                 // Separator
  u8g.print(gps.date.year() % 100);  // Wyświetlenie dwóch ostatnich cyfr roku
}

void displaylocation() {
  u8g.setFont(u8g_font_helvR08);  // Ustawienie czcionki
  u8g.setPrintPos(0, 10);         // Ustawienie pozycji kursora

  if (gps.satellites.value() == 0) {
    u8g.print("Czekam na GPS...");  // Komunikat oczekiwania na dane GPS
  } else {
    // Szerokość geograficzna w formacie DD.MM
    double lat = gps.location.lat();   // Stopnie dziesiętne
    u8g.print(abs(lat), 2);            // Wyświetlenie stopni z dwoma cyframi po przecinku
    u8g.print(lat < 0 ? " S" : " N");  // Kierunek (N/S)

    u8g.print(" - ");

    // Długość geograficzna w formacie DD.MM
    double lng = gps.location.lng();   // Stopnie dziesiętne
    u8g.print(abs(lng), 2);            // Wyświetlenie stopni z dwoma cyframi po przecinku
    u8g.print(lng < 0 ? " W" : " E");  // Kierunek (E/W)
  }
}

void displayQRA() {
  if (gps.satellites.value() == 0) return;  // Pominięcie, jeśli brak danych GPS

  float loclong = gps.location.lng() * 1000000;  // Przeliczenie długości geograficznej
  float loclat = gps.location.lat() * 1000000;   // Przeliczenie szerokości geograficznej

  // Korekta współrzędnych w zależności od kierunku (E/W, N/S)
  if (!gps.location.rawLng().negative) loclong += LONGITUDE_OFFSET;
  if (gps.location.rawLng().negative) loclong = LONGITUDE_OFFSET - abs(loclong);
  if (!gps.location.rawLat().negative) loclat += LATITUDE_OFFSET;
  if (gps.location.rawLat().negative) loclat = LATITUDE_OFFSET - abs(loclat);

  u8g.setFont(u8g_font_helvR08);  // Ustawienie czcionki
  u8g.setPrintPos(86, 10);        // Ustawienie pozycji kursora

  // Dodaj spację przed QRA
  u8g.print(" ");

  // Pierwszy i drugi znak
  u8g.print(char(int(loclong / GRID_SIZE_LONG) + 'A'));  // Pierwszy znak długości
  u8g.print(char(int(loclat / GRID_SIZE_LAT) + 'A'));    // Pierwszy znak szerokości

  // Trzeci i czwarty znak
  u8g.print(char(int((loclong - (GRID_SIZE_LONG * int(loclong / GRID_SIZE_LONG))) * 10 / 20 / 1000000) + '0'));  // Trzeci znak długości
  u8g.print(char(int((loclat - (GRID_SIZE_LAT * int(loclat / GRID_SIZE_LAT))) / 1000000) + '0'));  // Trzeci znak szerokości

  // Piąty i szósty znak
  u8g.print(char(int(((loclong / SUBGRID_SIZE_LONG) - int(loclong / SUBGRID_SIZE_LONG)) * 24) + 'A'));  // Piąty znak długości
  u8g.print(char(int(((loclat / SUBGRID_SIZE_LAT) - int(loclat / SUBGRID_SIZE_LAT)) * 24) + 'A'));  // Piąty znak szerokości
}

void displaysats() {
  u8g.setFont(u8g_font_helvR08);  // Ustawienie czcionki
  u8g.setPrintPos(70, 64);        // Ustawienie pozycji kursora
  if (gps.satellites.isValid() && gps.satellites.value() < 100) {
    u8g.print("Satelit: ");       // Wyświetlenie tekstu
    u8g.print(gps.satellites.value());  // Wyświetlenie liczby satelit
  }
}
