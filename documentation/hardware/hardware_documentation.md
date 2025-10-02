# Hardware Documentation - Luminária do Med-In
O documento tem o fito de descrever as especificações técnicas no que tange ao hardware do projeto da Luminária do Med-In. 

## 1. Arquitetura da Solução

### 1.1 Componentes e especificações Técnicas
| Categoria | Componente | Descrição | Função | Especificações | 
|-----------|------------|-----------|--------|---------------------- |
| Microcontrolador | ESP32 WROOM 32U | Microcontrolador com CPU dual-core, conectividade Wi-Fi e Bluetooth, e 16 MB de memória flash | Gerenciar todos os dispositivos e realizar processamento centralizado do sistema, de modo a receber os dados do oxímetro de pulso e enviar sinais para a fita de led e para o display, por intermédio da comunicação 12C | 5V | 
| Sensor | MAX30100 | Oxímetro de Pulso | Identificar o valor do BPM do usuário | 3.3V, comunicação 12C | 
| Feedback Visual | WS2812b | Fita de LED com 4 LEDs | Indicar visualmente a ocorrência de um batimento cardíaco. | 84 LEDs, alimentação 5V | 
| Feedback Visual | oled SDD1306 | Display | Simular um eletrocardiograma | 3.3V, comunicação 12C | 
| Controle Manual | Botão de Reset | Botão tátil momentâneo, conectado ao microcontrolador | Reinicializar o sistema para o estado inicial. Usado para trocar o usuário sem precisar tirar a luminária da fonte de alimentação |  | 
| Controle Manual | Botão de estado | Botão tátil momentâneo, conectado ao microcontrolador | Alterar o estado da fita de LED |  | 
| Resistor de pull-up | Resistor de 4,7kΩ | Resistor de carga, conectado entre o SDA ou o SCL e o VCC.  | Usado para garantir que o SDA e o SCL estejam sempre no nível lógico "alto" quando não estiver sendo ativado pelo microcontrolador. | 4,7kΩ| 

### 1.1.1 Características do microcontrolador ESP32 WROOM 32U

### 1.1.2 Características do Oxímetro de Pulso MAX30100

#### 1.1.2.1 Por que usar o MAX30100 ao invés do sensor de batimentos cardíacos comum?

### 1.1.3 Características do Display oled SDD1306

### 1.1.4 Carcterísticas da fita de LED WS2812b 

### 1.2 Conexões de Hardware

| Componente  | Pino no Microcontrolador | Conexão Física        |
|-------------|--------------------------|-----------------------|
| Fita de LED      | 25                  | Via conector 3 pinos  |
| Push Bottom 1    | 27                  | Via conector 3 pinos  |
| Push Bottom 2    | 32                  | Via conector 3 pinos  |
| SCL geral        | 22                  | Via cabo jumper       |
| SDA geral        | 21                  | Via cabo jumper       |
| GND Geral        | GND                 | Barramento comum      |
| VCC Geral        | 5V                  | Fonte regulada 5V     |

## 2. Diagrama de Circuito

### 2.1 Protótipo Físico do Projeto - Protoboard

### 2.2 Esquemático da placa - KiCAD

<p align = "center">Figura 1 - Esquemático </p>
<div align = "center">
  <img src = "/assets/esquematico.png">
</div>
<p align = "center"> Fonte: material produzido pelos autores.</p>

### 2.3 PCI - Placa de Circuito Impresso - KiCAD

<p align = "center">Figura 2 - Placa de Circuito Impresso </p>
<div align = "center">
  <img src = "/assets/pci.png">
</div>
<p align = "center"> Fonte: material produzido pelos autores.</p>

## Instruções de Montagem

1. Conecte os sensores EMG aos pinos analógicos A0 e A1 do microcontrolador.
2. Conecte a fita de LED ao pino digital D5, e alimente-a com 5V estável.
3. Garanta que todos os componentes compartilhem o mesmo GND.
4. Utilize um multímetro para verificar continuidade antes de alimentar o circuito.


## Requisitos de Alimentação

- Fonte de alimentação: 5V/2A.
- Consumo típico: 1.2A (dependente da intensidade dos LEDs).

---

## Logs e Resultados

Documente aqui os resultados obtidos durante testes de hardware, como medições de tensão, correntes e sinais capturados pelos sensores.
