#define TRIG_PIN 18
#define ECHO_PIN 19

void setup() {

  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {

  // Limpiar el pin TRIG
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Generar el pulso ultrasónico de 10 microsegundos
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  // Apagar el pulso
  digitalWrite(TRIG_PIN, LOW);

  // Leer el tiempo que tarda el eco en regresar (en microsegundos)
  long duracion = pulseIn(ECHO_PIN, HIGH);

  // Si pulseIn devuelve 0, significa que se agotó el tiempo de espera
  if (duracion == 0) {

    Serial.println("No se detectó eco");

  } else {

    // Calcular la distancia usando la fórmula de la guía: d = (0.0343 * t) / 2
    float distancia = (0.0343 * duracion) / 2;

    // Imprimir los resultados en el Monitor Serial
    Serial.print("Distancia: ");
    Serial.print(distancia);
    Serial.println(" cm");
  }

  delay(500);
}