/*
  Proyecto: Dosificador automático de agua de 500 ml
  Microcontrolador: ESP32
  By: Luis Luva
  https://www.tiktok.com/@lab.luva?_r=1&_t=ZS-97M1VoaISzk

  Descripción:
  Este programa activa una bomba de agua al presionar un botón.
  El sensor de caudal mide la cantidad de agua que pasa por la manguera.
  Cuando el volumen llega aproximadamente a 500 ml, la bomba se apaga automáticamente.

  Componentes:
  - ESP32
  - Sensor de caudal YF-S201 o similar
  - Bomba de agua DC
  - Módulo relé o MOSFET para controlar la bomba
  - Pantalla LCD 16x2 con módulo I2C
  - Botón pulsador

  Conexiones sugeridas:
  Sensor de caudal señal -> GPIO 27
  Botón                 -> GPIO 14
  Relé/MOSFET bomba     -> GPIO 26
  LCD SDA               -> GPIO 21
  LCD SCL               -> GPIO 22
  VCC LCD               -> 5V o 3.3V según el módulo LCD
  GND                   -> GND

  Nota:
  Si el sensor de caudal trabaja a 5V, revisar que la salida de señal sea segura para el ESP32.
  El ESP32 trabaja con lógica de 3.3V.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Dirección I2C del LCD.
// Si tu LCD no muestra nada, prueba cambiando 0x27 por 0x3F.
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pines del ESP32
const byte pinSensorCaudal = 27;  // Pin para contar pulsos del sensor
const byte pinBoton = 14;         // Botón para iniciar la dosificación
const byte pinBomba = 26;         // Salida para controlar relé o MOSFET de la bomba

// Pines I2C del ESP32
const byte pinSDA = 21;
const byte pinSCL = 22;

// Volumen objetivo en mililitros.
// En este caso el sistema se detiene al llegar aproximadamente a 500 ml.
const float volumenObjetivoML = 500.0;

// Factor de calibración del sensor de caudal.
// Este valor depende del modelo del sensor, la presión del agua y la instalación.
// Para sensores tipo YF-S201 se suele usar 450 pulsos por litro como valor inicial.
// Para mayor precisión, este valor debe calibrarse con una prueba real.
const float pulsosPorLitro = 450.0;

/*
  Fórmula de calibración:

  pulsosPorLitro = pulsosContados / litrosReales

  Ejemplo:
  Si al llenar 500 ml el sensor contó 230 pulsos:

  pulsosPorLitro = 230 / 0.5
  pulsosPorLitro = 460

  Entonces se reemplaza el valor:
  const float pulsosPorLitro = 460.0;
*/

// Variable que aumenta cada vez que el sensor detecta un pulso
volatile unsigned long pulsos = 0;

// Variable donde se calcula el volumen de agua en mililitros
float volumenML = 0;

// Estado del sistema
bool dosificando = false;

// Variable para detectar la pulsación del botón
bool botonAnterior = HIGH;

// Configuración del relé.
// Muchos módulos relé se activan con LOW.
// Si tu relé se activa con HIGH, cambia este valor a false.
const bool releActivoEnLOW = true;

// Función de interrupción para ESP32.
// IRAM_ATTR ayuda a que la interrupción trabaje correctamente en ESP32.
void IRAM_ATTR contarPulsos() {
  pulsos++;
}

// Enciende la bomba según el tipo de relé usado
void encenderBomba() {
  if (releActivoEnLOW) {
    digitalWrite(pinBomba, LOW);
  } else {
    digitalWrite(pinBomba, HIGH);
  }
}

// Apaga la bomba según el tipo de relé usado
void apagarBomba() {
  if (releActivoEnLOW) {
    digitalWrite(pinBomba, HIGH);
  } else {
    digitalWrite(pinBomba, LOW);
  }
}

void setup() {
  // Monitor serial para pruebas y depuración
  Serial.begin(115200);

  // Configuración de pines
  pinMode(pinSensorCaudal, INPUT_PULLUP);
  pinMode(pinBoton, INPUT_PULLUP);
  pinMode(pinBomba, OUTPUT);

  // Asegura que la bomba inicie apagada
  apagarBomba();

  // Inicialización del bus I2C en ESP32
  Wire.begin(pinSDA, pinSCL);

  // Inicialización del LCD
  lcd.init();
  lcd.backlight();

  // Configuración de interrupción para el sensor de caudal
  attachInterrupt(digitalPinToInterrupt(pinSensorCaudal), contarPulsos, RISING);

  // Mensaje inicial
  lcd.setCursor(0, 0);
  lcd.print("Dosificador");
  lcd.setCursor(0, 1);
  lcd.print("ESP32");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Presione boton");
  lcd.setCursor(0, 1);
  lcd.print("Objetivo:500ml");

  Serial.println("Sistema listo");
}

void loop() {
  // Lee el estado actual del botón
  bool estadoBoton = digitalRead(pinBoton);

  // Detecta cuando el botón pasa de no presionado a presionado
  if (botonAnterior == HIGH && estadoBoton == LOW && !dosificando) {
    iniciarDosificacion();
    delay(200); // Pequeño antirrebote
  }

  botonAnterior = estadoBoton;

  // Si el sistema está dosificando, calcula el volumen
  if (dosificando) {
    // Copia segura de la variable pulsos
    noInterrupts();
    unsigned long pulsosActuales = pulsos;
    interrupts();

    // Conversión de pulsos a mililitros
    volumenML = (pulsosActuales / pulsosPorLitro) * 1000.0;

    // Muestra información en LCD
    lcd.setCursor(0, 0);
    lcd.print("Dosificando... ");

    lcd.setCursor(0, 1);
    lcd.print("Vol:");
    lcd.print(volumenML, 0);
    lcd.print(" ml     ");

    // También muestra datos en el monitor serial
    Serial.print("Pulsos: ");
    Serial.print(pulsosActuales);
    Serial.print(" | Volumen: ");
    Serial.print(volumenML);
    Serial.println(" ml");

    // Si llega al volumen objetivo, apaga la bomba
    if (volumenML >= volumenObjetivoML) {
      apagarBomba();
      dosificando = false;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Completado!");
      lcd.setCursor(0, 1);
      lcd.print("Total:");
      lcd.print(volumenML, 0);
      lcd.print(" ml");

      Serial.println("Dosificacion completada");

      delay(3000);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Presione boton");
      lcd.setCursor(0, 1);
      lcd.print("Objetivo:500ml");
    }
  }
}

// Función para iniciar una nueva dosificación
void iniciarDosificacion() {
  // Reinicia las variables de medición
  pulsos = 0;
  volumenML = 0;
  dosificando = true;

  // Mensaje de inicio
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");
  lcd.setCursor(0, 1);
  lcd.print("Bomba ON");

  Serial.println("Iniciando dosificacion");

  // Activa la bomba
  encenderBomba();

  delay(500);
}
