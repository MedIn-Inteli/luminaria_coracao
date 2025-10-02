# Template de Documentação para Hardware

## 🔠 Visão Geral

Esta seção descreve os componentes de hardware utilizados no projeto e como eles interagem entre si. Inclui detalhes sobre conexões, esquemas elétricos e requisitos de alimentação.

---

## 📋 Especificações Técnicas

| Componente        | Modelo/Referência  | Quantidade | Especificações          | Observações                 |
|--------------------|-------------------|------------|-------------------------|-----------------------------|
| Microcontrolador  | ESP32-WROOM-32U    | 1          | 5V                      | Principal unidade de controle. |
| Oxímetro de Pulso | MAX30100           | 1          | 0-3.3V analógico        | Utilizado para leitura de batimentos cardíacos. |
| Fita de led       | WS2812b            | 4          | 84 LEDs, alimentação 5V | Indica visualmente a ocorrência do batimento cardíaco |
| Display           | oled, SSD1306      | 1          | 3.3V                    | Usado para simulação de eletrocardiograma. |
| Resistores        | 4,7kΩ              | 1          | Resistência de carga.   | Conectado entre o 3.3V e o pino INT do MAX30100 |
| Push Bottom       |              | 2          |    | Usado para alterar o estado da fita de led. |

---

## 🛠️ Diagrama de Circuito

O diagrama elétrico configura-se como o esquemático da placa, feito no software de desenvolvimento KiCAD, conforme a figura abaixo:

<p align = "center">Figura 1 - Esquemático </p>
<div align = "center">
  <img src = "/assets/esquematico.png">
</div>
<p align = "center"> Fonte: material produzido pelos autores.</p>

---

## PCI - Placa de Circuito Impresso

<p align = "center">Figura 2 - Placa de Circuito Impresso </p>
<div align = "center">
  <img src = "/assets/pci.png">
</div>
<p align = "center"> Fonte: material produzido pelos autores.</p>

## 🔐 Conexões de Hardware

| Componente  | Pino no Microcontrolador | Conexão Física        |
|-------------|--------------------------|-----------------------|
| Fita de LED      | 25                  | Via conector 3 pinos  |
| Push Bottom 1    | 27                  | Via conector 3 pinos  |
| Push Bottom 2    | 32                  | Via conector 3 pinos  |
| SCL geral        | 22                  | Via cabo jumper       |
| SDA geral        | 21                  | Via cabo jumper       |
| GND Geral        | GND                 | Barramento comum      |
| VCC Geral        | 5V                  | Fonte regulada 5V     |

---

## ⚙️ Instruções de Montagem

1. Conecte os sensores EMG aos pinos analógicos A0 e A1 do microcontrolador.
2. Conecte a fita de LED ao pino digital D5, e alimente-a com 5V estável.
3. Garanta que todos os componentes compartilhem o mesmo GND.
4. Utilize um multímetro para verificar continuidade antes de alimentar o circuito.

---

## 🔋 Requisitos de Alimentação

- Fonte de alimentação: 5V/2A.
- Consumo típico: 1.2A (dependente da intensidade dos LEDs).

---

## 🔂 Logs e Resultados

Documente aqui os resultados obtidos durante testes de hardware, como medições de tensão, correntes e sinais capturados pelos sensores.
