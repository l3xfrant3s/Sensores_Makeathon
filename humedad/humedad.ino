#include <dht11.h>
#define DHT11PIN 4          // Pin D2 (GPIO 4) para el DHT11
#define ANALOG_PIN A0       // Pin analógico A0
#define VOLTAGE 220.0       // Voltaje del circuito en voltios
#define POWER_FACTOR 1.0    // Factor de potencia

dht11 DHT11;

void setup() {
  Serial.begin(115200);  // Velocidad alta para manejar la salida
  pinMode(ANALOG_PIN, INPUT);
  Serial.println("Iniciando...");
}

void loop() {
  // Variables para el muestreo analógico
  int minValue = 1023;  // Inicializamos en el máximo posible
  int maxValue = 0;     // Inicializamos en el mínimo posible
  const int SAMPLES = 1000;  // 1000 muestras por segundo
  unsigned long startTime = micros();  // Tiempo inicial en microsegundos
  unsigned long sampleInterval = 1000; // 1000 µs = 1 ms por muestra

  // Tomar 1000 muestras a 1000 Hz y actualizar min/max directamente
  for (int i = 0; i < SAMPLES; i++) {
    while (micros() - startTime < i * sampleInterval) {
      // Esperar hasta el próximo intervalo de muestreo
    }
    int currentValue = analogRead(ANALOG_PIN);  // Leer el pin A0
    if (currentValue < minValue) minValue = currentValue;  // Actualizar mínimo
    if (currentValue > maxValue) maxValue = currentValue;  // Actualizar máximo
  }

  // Convertir valores a corriente (en amperios) para ACS712-20A
  float minCurrent = ((minValue / 1023.0) - 0.5) * 40.0;  // Mapeo de 0-1023 a -20A a +20A
  float maxCurrent = ((maxValue / 1023.0) - 0.5) * 40.0;  // Ajustado para ±20A

  // Calcular potencia en vatios con factor de potencia (P = V * I * FP)
  float minPower = minCurrent * VOLTAGE * POWER_FACTOR;  // Potencia mínima
  float maxPower = maxCurrent * VOLTAGE * POWER_FACTOR;  // Potencia máxima
  float rangePower = maxPower - minPower;                // Rango de potencia

  // Leer el DHT11
  int chk = DHT11.read(DHT11PIN);
  float humidity = (float)DHT11.humidity;
  float temperature = (float)DHT11.temperature;

  // Mostrar resultados cada segundo
  Serial.println("\n--- Medición (1 segundo) ---");
  Serial.print("Corriente mínima: ");
  Serial.print(minCurrent, 2);
  Serial.println(" A");
  Serial.print("Corriente máxima: ");
  Serial.print(maxCurrent, 2);
  Serial.println(" A");
  Serial.print("Potencia mínima: ");
  Serial.print(minPower, 2);
  Serial.println(" W");
  Serial.print("Potencia máxima: ");
  Serial.print(maxPower, 2);
  Serial.println(" W");
  Serial.print("Rango de potencia (max - min): ");
  Serial.print(rangePower, 2);
  Serial.println(" W");
  Serial.print("Consumo (rango de potencia): ");
  Serial.print(rangePower, 2);
  Serial.println(" W");

  Serial.print("Humedad: ");
  if (chk == DHTLIB_OK) Serial.println(humidity, 2);
  else Serial.println("Error en DHT11");

  Serial.print("Temperatura (C): ");
  if (chk == DHTLIB_OK) Serial.println(temperature, 2);
  else Serial.println("Error en DHT11");

  // Añadir un delay para controlar el intervalo entre mediciones
  delay(4000);  // 4000 ms + ~1000 ms de muestreo = 5 segundos totales
}
