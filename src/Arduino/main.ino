#include <FastLED.h>
#define LED_PIN     2        // Porta digital D2 para a fita LED WS2812
#define NUM_LEDS    5        // Número de LEDs na fita (incluindo o último LED que ficará apagado)
#define SENSOR_PIN  A2       // Porta analógica A2 para o sensor de batimentos cardíacos
CRGB leds[NUM_LEDS];
unsigned long startTime = 0;    // Tempo de início da medição
unsigned long measureDuration = 10000;  // Duração da medição em milissegundos (10 segundos)
bool isMeasuring = true;        // Flag para indicar se a medição está ativa
int bpmSum = 0;                 // Soma dos BPMs durante a medição
int bpmCount = 0;               // Contagem dos batimentos para a média
int bpmFinal = 0;               // BPM final calculado após 10 segundos
unsigned long lastBeatTime = 0;  // Armazena o tempo do último batimento
int beatInterval = 0;           // Intervalo entre batimentos em milissegundos
int threshold = 515;            // Threshold para detectar picos no sensor
void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);  // Configura a fita de LED
  FastLED.clear();                                        // Limpa os LEDs inicialmente
  pinMode(SENSOR_PIN, INPUT);
  Serial.begin(9600);  // Inicializa a comunicação serial
  startTime = millis();  // Marca o início do tempo de medição
}
void loop() {
  // Leitura do sensor de batimentos cardíacos
  int sensorValue = analogRead(SENSOR_PIN);
  Serial.println(sensorValue);  // Exibe os valores do sensor no Serial Plotter para ajuste do threshold
  // Durante a fase de medição (10 segundos)
  if (isMeasuring) {
    if (sensorValue > threshold && millis() - lastBeatTime > 300) {  // Detecção de batimento acima de 600
      unsigned long currentTime = millis();
      beatInterval = currentTime - lastBeatTime;
      lastBeatTime = currentTime;
      int bpm = 60000 / beatInterval;  // Calcula o BPM
      bpmSum += bpm;  // Soma os BPMs
      bpmCount++;     // Incrementa o contador de batimentos
      Serial.print("BPM Medido: ");
      Serial.println(bpm);
    }
    // Se 10 segundos se passaram, encerra a medição e calcula a média
    if (millis() - startTime >= measureDuration) {
      bpmFinal = bpmSum / bpmCount;  // Calcula a média dos BPMs medidos
      Serial.print("BPM Final (média): ");
      Serial.println(bpmFinal);
      isMeasuring = false;  // Desativa a medição
    }
  } else {
    // Após a medição de 10 segundos, começa a piscar a fita LED na frequência dos batimentos cardíacos
    smoothBlinkLeds(bpmFinal);
  }
}
void smoothBlinkLeds(int bpm) {
  int duration = 60000 / bpm / 2;  // Define a duração do piscar com base no BPM
  // Efeito de transição suave (fade in)
  for (int brightness = 0; brightness <= 255; brightness += 5) {
    for (int i = 0; i < NUM_LEDS - 1; i++) {  // O último LED (index NUM_LEDS - 1) será ignorado
      leds[i] = CRGB(brightness, 0, 0);  // Transição suave para cor vermelha
    }
    leds[NUM_LEDS - 1] = CRGB::Black;  // Mantém o último LED apagado
    FastLED.show();
    delay(duration / 51);  // Ajusta o tempo de cada passo da transição
  }
  // Efeito de transição suave (fade out)
  for (int brightness = 255; brightness >= 0; brightness -= 5) {
    for (int i = 0; i < NUM_LEDS - 1; i++) {  // O último LED (index NUM_LEDS - 1) será ignorado
      leds[i] = CRGB(brightness, 0, 0);  // Transição suave para apagar
    }
    leds[NUM_LEDS - 1] = CRGB::Black;  // Mantém o último LED apagado
    FastLED.show();
    delay(duration / 51);  // Ajusta o tempo de cada passo da transição
  }
}
