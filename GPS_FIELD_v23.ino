// GPS_Display
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
// - 26-01-2025    Dodano kilka funkcji, optymalizacja kodu - SQ1KSM

#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <U8glib.h>

// Ustaw na false, aby wyświetlać czas w formacie 12-godzinnym, lub true, aby używać formatu 24-godzinnego:
#define HU_24_H true

// Zmień tę wartość, aby przesunąć godziny od UTC (czasu uniwersalnego) do lokalnego czasu.
const int HOUR_OFFSET = 0;
//const int OLED_RESET = 4;

char latns, lonew;  // zapisz wskaźniki lokalizacji, aby zapobiec problemom z synchronizacją

// Inicjalizacja sprzętu
Adafruit_GPS GPS(&Serial);                      // Sprzętowy serial wykorzystujący Rx0
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);  // Wyświetlacz, który nie wysyła ACK

// Ustaw GPSECHO na 'false', aby wyłączyć przesyłanie danych GPS do konsoli szeregowej
// Ustaw na 'true', jeśli chcesz debugować i słuchać surowych informacji GPS.
#define GPSECHO false
void setup() {
  // Połącz z prędkością 9600, aby odczytać dane GPS wystarczająco szybko i przesłać je bez gubienia znaków
  // a także wypisz dane
  Serial.begin(9600);
  // 9600 NMEA to domyślna prędkość transmisji dla modułów GPS Adafruit MTK - niektóre używają 4800
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);  // zmniejsza do niezbędnego minimum wysyłane dane - nowa funkcja! (SQ1KSM)

  // Ekran powitalny
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_helvR08);
    u8g.drawStr(35, 10, "GPS - FIELD");
    u8g.setFont(u8g_font_gdr20);    // Użyj dużej czcionki dla podanego znaku
    u8g.drawStr(10, 40, "SQ1KSM");  // Wpisz swój znak
    u8g.setFont(u8g_font_helvR08);  // Powrót do domyślnej czcionki
    u8g.drawStr(55, 60, "Slawek");  // Wpisz swoje imie
  } while (u8g.nextPage());
  delay(2000);  // Czekaj 2 sekundy
}

uint32_t timer = millis();

//************************** P Ę T L A   **************************************************
void loop()  // wykonywane w nieskończoność
{
  // odczytaj dane z GPS w głównej pętli
  char c = GPS.read();
  // jeśli chcesz debugować, to dobry moment, aby to zrobić!
  if (GPSECHO)
    if (c) Serial.print(c);

  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) return;
  }

  // jeśli millis() lub timer się przewijają, po prostu je zresetuj
  if (timer > millis()) timer = millis();

  // mniej więcej co 1 sekundę wyświetl bieżące statystyki
  if (millis() - timer > 1000) {
    timer = millis();  // zresetuj timer

    if (GPSECHO)
      Serial.println(GPS.seconds, DEC);

    // Pętla rysowania obrazu
    u8g.firstPage();
    do {
      displaytime();
      displaydate();
      displaylocation();
      displaysats();
      displayQRA();
    } while (u8g.nextPage());
  }
}
/******************************************************************
 *   W Y Ś W I E T L A N I E
 *   
 *   Wyświetla dane o czasie, dacie i lokalizacji
 *   Czas jest skorygowany o strefę czasową i format 12/24 godzinny
 ******************************************************************/
void printTimeElement(uint8_t element) {
  if (element < 10) u8g.print("0");
  u8g.print(element, DEC);
}

void displaytime() {
  u8g.setFont(u8g_font_gdr20);
  u8g.setPrintPos(10, 42);

  // Korekta strefy czasowej
  uint8_t hours = GPS.hour + HOUR_OFFSET;  // Dodaj przesunięcie godzin, aby przekształcić UTC na czas lokalny.

  // Obsługa sytuacji, gdy UTC + przesunięcie wychodzi poza zakres (wartość ujemna lub > 23).
  if (GPS.hour < 0) {
    hours = 24 + GPS.hour;
  }
  if (GPS.hour > 23) {
    hours = 24 - GPS.hour;
  }

  // Konwersja z formatu 24-godzinnego na 12-godzinny, jeśli wymagane.
  if (!HU_24_H) {
    // Obsługa godzin powyżej 12 przez odjęcie 12 godzin.
    if (GPS.hour > 12) {
      hours -= 12;
    }
    // Obsługa godziny 0 (północ), aby wyświetlała 12.
    else if (GPS.hour == 0) {
      hours += 12;
    }
  }

  printTimeElement(hours);
  u8g.print(":");
  printTimeElement(GPS.minute);
  u8g.print(":");
  printTimeElement(GPS.seconds);
  u8g.print(" ");
}

void displaydate() {
  // Wyświetlanie daty
  u8g.setFont(u8g_font_helvR08);
  u8g.setPrintPos(0, 64);
  u8g.print(GPS.day, DEC);
  u8g.print('-');
  u8g.print(GPS.month, DEC);
  u8g.print("-20");
  u8g.print(GPS.year, DEC);
}

void displaylocation() {
  // Wyświetlanie lokalizacji
  u8g.setFont(u8g_font_helvR08);
  u8g.setPrintPos(0, 10);

  if (GPS.satellites == 0) {
    // Jeśli brak satelitów, wyświetl " -- -- "
    u8g.print("Czekam na GPS...");
  } else {
    // Wyświetlanie rzeczywistych współrzędnych
    latns = GPS.lat;
    lonew = GPS.lon;
    u8g.print(GPS.latitudeDegrees);
    u8g.print(latns);
    u8g.print(" - ");
    u8g.print(GPS.longitudeDegrees);
    u8g.print(lonew);
  }
}

void displayQRA() {
  // Sprawdzenie, czy liczba satelitów jest większa od 0
  if (GPS.satellites == 0) {
    return;  // Nie wyświetlaj lokalizatora QRA, jeśli brak satelitów
  }

  // Wyświetlanie lokalizatora QRA
  float loclong = GPS.longitudeDegrees * 1000000;  // Przeliczanie długości geograficznej na całkowite liczby
  float loclat = GPS.latitudeDegrees * 1000000;    // Przeliczanie szerokości geograficznej na całkowite liczby
  float scrap;
  char char_string[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";  // Litery używane w lokalizatorze QRA
  char num_string[] = "0123456789";                   // Cyfry używane w lokalizatorze QRA

  // Korekta dla długości geograficznej wschodniej
  if (lonew == 'E') {
    loclong = (loclong) + 180000000;  // Dodanie korekty dla południka 0
  }
  // Korekta dla długości geograficznej zachodniej
  if (lonew == 'W') {
    loclong = 180000000 - (abs(loclong));  // Odjęcie wartości absolutnej długości geograficznej od 180000000
  }

  // Korekta dla szerokości geograficznej północnej
  if (latns == 'N') {
    loclat = loclat + 90000000;  // Dodanie korekty dla równika
  }
  // Korekta dla szerokości geograficznej południowej
  if (latns == 'S') {
    loclat = 90000000 - (abs(loclat));  // Odjęcie wartości absolutnej szerokości geograficznej od 90000000
  }

  u8g.setFont(u8g_font_helvR08);
  u8g.setPrintPos(86, 10);
  // Pierwszy znak - oparty na długości geograficznej (co 20° = 1 kwadrat siatki)
  u8g.print(char_string[int(loclong / 20000000)]);

  // Drugi znak - oparty na szerokości geograficznej (co 10° = 1 kwadrat siatki)
  u8g.print(char_string[int(loclat / 10000000)]);

  // Trzeci znak - oparty na długości geograficznej (co 2° = 1 kwadrat siatki)
  scrap = loclong - (20000000 * int(loclong / 20000000));
  u8g.print(num_string[int(scrap * 10 / 20 / 1000000)]);

  // Czwarty znak - oparty na szerokości geograficznej (co 1° = 1 kwadrat siatki)
  scrap = loclat - (10000000 * int(loclat / 10000000));
  u8g.print(num_string[int(scrap / 1000000)]);

  // Piąty znak - oparty na długości geograficznej (co 5' = 1 kwadrat siatki)
  scrap = (loclong / 2000000) - (int(loclong / 2000000));
  u8g.print(char_string[int(scrap * 24)]);

  // Szósty znak - oparty na szerokości geograficznej (co 2.5' = 1 kwadrat siatki)
  scrap = (loclat / 1000000) - (int(loclat / 1000000));
  u8g.print(char_string[int(scrap * 24)]);
}

void displaysats() {
  // Wyświetlanie liczby satelitów
  u8g.setFont(u8g_font_helvR08);
  u8g.setPrintPos(70, 64);
  if (GPS.satellites >= 0 && GPS.satellites < 20) {
    u8g.print("Satelit: ");
    u8g.print((int)GPS.satellites);
  }
}
