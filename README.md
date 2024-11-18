# 🕒 CRONO VIDA

**CRONO VIDA** es un dispositivo wearable basado en el microcontrolador **ESP32-PICO-D4** que ayuda a gestionar horarios de medicación. Utiliza una pantalla E-Paper para mostrar la hora y emite recordatorios vibrantes. Permite interacción vía **Bluetooth** para registrar horarios y ajustar configuraciones, promoviendo una vida más organizada y saludable.

### Síguenos en Facebook

Puedes seguirnos en Facebook para más actualizaciones y noticias en nuestra página oficial:

[Visítanos en Facebook](https://www.facebook.com/profile.php?id=61561359332791&mibextid=ZbWKwL)

## 🔧 Componentes

Este proyecto se basa en **Watchy 2.0** y utiliza los siguientes componentes:

| Componente                               | Descripción                        |
|------------------------------------------|------------------------------------|
| 🛠️ Microcontrolador ESP32-PICO-D4       | Controlador principal              |
| 💻 USB-Serial CP2104                    | Conversor USB a Serial             |
| 🖥️ E-Paper Display GDEH0154D67         | Pantalla de bajo consumo           |
| 🔌 Display Connector AFC07-S24ECC-00    | Conector de pantalla               |
| 📏 3-Axis Accelerometer BMA423          | Sensor de movimiento               |
| ⏰ Real Time Clock DS3231               | Módulo de reloj en tiempo real     |
| 🔋 Battery LiPo 3.7V 200mAh 402030      | Fuente de alimentación             |

## 📚 Librerías

El proyecto utiliza las siguientes librerías:

- `Arduino.h`
- `DS3232RTC.h`
- `Wire.h`
- `BluetoothSerial.h`
- `GxEPD2_BW.h`

### Permisos requeridos

Para que la aplicación funcione correctamente, es necesario agregar los siguientes permisos en el archivo `AndroidManifest.xml`:

#### Permisos de Bluetooth:
```xml
<uses-permission android:name="android.permission.BLUETOOTH" />
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN" />
<uses-permission android:name="android.permission.BLUETOOTH_CONNECT" />
<uses-permission android:name="android.permission.BLUETOOTH_SCAN" />
<uses-permission android:name="android.permission.BLUETOOTH_ADVERTISE" />

## 🚀 Funcionamiento

**CRONO VIDA** permite a los usuarios gestionar sus horarios de medicación de la siguiente manera:

- **Visualización de Hora y Fecha**: Muestra la hora actual y la fecha en la pantalla E-Paper, actualizándose cada minuto.
- **Registro de Medicamentos**: Los usuarios pueden registrar horarios de toma de medicamentos a través de comandos enviados por Bluetooth.
- **Alertas de Medicación**: Emite vibraciones para recordar a los usuarios cuando es hora de tomar sus medicamentos.
- **Interacción Simple**: Los usuarios pueden ajustar la hora y la fecha, así como consultar la lista de medicamentos registrados mediante comandos específicos.

## 📋 Cómo Usar

1. **Conectar el dispositivo**: Asegúrate de que la batería esté cargada y el dispositivo esté encendido.
2. **Usar Bluetooth**: Conéctate al dispositivo usando un terminal Bluetooth para enviar comandos.
3. **Registrar Medicamentos**: Usa el comando `/registrar <nombre> <hora>` para añadir un medicamento.
4. **Consultar Hora**: La hora se muestra automáticamente en la pantalla, actualizándose cada minuto.
5. **Recibir Alertas**: El dispositivo vibrará para recordarte cuando sea hora de tomar un medicamento registrado.

## 📍 Pinout del ESP32-PICO-D4

| Función                          | Pin  | Descripción                           |
|----------------------------------|------|---------------------------------------|
| **Baudrate**                     | -    | 9600                                  |
| **Nombre Bluetooth**             | -    | "ESP-CLOCK"                           |
| **E-Paper Display GDEH0154D67** |      |                                       |
| Chip Select                      | 5    | Selección del chip                    |
| Reset                            | 9    | Reinicio                              |
| Data/Command                     | 10   | Comando de datos                      |
| Clock                            | 18   | Reloj                                 |
| Busy                             | 19   | Ocupado                               |
| Master Out Slave In (MOSI)      | 23   | Datos del maestro al esclavo          |
| **Acelerómetro BMA423**         |      |                                       |
| Interrupción 1                   | 14   | Primera interrupción                  |
| Interrupción 2                   | 12   | Segunda interrupción                  |
| Serial Data Line (SDA)          | 21   | Línea de datos serial                 |
| Serial Clock Line (SCL)         | 22   | Línea de reloj serial                 |
| **Reloj en Tiempo Real DS3231** |      |                                       |
| SDA (compartido)                | 21   | Línea de datos serial (compartido)    |
| SCL (compartido)                | 22   | Línea de reloj (compartido)           |
| **Botones**                     |      |                                       |
| Botón 1                          | 26   | Primer botón                          |
| Botón 2                          | 25   | Segundo botón                         |
| Botón 3                          | 32   | Tercer botón                          |
| Botón 4                          | 4    | Cuarto botón                          |
| **Buzzer**                       | 13   | Motor de vibración                    |
| **ADC**                          | 33   | Entrada analógica                     |
| **UART**                         |      |                                       |
| UART TX                          | 1    | Transmisión UART                      |
| UART RX                          | 3    | Recepción UART                        |
| **Dimensiones de la Pantalla**  |      |                                       |
| Altura                           | -    | 200                                   |
| Ancho                            | -    | 200                                   |
| **Acceso AP**                    |      |                                       |
| Nombre de AP                     | -    | "EspClock"                            |
| Contraseña de AP                 | -    | "12345"                               |

## 📊 Resultados

A continuación se muestran algunas imágenes de cómo quedó el proyecto final:

### Diagrama de Bloques

![Diagrama de Bloques](watchy-block-diagram2.webp)

*Este diagrama muestra la estructura y la conexión de los componentes del proyecto.*

### Diseño del Reloj

![Diseño del Reloj](watchy-esp32-smart-watch-open-source-design.webp)

*Esta imagen muestra el diseño final del reloj utilizando la pantalla E-Paper y el ESP32.*

## 📋 Cómo se usa

Este sistema permite controlar y registrar pastillas mediante comandos enviados por Bluetooth. Los siguientes comandos están disponibles para interactuar con el sistema:

### Comandos disponibles:

1. **🕒 `/setHora <hora en formato 24 horas>`**  
   Este comando te permite actualizar la hora actual del sistema. La hora debe estar en formato de 24 horas.  
   - **Ejemplo:**  
     ```cpp
     String comandoSetHora = "/setHora 15:30";
     processCommand(comandoSetHora);  // Establecer hora a las 15:30
     ```

2. **📅 `/setFecha <día/mes/año>`**  
   Este comando establece la fecha actual del sistema. El formato es `día/mes/año`.  
   - **Ejemplo:**  
     ```cpp
     String comandoSetFecha = "/setFecha 25/12/2024";
     processCommand(comandoSetFecha);  // Establecer fecha a 25 de diciembre de 2024
     ```

3. **💊 `/registrar[<nombre>,<hora en formato 24 horas>]`**  
   Este comando registra una nueva pastilla con su nombre y la hora en la que debe tomarse. El formato de la hora debe ser de 24 horas.  
   - **Ejemplo:**  
     ```cpp
     String comandoRegistrar = "/registrar[MedicamentoA,08:00]";
     processCommand(comandoRegistrar);  // Registrar la pastilla "MedicamentoA" a las 08:00
     ```

4. **📜 `/listar`**  
   Este comando muestra una lista de todas las pastillas registradas junto con sus horas programadas.  
   - **Ejemplo:**  
     ```cpp
     String comandoListar = "/listar";
     processCommand(comandoListar);  // Listar todas las pastillas registradas
     ```

5. **❌ `/eliminar[<id>]`**  
   Este comando elimina una pastilla registrada por su ID.  
   - **Ejemplo:**  
     ```cpp
     String comandoEliminar = "/eliminar[1]";  // Eliminar la pastilla con ID 1
     processCommand(comandoEliminar);
     ```

6. **🔄 `/actualizar[id=<id>,nombre=<nombre>,hora=<hora en formato 24 horas>]`**  
   Este comando actualiza el nombre o la hora de una pastilla registrada mediante su ID.  
   - **Ejemplo:**  
     ```cpp
     String comandoActualizar = "/actualizar[id=1,nombre=A,hora=9:43]";  // Actualizar la pastilla con ID 1
     processCommand(comandoActualizar);
     ```

### Notas adicionales:

- El sistema puede manejar hasta **10 pastillas** registradas al mismo tiempo.
- Cada minuto, el sistema verifica si hay alguna pastilla programada dentro de los próximos 15 minutos. Si es así, se activará una **alerta** con vibración y una notificación en pantalla.

## 🤝 Contribuciones

Si deseas contribuir a este proyecto, siéntete libre de hacer un fork y enviar pull requests.  
Agradecimientos especiales a los siguientes colaboradores:  
- **JonathanMM03** - Desarrollador principal, responsable de la implementación y mantenimiento del código.

## 📄 Licencia

Este proyecto está bajo la Licencia MIT.

## 📜 Cita

Aggarwal, P. (2021, 7 agosto). Watchy: an open source E-Paper display watch. Pallav Aggarwal. [https://pallavaggarwal.in/2021/07/04/watchy-epaper-display-esp32-smart-watch/](https://pallavaggarwal.in/2021/07/04/watchy-epaper-display-esp32-smart-watch/)

---

Para la producción de las piezas KP3s de 18 cm³, se empleó un riguroso proceso de fabricación aditiva. Se seleccionó el material PETG+ por su excelente resistencia química, impacto y flexibilidad, propiedades ideales para las aplicaciones previstas. Los filamentos PETG+, tanto en color blanco como negro, fueron adquiridos de 3D Market, un proveedor mexicano reconocido por la alta calidad de sus materiales. Con un diámetro de 1.75 mm y una tolerancia de ±0.02 mm, estos filamentos garantizaron una extrusión precisa y consistente durante el proceso de impresión.

El diseño de las piezas KP3s se modeló meticulosamente en SolidWorks, software CAD de referencia en la industria. Se prestó especial atención a los detalles geométricos y tolerancias dimensionales para asegurar un correcto ensamblaje y funcionamiento de las piezas finales. Una vez finalizado el diseño, el modelo 3D se exportó a Cura 5.3.0, donde se configuraron los parámetros de impresión, como velocidad de impresión, temperatura de extrusión, entre otros. La impresión se realizó en una impresora Creality Ender 3, garantizando la máxima precisión.

---

Este es el formato de tu README en Markdown actualizado con toda la información proporcionada.
