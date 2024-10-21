#include <Arduino.h>
#include <DS3232RTC.h>
#include <Wire.h>
#include <BluetoothSerial.h>
#include <GxEPD2_BW.h>
#include "esp32_pinout.h"

BluetoothSerial bt;
DS3232RTC myRTC;
tmElements_t timeInfo;

// Estructura para registros de pastillas
struct Registro {
  String nombre;
  String hora;  // Hora en formato HH:MM
};
Registro registros[10];
int registroCount = 0;

GxEPD2_BW<GxEPD2_154_D67, SCREEN_HEIGHT> display(GxEPD2_154_D67(EPD_CS, EPD_DC, EPD_RES, EPD_BUSY));

unsigned long previousMillis = 0;
const long interval = 60000;  // 1 minuto
bool showClock = false;
bool showList = false;    // Indica si se debe mostrar la lista de pastillas
bool alertTriggered[10];  // Para rastrear alertas de pastillas

void setup() {
  Serial.begin(BAUDIOS);
  bt.begin(BT_NAME);
  myRTC.begin();
  display.init();

  pinMode(VIB_MOTOR_PIN, OUTPUT);

  setDefaultTime();
  displayMessage("¡Inicializado!", "Fecha y hora por defecto establecida.");

  delay(2000);
  showClock = true;  // Activar la visualización del reloj
}

void setDefaultTime() {
  timeInfo.Year = 2024 - 1970;
  timeInfo.Month = 1;
  timeInfo.Day = 1;
  timeInfo.Hour = 0;
  timeInfo.Minute = 0;
  timeInfo.Second = 0;
  myRTC.write(timeInfo);
}

void displayMessage(const String& line1, const String& line2) {
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print(line1);
  display.setCursor(0, 50);
  display.print(line2);
  display.display();
  delay(2000);
}

void mostrarReloj() {
  // Leer la hora actual del RTC
  myRTC.read(timeInfo);

  // Sumar un minuto
  timeInfo.Minute += 1;
  if (timeInfo.Minute >= 60) {
    timeInfo.Minute = 0;
    timeInfo.Hour += 1;
    if (timeInfo.Hour >= 24) {
      timeInfo.Hour = 0;
      timeInfo.Day += 1;
      // Aquí puedes manejar el cambio de mes y año si es necesario
    }
  }

  // Actualizar el RTC
  myRTC.write(timeInfo);

  // Mostrar la fecha y hora en la pantalla
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(2);

  String fecha = String(timeInfo.Day) + "/" + String(timeInfo.Month) + "/" + String(timeInfo.Year + 1970);
  display.setCursor((SCREEN_WIDTH - 160) / 2, 30);
  display.print(fecha);

  // Formatear hora en AM/PM
  int hour12 = timeInfo.Hour % 12;  // Convertir a formato 12 horas
  hour12 = hour12 ? hour12 : 12;    // Si es 0, cambiar a 12
  String horaFormateada = String(hour12) + ":" + (timeInfo.Minute < 10 ? "0" : "") + String(timeInfo.Minute) + (timeInfo.Hour >= 12 ? " PM" : " AM");

  display.setCursor((SCREEN_WIDTH - 160) / 2, 70);
  display.print(horaFormateada);
  display.drawBitmap(0, 90, logo, SCREEN_WIDTH, SCREEN_HEIGHT, GxEPD_BLACK);
  
  display.display();
}

void listarPastillas() {
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(2);

  for (int i = 0; i < registroCount; i++) {
    display.setCursor(10, 20 + (i * 20));  // Ajustar la posición de cada línea
    display.print(registros[i].nombre + " a las " + registros[i].hora);
  }

  display.display();
}

void processCommand(String command) {
  if (command.startsWith("/setHora ")) {
    String hora = command.substring(9);
    if (setHora(hora)) {
      Serial.println("Hora actualizada: " + hora);
      displayMessage("Hora actualizada:", hora);
    } else {
      Serial.println("Formato de hora inválido.");
      displayMessage("Formato de hora", "inválido.");
    }
  } else if (command.startsWith("/setFecha ")) {
    String fecha = command.substring(10);
    if (setFecha(fecha)) {
      Serial.println("Fecha actualizada: " + fecha);
      displayMessage("Fecha actualizada:", fecha);
    } else {
      Serial.println("Formato de fecha inválido.");
      displayMessage("Formato de fecha", "inválido.");
    }
  } else if (command.startsWith("/registrar ")) {
    String registro = command.substring(12);
    int spaceIndex = registro.indexOf(' ');
    if (spaceIndex != -1) {
      String nombre = registro.substring(0, spaceIndex);
      String hora = registro.substring(spaceIndex + 1);
      if (registrarPastilla(nombre, hora)) {
        Serial.println("Pastilla registrada: " + nombre + " a las " + hora);
        displayMessage("Pastilla registrada:", nombre);
      } else {
        Serial.println("Registro de pastilla fallido.");
        displayMessage("Registro de pastilla", "fallido.");
      }
    }
  } else if (command.startsWith("/listar")) {
    showList = true;  // Activa la visualización de la lista de pastillas
  } else {
    Serial.println("Comando no reconocido.");
    displayMessage("Comando no", "reconocido.");
  }
}

bool setHora(String hora) {
  int horas, minutos;
  char periodo[3];
  if (sscanf(hora.c_str(), "%d:%d %2s", &horas, &minutos, periodo) == 3) {
    if (strcmp(periodo, "AM") == 0 && horas == 12) {
      horas = 0;
    } else if (strcmp(periodo, "PM") == 0 && horas != 12) {
      horas += 12;
    }
    timeInfo.Hour = horas;
    timeInfo.Minute = minutos;
    myRTC.write(timeInfo);
    vibrate(250);
    return true;
  }
  return false;
}

bool setFecha(String fecha) {
  int dia, mes, anio;
  if (sscanf(fecha.c_str(), "%d/%d/%d", &dia, &mes, &anio) == 3) {
    timeInfo.Day = dia;
    timeInfo.Month = mes;
    timeInfo.Year = anio - 1970;
    myRTC.write(timeInfo);
    vibrate(250);
    return true;
  }
  return false;
}

bool registrarPastilla(String nombre, String hora) {
  if (registroCount < 10) {
    registros[registroCount].nombre = nombre;
    registros[registroCount].hora = hora;
    alertTriggered[registroCount] = false;  // Reiniciar alerta para esta pastilla
    registroCount++;
    vibrate(250);
    return true;
  }
  return false;
}

bool alertIfMedicationDue(int index) {
  // Extraer la hora y minuto del registro
  int regHour, regMinute;
  sscanf(registros[index].hora.c_str(), "%d:%d", &regHour, &regMinute);

  // Calcular los minutos restantes hasta la hora de la pastilla
  int currentHour = timeInfo.Hour;
  int currentMinute = timeInfo.Minute;
  int remainingMinutes = (regHour - currentHour) * 60 + (regMinute - currentMinute);

  // Verificar si el tiempo restante es 0 o está entre 0 y 15 minutos
  if (remainingMinutes <= 15 && remainingMinutes >= 0) {
    // Mensaje para cuando el tiempo restante es 0
    String message = (remainingMinutes == 0)
                       ? "¡Es hora de tomar: " + registros[index].nombre + "!"
                       : "Pastilla: " + registros[index].nombre + " en " + String(remainingMinutes) + " min";

    displayMessage("¡Alerta!", message);
    vibrate(250);
    return true;  // Indica que se ha activado una alerta
  }
  return false;  // No se activó alerta
}

void vibrate(int duration) {
  digitalWrite(VIB_MOTOR_PIN, true);  // Habilita el motor de vibración
  delay(duration);                     // Vibre por la duración especificada
  digitalWrite(VIB_MOTOR_PIN, false); // Desactiva el motor de vibración
  delay(500);                          // Espera medio segundo
}

void loop() {
  if (bt.available()) {
    String command = bt.readStringUntil('\n');
    processCommand(command);
  }

  if (showClock) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      mostrarReloj();  // Actualizar el reloj y el RTC

      for (int i = 0; i < registroCount; i++) {
        alertIfMedicationDue(i);  // Verifica todas las pastillas registradas
      }

      if (showList) {
        listarPastillas();  // Listar pastillas si se ha activado
        showList = false;   // Desactivar la visualización de la lista después de mostrarla
      }
    }
  }

  delay(100);
}