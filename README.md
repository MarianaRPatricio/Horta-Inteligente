# 🌱 Horta Inteligente Automatizada com Arduino

Sistema embarcado desenvolvido em **Arduino (C++)** para monitoramento e automação de uma horta, realizando a leitura em tempo real da **umidade do solo** e da **luminosidade ambiente**, com alertas visuais, sonoros, exibição em display LCD I2C e controle automático de cobertura retrátil via servomotor.

---

## 📋 Funcionalidades

- **Monitoramento de Umidade do Solo:** Lê continuamente o nível de umidade (0% a 100%) e classifica o estado em **Seco**, **Ideal** ou **Úmido**.
- **Sinalização Visual por LEDs:**
  - 🔴 **LED Vermelho:** Solo seco (abaixo de 30%) — necessita irrigação.
  - 🟡 **LED Amarelo:** Umidade ideal (entre 30% e 70%).
  - 🟢 **LED Verde:** Solo úmido (acima de 70%).
- **Alerta Sonoro:** Aciona um buzzer intermitente sempre que o solo atingir o estado **Seco**.
- **Controle Automático de Cobertura (Sombrite):** Utiliza um sensor LDR e um servomotor para fechar a cobertura quando a incidência solar for excessiva e reabri-la quando a luminosidade normalizar (com controle por histerese para evitar oscilações).
- **Interface em Tempo Real:** Exibe os percentuais de umidade e luz, o status do solo e o estado da cobertura (`ABR` / `FEC`) em um display **LCD 16x2 I2C**, além de enviar logs detalhados via **Monitor Serial**.

---

## 🛠️ Componentes Utilizados

| Componente | Quantidade | Observação |
| :--- | :---: | :--- |
| Placa Arduino Uno (ou compatível) | 1 | Microcontrolador principal |
| Sensor de Umidade do Solo | 1 | Saída analógica |
| Sensor de Luminosidade (LDR) | 1 | Divisor de tensão com resistor de 10 kΩ |
| Display LCD 16x2 com Módulo I2C | 1 | Comunicação via barramento I2C (`SDA`/`SCL`) |
| Micro Servomotor (SG90 ou similar) | 1 | Acionamento da cobertura retrátil |
| Buzzer Ativo ou Passivo | 1 | Alerta sonoro de solo seco |
| LEDs (Vermelho, Amarelo e Verde) | 3 | Indicadores de status do solo |
| Resistores de 220 Ω (ou 330 Ω) | 3 | Proteção dos LEDs |
| Protoboard e Jumpers | - | Conexões do circuito |

---

## 🔌 Pinagem (Esquema de Ligação)

| Componente | Pino no Arduino | Tipo de Sinal |
| :--- | :---: | :--- |
| **Sensor de Umidade do Solo** | `A0` | Entrada Analógica |
| **Sensor LDR (Luminosidade)** | `A1` | Entrada Analógica |
| **LED Verde (Solo Úmido)** | `D2` | Saída Digital |
| **LED Amarelo (Solo Ideal)** | `D3` | Saída Digital |
| **LED Vermelho (Solo Seco)** | `D4` | Saída Digital |
| **Buzzer (Alerta Sonoro)** | `D8` | Saída Digital (`tone`) |
| **Servomotor (Cobertura)** | `D9` | Saída PWM |
| **Display LCD I2C (SDA)** | `A4` | Comunicação I2C |
| **Display LCD I2C (SCL)** | `A5` | Comunicação I2C |

> **Nota sobre o Buzzer e o Servo:** O buzzer foi alocado no pino `D8` para evitar conflito com o **Timer1** do Arduino Uno, utilizado internamente pela biblioteca `Servo.h` nos pinos `D9` e `D10`.

---

## 📚 Bibliotecas Necessárias

Certifique-se de ter as seguintes bibliotecas instaladas na sua **Arduino IDE** (ou ambiente de simulação como o Tinkercad):

1. **`Adafruit_LiquidCrystal`** — Controle do display LCD 16x2 via I2C.
2. **`Servo`** — Controle do servomotor (nativa da Arduino IDE).

---

## ⚙️ Parâmetros de Calibração

Caso utilize sensores físicos diferentes (resistivo ou capacitivo), ajuste as constantes no topo do código-fonte conforme as leituras observadas no Monitor Serial (`9600 bps`):

```cpp
// Faixa de leitura analógica do sensor de solo
const int LEITURA_MIN_SECO  = 0;
const int LEITURA_MAX_UMIDO = 876;

// Limites percentuais para mudança de estado
const int UMIDADE_SECA_ABAIXO = 30; // < 30% = SECO
const int UMIDADE_UMIDA_ACIMA = 70; // > 70% = ÚMIDO

// Limites de luminosidade com histerese
const int LUZ_FECHAR = 820; // Fecha a cobertura acima deste valor
const int LUZ_ABRIR  = 750; // Reabre a cobertura abaixo deste valor
