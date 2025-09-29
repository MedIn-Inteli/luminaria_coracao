#include <FastLED.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30100_PulseOximeter.h"
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "SHARE-RESIDENTE";
const char* password = "Share@residente23";

WebServer server(80);

#define GRAPH_POINTS 128
int ecgWebData[GRAPH_POINTS] = {90};
bool desenhandoWebPico = false;
int picoWebCol = 0;

#define LED_PIN     25
#define NUM_LEDS    4
#define BTN_PIN     32
#define BTN_PIN2    27
CRGB leds[NUM_LEDS];

PulseOximeter pox;

#define REPORTING_PERIOD_MS 1200
#define MAX_POINTS 50
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int bpmBuffer[MAX_POINTS] = {0};
int bpmIndex = 0;
bool sensorInitialized = false;
bool mediaDefinida = false;
int mediaFixaBPM = 0;

unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 800;
int estado = 0;

unsigned long lastBlinkUpdate = 0;
int brightness = 0;
bool fadingUp = true;

unsigned long startMillis;

// ECG
const byte figura[19][18] = {
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0},
  {1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1},
  {0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}
};

#define HISTORY_SIZE 128
#define MAX_PICOS 5

int escala = 2;
int yBase = 13;
int currentY = 31;
int historyY[HISTORY_SIZE];

bool desenhandoPico = false;
int picoCol = 0;
int filaPicos[MAX_PICOS];
int inicioFila = 0;
int fimFila = 0;

bool filaCheia() {
  return ((fimFila + 1) % MAX_PICOS) == inicioFila;
}

bool filaVazia() {
  return inicioFila == fimFila;
}

void enfileirarPico() {
  if (!filaCheia()) {
    filaPicos[fimFila] = 1;
    fimFila = (fimFila + 1) % MAX_PICOS;
  }
}

void desenfileirarPico() {
  if (!filaVazia()) {
    inicioFila = (inicioFila + 1) % MAX_PICOS;
  }
}

void desenhaECG() {
  if (!desenhandoPico && !filaVazia()) {
    desenhandoPico = true;
    picoCol = 0;
    desenfileirarPico();
  }

  if (desenhandoPico && picoCol < 18) {
    int row = -1;
    for (int r = 0; r < 19; r++) {
      if (figura[r][picoCol] == 1) {
        row = r;
        break;
      }
    }
    if (row != -1) {
      currentY = yBase + row * escala;
    }
    picoCol++;
  } else {
    desenhandoPico = false;
  }
}

void initializeSensor() {
  Serial.println("Inicializando sensor MAX30100...");
  if (!pox.begin()) {
    Serial.println("FALHA: Não foi possível inicializar o sensor MAX30100");
    sensorInitialized = false;
    return;
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_11MA);
  Serial.println("Sensor MAX30100 inicializado com sucesso!");
  sensorInitialized = true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear(); 
  FastLED.show();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay(); 
  display.display();

  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BTN_PIN2, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nConectado com IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", []() { server.send(200, "text/html", getHTMLPage()); });
  server.on("/bpm", []() { server.send(200, "text/plain", String(mediaFixaBPM)); });
  server.on("/bpmdata", []() {
    String json = "[";
    for (int i = 0; i < GRAPH_POINTS; i++) {
      json += String(ecgWebData[i]);
      if (i < GRAPH_POINTS - 1) json += ",";
    }
    json += "]";
    server.send(200, "application/json", json);
  });

  server.begin();

  initializeSensor();
  startMillis = millis();

  for (int i = 0; i < HISTORY_SIZE; i++) {
    historyY[i] = currentY;
  }
  
  // Inicializa o array do gráfico web com linha reta
  for (int i = 0; i < GRAPH_POINTS; i++) {
    ecgWebData[i] = 90;
  }
}

void loop() {
  server.handleClient();
  
  if (sensorInitialized) {
    pox.update();
    static unsigned long lastReport = 0;
    if (millis() - lastReport > REPORTING_PERIOD_MS) {
      float bpm = pox.getHeartRate();

      Serial.print("BPM: ");
      Serial.println(bpm);

      lastReport = millis();
      bpmBuffer[bpmIndex] = bpm;
      bpmIndex = (bpmIndex + 1) % MAX_POINTS;
    }
  }

  if (digitalRead(BTN_PIN) == LOW && millis() - lastButtonPress > debounceDelay) {
    lastButtonPress = millis();
    estado = (estado + 1) % 3;

    if (estado == 1) ledsEstaticos();
    else if (estado == 2) turnOffLeds();
  }

  if (digitalRead(BTN_PIN2) == LOW && millis() - lastButtonPress > debounceDelay) {
    lastButtonPress = millis();
    Serial.println("Reiniciando ESP32...");
    ESP.restart();
  }

  if (millis() - startMillis < 12000) {
    turnOffLeds();
  } else {
    if (!mediaDefinida) {
      mediaFixaBPM = calcularMediaBPM();
      mediaDefinida = true;
      Serial.print("BPM médio: ");
      Serial.println(mediaFixaBPM);
    }
    if (estado == 0) smoothBlinkLeds(mediaFixaBPM);
  }

  desenhaECG();
  
  for (int i = 0; i < HISTORY_SIZE - 1; i++) {
    historyY[i] = historyY[i + 1];
  }
  historyY[HISTORY_SIZE - 1] = currentY;
  
  display.clearDisplay();
  for (int x = 1; x < HISTORY_SIZE; x++) {
    display.drawLine(x - 1, historyY[x - 1], x, historyY[x], WHITE);
  }

  if (millis() - startMillis < 12000) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, SCREEN_HEIGHT / 2);
    display.print("Coloque o dedo no sensor!");
  }

  if (mediaDefinida) {
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, SCREEN_HEIGHT - 8);
    display.print("BPM medio: ");
    display.print(mediaFixaBPM);
  }

  display.display();

  // Atualiza o gráfico web
  if (desenhandoWebPico) {
    if (picoWebCol < 18) {
      int row = -1;
      for (int r = 0; r < 19; r++) {
        if (figura[r][picoWebCol] == 1) {
          row = r;
          break;
        }
      }
      int valor = (row != -1) ? 180 - (row * 7) : 90;
      
      // Desloca todos os dados para a esquerda
      for (int i = 0; i < GRAPH_POINTS - 1; i++) {
        ecgWebData[i] = ecgWebData[i + 1];
      }
      // Adiciona novo pixel no final
      ecgWebData[GRAPH_POINTS - 1] = valor;
      
      picoWebCol++;
    } else {
      // Mantem linha reta depois do pico
      for (int i = 0; i < 5; i++) {
        // Desloca todos os dados para a esquerda
        for (int j = 0; j < GRAPH_POINTS - 1; j++) {
          ecgWebData[j] = ecgWebData[j + 1];
        }
        // Adiciona ponto de linha reta no final
        ecgWebData[GRAPH_POINTS - 1] = 90;
      }
      desenhandoWebPico = false;
      picoWebCol = 0;
    }
  } else {
    // Quando não está desenhando pico, apenas desloca os dados e mantém linha reta
    for (int i = 0; i < GRAPH_POINTS - 1; i++) {
      ecgWebData[i] = ecgWebData[i + 1];
    }
    ecgWebData[GRAPH_POINTS - 1] = 90;
  }

  delay(15);

  static unsigned long lastSensorRetry = 0;
  if (!sensorInitialized && millis() - lastSensorRetry > 5000) {
    initializeSensor();
    lastSensorRetry = millis();
  }
}

int calcularMediaBPM() {
  int soma = 0, contador = 0;
  for (int i = 0; i < MAX_POINTS && contador < 10; i++) {
    if (bpmBuffer[i] >= 50 && bpmBuffer[i] <= 180) {
      soma += bpmBuffer[i];
      contador++;
    }
  }
  return (contador > 0) ? soma / contador : 70;
}

void turnOffLeds() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void ledsEstaticos() {
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
}

void smoothBlinkLeds(int bpm) {
  if (bpm == 0 || estado != 0) return;
  int ciclo = 60000 / bpm;
  int intervalo = ciclo / (51 * 2);

  if (millis() - lastBlinkUpdate >= intervalo) {
    lastBlinkUpdate = millis();

    if (fadingUp) {
      brightness += 15;
      if (brightness >= 255) {
        brightness = 255;
        fadingUp = false;
      }
    } else {
      brightness -= 15;
      if (brightness <= 0) {
        brightness = 0;
        fadingUp = true;
        enfileirarPico(); 
        // Inicia o pico do gráfico do site
        desenhandoWebPico = true; 
        picoWebCol = 0;
      }
    }

    fill_solid(leds, NUM_LEDS, CRGB(brightness, 0, 0));
    FastLED.show();
  }
}

String getHTMLPage() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Luminária Med-In</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>

    <!-- Fontes utilizadas -->
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Milonga&family=Montserrat:wght@100..900&display=swap" rel="stylesheet">

    <style>
        body {
            margin: 0;
            font-family: "Milonga", serif;
            background-color: #ffffff;
            color: #000000;
        }

        header {
            background-color: #00B1B1;
            padding: 15px 20px;
            display: flex;
            align-items: center;
            justify-content: space-between;
        }

        header h1 {
    font-size: 0.8rem;
    color: #000;
    margin: 0;
    font-family: "Milonga", serif;

}

main > h1 {
    font-size: 2.5rem;
    font-family: "Milonga", serif;
    margin-left: 30px;
}
        main > p {
            margin-left: 30px;
            color: #000000;
        }

        main > h1 {
            margin-top: 30px;
        }

        main > p {
            margin-top: 15px;
        }

        #bpmSection {
            font-family: 'Montserrat', sans-serif;
            background: #ffffff;
            color: #000000;
            padding-bottom: 50px;
        }

        #bpmSection canvas {
            background: #000;
            border: 1px solid #444;
            border-radius: 10px;
        }

        .grafico-logo {
            display: flex;
            justify-content: flex-start;
            align-items: flex-start;
            gap: 40px;
            margin: 30px;
            max-width: 900px;
        }

        .heart-container {
            display: flex;
            flex-direction: column;
            align-items: left;
            justify-content: left;
            font-family: 'Milonga', serif;
        }

        #bpmValue {
            font-size: 1em;
            color: rgb(0, 0, 0);
            font-weight: bold;
            font-family: 'Milonga', serif;
        }

        .footer {
            background-color: #00a9b3;
            color: white;
            display: flex;
            justify-content: space-around;
            padding: 80px 20px;
            font-family: 'Milonga', serif;
        }

        .footer-coluna {
            max-width: 400px;
        }

        .footer-coluna h3 {
            color: rgb(0, 0, 0);
            font-size: 24px;
            margin-bottom: 10px;
        }
        .footer-coluna h4 {
            color: rgb(255, 255, 255);
            font-size: 24px;
            margin-bottom: 10px;
        }

        .footer-coluna p {
            color: black;
            font-size: 20px;
            line-height: 1.4;
        }

        .footer-coluna ul {
            list-style: none;
            padding: 0;
            margin: 0;
        }

        .footer-coluna li {
            margin-bottom: 6px;
        }

        .footer-coluna a {
            color: white;
            text-decoration: none;
        }

        .footer-coluna a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <header>
        <h1>Luminária Med-In</h1>
    </header>
    <main>
        <h1>Luminária Med-In</h1>
        <p>Insira o dedo no sensor por 10 segundos e veja o valor do seu Batimento por Minuto (BPM) e uma simulação do seu eletrocardiograma!</p>
        <section id="bpmSection">
            <div class="grafico-logo">
                <canvas id="ecgChart" width="900" height="360"></canvas>
                <div class="heart-container">
                    <img src="data:image/png;base64,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" alt="Coração" width="200" height="310">
                    <p>BPM médio: <span id="bpmValue">--</span></p>
                </div>
            </div>
        </section>
        <footer class="footer">
            <div class="footer-coluna">
                <h3>Med-In Inteli</h3>
                <p>A primeira liga estudantil a trazer projetos com foco na interseção da medicina com a engenharia.</p>
            </div>
            <div class="footer-coluna">
                <h4>Contato dos Colaboradores</h4>
                <ul>
                    <li><a href="https://www.linkedin.com/in/nicolli-venino-santana-b84341254">Nicolli Venino Santana</a></li>
                    <li><a href="#">link_do_linkedin</a></li>
                    <li><a href="#">link_do_linkedin</a></li>
                    <li><a href="#">link_do_linkedin</a></li>
                    <li><a href="#">link_do_linkedin</a></li>
                </ul>
            </div>
            <div class="footer-coluna">
                <h4>Nossas redes sociais:</h4>
                <ul>
                    <li><a href="https://www.instagram.com/medin.inteli?utm_source=ig_web_button_share_sheet&igsh=ZDNlZDc0MzIxNw==">Instagram</a></li>
                    <li><a href="#">Linkedin</a></li>
                </ul>
                <h4>Github do projeto:</h4>
                <ul>
                    <li><a href="#">link_do_github</a></li>
                </ul>
            </div>
        </footer>
    </main>

    <script>
        function showSection(sectionId) {
            document.getElementById('bpmSection').style.display = sectionId === 'bpmSection' ? 'block' : 'none';
        }

        document.addEventListener("DOMContentLoaded", () => {
            showSection('bpmSection');

            const ctx = document.getElementById('ecgChart').getContext('2d');
            const ecgChart = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: Array.from({ length: 128 }, (_, i) => i),
                    datasets: [{
                        label: 'ECG BPM',
                        data: Array(128).fill(70),
                        borderColor: 'lime',
                        borderWidth: 2,
                        pointRadius: 0,
                        tension: 0.3
                    }]
                },
                options: {
                    animation: false,
                    responsive: false,
                    scales: {
                        y: {
                            min: 0,
                            max: 200,
                            ticks: { display: false }
                        },
                        x: { display: false }
                    },
                    plugins: { legend: { display: false } }
                }
            });

            function atualizarBPM() {
                fetch('/bpm')
                    .then(res => res.text())
                    .then(data => {
                        const bpmElement = document.getElementById('bpmValue');
                        if (bpmElement) bpmElement.textContent = data;
                    });
            }

            function atualizarGrafico() {
                fetch('/bpmdata')
                    .then(res => res.json())
                    .then(data => {
    ecgChart.data.datasets[0].data = data.map(v => v - 20);
    ecgChart.update();
});
            }

            setInterval(() => {
                atualizarBPM();
                atualizarGrafico();
            }, 100);
        });
    </script>
</body>
</html>
)rawliteral";
}
