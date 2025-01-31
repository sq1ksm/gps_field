// Ten program bazuje na kodzie testowym dla modułów GPS Adafruit wykorzystujących sterownik MTK3329/MTK3339.
// Wykorzystano również bibliotekę graficzną autorstwa Olivera Krausa.
// Zmodyfikowano do pracy z wyświetlaczem OLED i dodano informacje o lokalizatorze QRA.
// Program wyświetla CZAS LOKALNY i w formacie 12/24 godzinnym.
//
// Sprzęt:
// - Arduino Nano z wyświetlaczem OLED 64 x 128 (I2C)
// - Moduł GPS GY-GPS6MV2 (e-bay) z chipsetem NEO-6M-0-001
//
// Połączenia między Nano, OLED i modułem GPS:
// - OLED I2C: A4-SDA, A5-SCL, 5Volt-Vcc
// - GPS: 5Volt-Vcc, RX0-TX.  !!!>>  Odłączyć podczas programowania <<!!!
// - 31-01-2025    Dodano kilka funkcji, optymalizacja kodu - SQ1KSM

// Inicjalizacja bibliotek
#include <Adafruit_GPS.h>       // Biblioteka do obsługi modułu GPS
#include <SoftwareSerial.h>     // Biblioteka do obsługi komunikacji szeregowej
#include <U8glib.h>             // Biblioteka do obsługi wyświetlacza OLED

// Definicja formatu czasu (24-godzinny lub 12-godzinny)
#define HU_24_H false           // Ustawienie formatu czasu (false = 12-godzinny, true = 24-godzinny)
const int HOUR_OFFSET = 0;      // Przesunięcie strefy czasowej (w godzinach)

// Zmienne do przechowywania kierunków geograficznych
char latns, lonew;

// Inicjalizacja obiektów
Adafruit_GPS GPS(&Serial);      // Obiekt do obsługi modułu GPS
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);  // Obiekt do obsługi wyświetlacza OLED

// Definicja debugowania GPS (wyświetlanie danych NMEA w konsoli)
#define GPSECHO false

// Stałe do obliczeń lokalizatora QRA
const float LONGITUDE_OFFSET = 180000000;  // Przesunięcie długości geograficznej
const float LATITUDE_OFFSET = 90000000;    // Przesunięcie szerokości geograficznej
const float GRID_SIZE_LONG = 20000000;     // Rozmiar siatki dla długości geograficznej
const float GRID_SIZE_LAT = 10000000;      // Rozmiar siatki dla szerokości geograficznej
const float SUBGRID_SIZE_LONG = 2000000;   // Rozmiar podziałki dla długości geograficznej
const float SUBGRID_SIZE_LAT = 1000000;    // Rozmiar podziałki dla szerokości geograficznej

// Funkcja setup - inicjalizacja urządzenia
void setup() {
  Serial.begin(9600);           // Inicjalizacja komunikacji szeregowej
  GPS.begin(9600);              // Inicjalizacja modułu GPS
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);  // Ustawienie formatu danych NMEA

  // Wyświetlenie ekranu startowego na OLED
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_helvR08);  // Ustawienie czcionki
    u8g.drawStr(35, 10, "GPS - FIELD");  // Nazwa urządzenia
    u8g.setFont(u8g_font_gdr20);    // Ustawienie większej czcionki
    u8g.drawStr(10, 40, "SQ1KSM");  // Wyświetlenie znkau
    u8g.setFont(u8g_font_helvR08);  // Ustawienie czcionki
    u8g.drawStr(55, 60, "Slawek");  // Wyświetlenie imienia...
  } while (u8g.nextPage());
  delay(2000);  // Opóźnienie wyświetlania ekranu startowego
}

// Zmienna do pomiaru czasu
uint32_t timer = millis();

// Główna pętla programu
void loop() {
  char c = GPS.read();  // Odczyt danych z modułu GPS
  if (GPSECHO && c) Serial.print(c);  // Debugowanie: wyświetlanie danych NMEA w konsoli

  // Sprawdzenie, czy otrzymano nową ramkę NMEA
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) return;  // Parsowanie ramki NMEA
  }

  // Aktualizacja timera
  if (timer > millis()) timer = millis();

  // Aktualizacja wyświetlacza co 1 sekundę
  if (millis() - timer > 1000) {
    timer = millis();

    // Debugowanie: wyświetlanie sekund w konsoli
    if (GPSECHO) Serial.println(GPS.seconds, DEC);

    // Wyświetlenie informacji na OLED
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
  if (element < 10) u8g.print("0");  // Dodanie zera wiodącego dla liczb jednocyfrowych
  u8g.print(element, DEC);  // Wyświetlenie liczby
}

// Funkcja wyświetlająca czas
void displaytime() {
  u8g.setFont(u8g_font_gdr20);  // Ustawienie czcionki
  u8g.setPrintPos(0, 42);       // Ustawienie pozycji kursora

  // Korekta strefy czasowej
  uint8_t hours = (GPS.hour + HOUR_OFFSET + 24) % 24;

  // Wybór formatu czasu (12-godzinny lub 24-godzinny)
  if (HU_24_H) {
    u8g.setFont(u8g_font_helvR08);  // Ustawienie czcionki
    u8g.print("   ");               // Spacja dla formatu 24-godzinnego
    u8g.setFont(u8g_font_gdr20);    // Powrót do większej czcionki
  } else {
    u8g.setFont(u8g_font_helvR08);  // Ustawienie czcionki
    u8g.print(hours > 12 ? "PM " : "AM ");  // Wyświetlenie AM/PM
    u8g.setFont(u8g_font_gdr20);    // Powrót do większej czcionki
    if (hours > 12) hours -= 12;    // Konwersja na format 12-godzinny
  }

  // Wyświetlenie czasu
  printTimeElement(hours);  // Godziny
  u8g.print(":");           // Separator
  printTimeElement(GPS.minute);  // Minuty
  u8g.print(":");           // Separator
  printTimeElement(GPS.seconds);  // Sekundy
  u8g.print(" ");           // Spacja
}

// Funkcja wyświetlająca datę
void displaydate() {
  u8g.setFont(u8g_font_helvR08);  // Ustawienie czcionki
  u8g.setPrintPos(0, 64);         // Ustawienie pozycji kursora
  u8g.print(GPS.day, DEC);        // Wyświetlenie dnia
  u8g.print('-');                 // Separator
  u8g.print(GPS.month, DEC);      // Wyświetlenie miesiąca
  u8g.print("-20");               // Prefix roku (dla lat 2000+)
  u8g.print(GPS.year, DEC);       // Wyświetlenie roku
}

// Funkcja wyświetlająca lokalizację (szerokość i długość geograficzną)
void displaylocation() {
  u8g.setFont(u8g_font_helvR08);  // Ustawienie czcionki
  u8g.setPrintPos(0, 10);         // Ustawienie pozycji kursora

  // Sprawdzenie, czy moduł GPS ma połączenie z satelitami
  if (GPS.satellites == 0) {
    u8g.print("Czekam na GPS...");  // Komunikat oczekiwania na dane GPS
  } else {
    latns = GPS.lat;  // Kierunek szerokości geograficznej (N/S)
    lonew = GPS.lon;  // Kierunek długości geograficznej (E/W)
    u8g.print(GPS.latitudeDegrees);  // Wyświetlenie szerokości geograficznej
    u8g.print(latns);                // Wyświetlenie kierunku (N/S)
    u8g.print(" - ");                // Separator
    u8g.print(GPS.longitudeDegrees); // Wyświetlenie długości geograficznej
    u8g.print(lonew);                // Wyświetlenie kierunku (E/W)
  }
}

// Funkcja wyświetlająca lokalizator QRA
void displayQRA() {
  if (GPS.satellites == 0) return;  // Pominięcie, jeśli brak danych GPS

  // Przeliczenie współrzędnych na wartości liczbowe
  float loclong = GPS.longitudeDegrees * 1000000;
  float loclat = GPS.latitudeDegrees * 1000000;

  // Korekta współrzędnych w zależności od kierunku (E/W, N/S)
  if (lonew == 'E') loclong += LONGITUDE_OFFSET;
  if (lonew == 'W') loclong = LONGITUDE_OFFSET - abs(loclong);
  if (latns == 'N') loclat += LATITUDE_OFFSET;
  if (latns == 'S') loclat = LATITUDE_OFFSET - abs(loclat);

  u8g.setFont(u8g_font_helvR08);  // Ustawienie czcionki
  u8g.setPrintPos(86, 10);        // Ustawienie pozycji kursora

  // Obliczenie i wyświetlenie lokalizatora QRA
  u8g.print(char(int(loclong / GRID_SIZE_LONG) + 'A'));  // Pierwszy znak
  u8g.print(char(int(loclat / GRID_SIZE_LAT) + 'A'));    // Drugi znak
  u8g.print(char(int((loclong - (GRID_SIZE_LONG * int(loclong / GRID_SIZE_LONG))) * 10 / 20 / 1000000) + '0'));  // Trzeci znak
  u8g.print(char(int((loclat - (GRID_SIZE_LAT * int(loclat / GRID_SIZE_LAT))) / 1000000) + '0'));  // Czwarty znak
  u8g.print(char(int(((loclong / SUBGRID_SIZE_LONG) - int(loclong / SUBGRID_SIZE_LONG)) * 24) + 'A'));  // Piąty znak
  u8g.print(char(int(((loclat / SUBGRID_SIZE_LAT) - int(loclat / SUBGRID_SIZE_LAT)) * 24) + 'A'));  // Szósty znak
}

// Funkcja wyświetlająca liczbę satelit
void displaysats() {
  u8g.setFont(u8g_font_helvR08);  // Ustawienie czcionki
  u8g.setPrintPos(70, 64);        // Ustawienie pozycji kursora
  if (GPS.satellites >= 0 && GPS.satellites < 20) {
    u8g.print("Satelit: ");       // Wyświetlenie tekstu
    u8g.print((int)GPS.satellites);  // Wyświetlenie liczby satelit
  }
}
