#include <Arduino.h>
#include <DS3232RTC.h>
#include <Wire.h>
#include <BluetoothSerial.h>
#include <GxEPD2_BW.h>
#include "esp32_pinout.h"
#include <ArduinoJson.h>
#include "esp_sleep.h"

BluetoothSerial bt;
DS3232RTC myRTC;
tmElements_t timeInfo;

struct Registro {
  int id;
  String nombre;
  String hora;
  bool status;
};
Registro registros[10];
int registroCount = 0;
int nextId = 1;

GxEPD2_BW<GxEPD2_154_D67, SCREEN_HEIGHT> display(GxEPD2_154_D67(EPD_CS, EPD_DC, EPD_RES, EPD_BUSY));

unsigned long previousMillis = 0;
const long interval = 60000;  // 1 minuto
bool showClock = false;       // Controla si se muestra el reloj
bool showList = false;        // Controla si se muestra la lista de pastillas
bool alertTriggered[10];

const int BAUDIOS = BAUD_RATE;
const char* BT_NAME = DEVICE_NAME;

// Intervalo de despertar para el ESP32 (en microsegundos)
#define WAKEUP_INTERVAL 6000000000  // 10 minutos (6000000000 us)

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

  // Configurar el temporizador de despertar cada 10 minutos
  //esp_sleep_enable_timer_wakeup(WAKEUP_INTERVAL);
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
  myRTC.read(timeInfo);

  timeInfo.Minute += 1;
  if (timeInfo.Minute >= 60) {
    timeInfo.Minute = 0;
    timeInfo.Hour += 1;
    if (timeInfo.Hour >= 24) {
      timeInfo.Hour = 0;
      timeInfo.Day += 1;
    }
  }

  myRTC.write(timeInfo);

  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(2);

  String fecha = String(timeInfo.Day) + "/" + String(timeInfo.Month) + "/" + String(timeInfo.Year + 1970);
  display.setCursor((SCREEN_WIDTH - 160) / 2, 30);
  display.print(fecha);

  int hour12 = timeInfo.Hour % 12;
  hour12 = hour12 ? hour12 : 12;
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
    display.setCursor(10, 20 + (i * 20));
    display.print(registros[i].nombre + " a las " + registros[i].hora);
  }

  display.display();

  DynamicJsonDocument doc(1024);

  JsonArray registrosArray = doc.createNestedArray("registros");

  for (int i = 0; i < registroCount; i++) {
    JsonObject pastilla = registrosArray.createNestedObject();
    pastilla["id"] = registros[i].id;
    pastilla["nombre"] = registros[i].nombre;
    pastilla["hora"] = registros[i].hora;
    pastilla["status"] = registros[i].status;
  }

  String jsonString;
  serializeJson(doc, jsonString);

  bt.println(jsonString);
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
  } else if (command.startsWith("/registrar[")) {
    String registro = command.substring(11, command.length() - 1);  // Eliminar los corchetes
    int commaIndex = registro.indexOf(',');
    if (commaIndex != -1) {
      String nombre = registro.substring(0, commaIndex);
      String hora = registro.substring(commaIndex + 1);
      if (registrarPastilla(nombre, hora)) {
        Serial.println("Pastilla registrada: " + nombre + " a las " + hora);
        displayMessage("Pastilla registrada:", nombre);
      } else {
        Serial.println("Registro de pastilla fallido.");
        displayMessage("Registro de pastilla", "fallido.");
      }
    }
  } else if (command.startsWith("/listar")) {
    showList = true;
  } else if (command.startsWith("/eliminar[")) {
    String idStr = command.substring(10, command.length() - 1);  // Eliminar corchetes
    int id = idStr.toInt();
    if (eliminarPastilla(id)) {
      Serial.println(F("Pastilla eliminada"));
    } else {
      Serial.println(F("ID no encontrado"));
    }
  } else if (command.startsWith("/actualizar[")) {
    String updateCmd = command.substring(11, command.length() - 1);  // Eliminar los corchetes
    int commaIndex = updateCmd.indexOf(',');
    if (commaIndex != -1) {
      String idStr = updateCmd.substring(0, commaIndex).substring(3);  // Eliminar 'id='
      String nombre = updateCmd.substring(commaIndex + 1).substring(7);  // Eliminar 'nombre='
      String hora = updateCmd.substring(updateCmd.lastIndexOf('=') + 1);
      int id = idStr.toInt();
      if (actualizarPastilla(id, nombre, hora)) {
        Serial.println(F("Pastilla actualizada"));
      } else {
        Serial.println(F("Error al actualizar"));
      }
    }
  } else {
    Serial.println("Comando no reconocido.");
    displayMessage("Comando no", "reconocido.");
  }
}

bool registrarPastilla(const String& nombre, const String& hora) {
  if (registroCount >= 10) {
    Serial.println(F("Límite de registros alcanzado."));
    return false;
  }

  int horas, minutos;
  if (sscanf(hora.c_str(), "%d:%d", &horas, &minutos) != 2 || horas > 23 || minutos > 59) {
    Serial.println("Hora inválida.");
    return false;
  }

  int horaInt = horas * 100 + minutos;

  int insertPos = registroCount;
  for (int i = 0; i < registroCount; i++) {
    int regHora = parseTimeToInt(registros[i].hora);
    if (horaInt < regHora) {
      insertPos = i;
      break;
    }
  }

  for (int i = registroCount; i > insertPos; i--) {
    registros[i] = registros[i - 1];
  }

  registros[insertPos] = { nextId++, nombre, hora, false };
  registroCount++;
  return true;
}

int parseTimeToInt(String time) {
  int hours, minutes;
  sscanf(time.c_str(), "%d:%d", &hours, &minutes);
  return hours * 100 + minutes;
}

bool eliminarPastilla(int id) {
  int pos = -1;
  for (int i = 0; i < registroCount; i++) {
    if (registros[i].id == id) {
      pos = i;
      break;
    }
  }
  if (pos != -1) {
    for (int i = pos; i < registroCount - 1; i++) {
      registros[i] = registros[i + 1];
    }
    registroCount--;
    return true;
  }
  return false;
}

bool actualizarPastilla(int id, const String& nombre, const String& hora) {
  int pos = -1;
  for (int i = 0; i < registroCount; i++) {
    if (registros[i].id == id) {
      pos = i;
      break;
    }
  }
  if (pos != -1) {
    registros[pos].nombre = nombre;
    registros[pos].hora = hora;
    return true;
  }
  return false;
}

void loop() {
  if (bt.available()) {
    String command = bt.readStringUntil('\n');
    processCommand(command);
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (showClock) {
      mostrarReloj();
    }

    for (int i = 0; i < registroCount; i++) {
      if (!alertTriggered[i]) {
        String currentTime = String(timeInfo.Hour) + ":" + (timeInfo.Minute < 10 ? "0" : "") + String(timeInfo.Minute);
        if (currentTime == registros[i].hora) {
          activarAlerta(i);
          alertTriggered[i] = true;
        }
      }
    }
  }
}

// Función para establecer la hora en el RTC
bool setHora(const String& hora) {
  int horas, minutos;
  if (sscanf(hora.c_str(), "%d:%d", &horas, &minutos) != 2 || horas > 23 || minutos > 59) {
    return false;  // Hora no válida
  }
  
  timeInfo.Hour = horas;
  timeInfo.Minute = minutos;
  myRTC.write(timeInfo);  // Actualiza la hora en el RTC
  return true;
}

// Función para establecer la fecha en el RTC
bool setFecha(const String& fecha) {
  int dia, mes, anio;
  if (sscanf(fecha.c_str(), "%d/%d/%d", &dia, &mes, &anio) != 3 || dia > 31 || mes > 12 || anio < 1970) {
    return false;  // Fecha no válida
  }
  
  timeInfo.Day = dia;
  timeInfo.Month = mes;
  timeInfo.Year = anio - 1970;  // El RTC usa años desde 1970
  
  myRTC.write(timeInfo);  // Actualiza la fecha en el RTC
  return true;
}

void activarAlerta(int index) {
  digitalWrite(VIB_MOTOR_PIN, HIGH);
  delay(1000);
  digitalWrite(VIB_MOTOR_PIN, LOW);
  displayMessage("¡Alerta!", "Es hora de tomar " + registros[index].nombre);
}