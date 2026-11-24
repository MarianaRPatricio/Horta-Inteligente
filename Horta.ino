#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// ========================================
// HORTA INTELIGENTE AUTOMATIZADA
// ========================================

// ---------- Pinos ----------
const int PIN_SOLO   = A0;
const int PIN_LDR    = A1; 
const int PIN_SERVO  = 9;
const int PIN_BUZZER = 10; //Porta do buzzer foi alterada pela Lais

const int LED_VERMELHO = 4;
const int LED_AMARELO  = 3;
const int LED_VERDE    = 2;

// ---------- Limites de umidade (sensor INVERTIDO) ----------
// Valor ALTO = solo SECO | Valor BAIXO = solo ÚMIDO
const int SOLO_SECO_ACIMA   = 1000; // acima disso = SECO (vermelho + buzzer)
const int SOLO_UMIDO_ABAIXO = 400;  // abaixo disso = ÚMIDO (verde)
const int HISTERESE = 20;

// ---------- Luz (com histerese) ----------
const int LUZ_FECHAR = 820;
const int LUZ_ABRIR  = 750;

// ---------- Servo ----------
const int COBERTURA_ABERTA  = 180; //giro do servo motor foi alterada para melhorar o giro
const int COBERTURA_FECHADA = 90;

// ---------- Objetos ----------
LiquidCrystal_I2C *lcd = nullptr;
Servo servoCobertura;

// ---------- Estados ----------
enum EstadoSolo { ESTADO_SECO, ESTADO_IDEAL, ESTADO_UMIDO };
EstadoSolo estadoAtual = ESTADO_IDEAL;
bool primeiraLeitura = true;

bool coberturaFechada = false;
unsigned long ultimoBip = 0;
unsigned long ultimoLCD = 0;


// ========================================
// PROCURA O LCD NO BARRAMENTO I2C
// ========================================
byte procuraLCD() {
  for (byte addr = 0x20; addr <= 0x3F; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) return addr;
  }
  return 0;
}


// ========================================
// CONFIGURAÇÃO INICIAL
// ========================================
void setup() {
  Serial.begin(9600);
  Wire.begin();

  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  // Servo começa aberto (90°)
  servoCobertura.attach(PIN_SERVO, 500, 2400);
  servoCobertura.write(COBERTURA_ABERTA);
  delay(500);

  // LCD
  byte endereco = procuraLCD();
  if (endereco == 0) {
    Serial.println("LCD NAO ENCONTRADO! Veja SDA/SCL, VCC e GND.");
  } else {
    Serial.print("LCD encontrado no endereco 0x");
    Serial.println(endereco, HEX);

    lcd = new LiquidCrystal_I2C(endereco, 16, 2);
    lcd->init();
    lcd->backlight();
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("Horta Inteligent");   // 16 caracteres
    lcd->setCursor(0, 1);
    lcd->print("Testando...");
  }

  // Teste rápido: LEDs acesos e um bip
  digitalWrite(LED_VERMELHO, HIGH);
  digitalWrite(LED_AMARELO, HIGH);
  digitalWrite(LED_VERDE, HIGH);
  tone(PIN_BUZZER, 1000, 300);
  delay(1000);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(LED_AMARELO, LOW);
  digitalWrite(LED_VERDE, LOW);

  if (lcd != nullptr) lcd->clear();
}


// ========================================
// PROGRAMA PRINCIPAL
// ========================================
void loop() {
  int soloBruto = analogRead(PIN_SOLO);
  int luzBruta  = analogRead(PIN_LDR);

  // Umidade em %: 1023 = 0% (seco) | 0 = 100% (úmido)
  int umidadePct = constrain(map(soloBruto, 0, 1023, 100, 0), 0, 100);
  int luzPct     = constrain(map(luzBruta, 0, 1023, 0, 100), 0, 100);

  // Estado do solo
  EstadoSolo novoEstado = calculaEstado(soloBruto);
  if (novoEstado != estadoAtual || primeiraLeitura) {
    estadoAtual = novoEstado;
    primeiraLeitura = false;
    atualizaLeds();
  }

  // Buzzer só no solo seco
  if (estadoAtual == ESTADO_SECO) {
    bipaSoloSeco();
  } else {
    noTone(PIN_BUZZER);
  }

  atualizaCobertura(luzBruta);

  // LCD a cada 500 ms
  if (millis() - ultimoLCD >= 500) {
    ultimoLCD = millis();
    atualizaDisplay(umidadePct, luzPct);
  }

  // Monitor Serial
  Serial.print("Solo: ");
  Serial.print(soloBruto);
  Serial.print(" (umidade ");
  Serial.print(umidadePct);
  Serial.print("%) -> ");
  if (estadoAtual == ESTADO_SECO)       Serial.print("SECO  [Vermelho + Buzzer]");
  else if (estadoAtual == ESTADO_IDEAL) Serial.print("IDEAL [Amarelo]");
  else                                  Serial.print("UMIDO [Verde]");
  Serial.print(" | Luz: ");
  Serial.print(luzBruta);
  Serial.print(" | Cobertura: ");
  Serial.println(coberturaFechada ? "Fechada" : "Aberta");

  delay(100);
}


// ----- Define o estado do solo (com histerese) -----
EstadoSolo calculaEstado(int solo) {
  int limSeco  = SOLO_SECO_ACIMA;
  int limUmido = SOLO_UMIDO_ABAIXO;

  if (estadoAtual == ESTADO_SECO)  limSeco  -= HISTERESE;
  if (estadoAtual == ESTADO_UMIDO) limUmido += HISTERESE;

  if (solo > limSeco)  return ESTADO_SECO;
  if (solo < limUmido) return ESTADO_UMIDO;
  return ESTADO_IDEAL;
}


// ----- LEDs conforme o estado -----
void atualizaLeds() {
  digitalWrite(LED_VERMELHO, estadoAtual == ESTADO_SECO  ? HIGH : LOW);
  digitalWrite(LED_AMARELO,  estadoAtual == ESTADO_IDEAL ? HIGH : LOW);
  digitalWrite(LED_VERDE,    estadoAtual == ESTADO_UMIDO ? HIGH : LOW);
}


// ----- Buzzer: bipe curto a cada 600 ms com solo seco -----
void bipaSoloSeco() {
  unsigned long agora = millis();
  if (agora - ultimoBip >= 600) {
    ultimoBip = agora;
    tone(PIN_BUZZER, 1000, 250);
  }
}


// ----- Cobertura pelo LDR -----
void atualizaCobertura(int luz) {
  if (luz > LUZ_FECHAR && !coberturaFechada) {
    servoCobertura.write(COBERTURA_FECHADA);
    coberturaFechada = true;
    Serial.println("Sol forte! Fechando cobertura.");
  }
  else if (luz < LUZ_ABRIR && coberturaFechada) {
    servoCobertura.write(COBERTURA_ABERTA);
    coberturaFechada = false;
    Serial.println("Luz normalizada! Abrindo cobertura.");
  }
}


// ----- LCD 16x2 -----
void atualizaDisplay(int umidadePct, int luzPct) {
  if (lcd == nullptr) return;

  const char *status;
  if (estadoAtual == ESTADO_SECO)       status = "SECO";
  else if (estadoAtual == ESTADO_IDEAL) status = "IDEAL";
  else                                  status = "UMIDO";

  char linha1[17];
  char linha2[17];

  snprintf(linha1, sizeof(linha1), "Solo:%3d%% %-6s", umidadePct, status);
  snprintf(linha2, sizeof(linha2), "Luz:%3d%% Cob:%s", luzPct, coberturaFechada ? "FEC" : "ABR");

  lcd->setCursor(0, 0);
  lcd->print(linha1);
  lcd->setCursor(0, 1);
  lcd->print(linha2);
}
