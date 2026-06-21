# Dosificador automático de agua con Arduino Nano y ESP32

Este proyecto permite dosificar aproximadamente **500 ml de agua** de forma automática usando un microcontrolador, una bomba de agua, un sensor de caudal, una pantalla LCD 16x2 y un botón.

El prototipo fue probado inicialmente con **Arduino Nano**, pero también se incluye una versión adaptada para **ESP32**.

## Funcionamiento

Al presionar el botón, la bomba de agua se activa y empieza a enviar agua hacia el recipiente de prueba.

El sensor de caudal mide la cantidad de agua que pasa por la manguera, mientras que la pantalla LCD muestra el volumen en tiempo real.

Cuando el sistema detecta que se alcanzaron aproximadamente **500 ml**, la bomba se detiene automáticamente.

## Componentes utilizados

- Arduino Nano o ESP32
- Sensor de caudal de agua, por ejemplo YF-S201 o similar
- Bomba de agua DC
- Pantalla LCD 16x2 con módulo I2C
- Botón pulsador
- Módulo relé o MOSFET para controlar la bomba
- Fuente de alimentación para la bomba
- Cables jumper
- Manguera
- Recipiente de agua

## Versiones incluidas

Este repositorio incluye dos versiones del código:

```text
Arduino_Nano/dosificador_arduino_nano.ino
ESP32/dosificador_esp32.ino
