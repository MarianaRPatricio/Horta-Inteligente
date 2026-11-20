
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
const int PIN_BUZZER = 10;

const int LED_VERMELHO = 4;
const int LED_AMARELO  = 3;
const int LED_VERDE    = 2;

// ---------- Limites de umidade ----------
const int SOLO_MAX   = 876;
const int SOLO_SECO  = 300;
const int SOLO_UMIDO = 600;

// ---------- Limite de luminosidade ----------
const int LUZ_EXCESSIVA = 820;

// ---------- Servo ----------
const int COBERTURA_ABERTA  = 80;
const int COBERTURA_FECHADA = 0;

// ---------- Objetos ----------
LiquidCrystal_I2C *lcd = nullptr;
Servo servoCobertura;

// ---------- Variáveis ----------
bool coberturaFechada = false;

unsigned long ultimoBip = 0;
bool buzzerLigado = false;


// ========================================
// LOCALIZAÇÃO DO LCD
// ========================================

byte encontraLCD() {

  for (byte addr = 1; addr < 127; addr++) {

    Wire.beginTransmission(addr);

    if (Wire.endTransmission() == 0) {
      return addr;
    }
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

  // Inicialização do servo
  servoCobertura.attach(PIN_SERVO, 500, 2400);

  servoCobertura.write(COBERTURA_ABERTA);

  coberturaFechada = false;

  // Inicialização do LCD
  byte endereco = encontraLCD();

  if (endereco == 0) {

    Serial.println("LCD nao encontrado!");

  } else {

    lcd = new LiquidCrystal_I2C(endereco, 16, 2);

    lcd->init();
    lcd->backlight();
    lcd->clear();

    lcd->setCursor(0, 0);
    lcd->print("Horta Inteligente");

    lcd->setCursor(0, 1);
    lcd->print("Iniciando...");

    delay(1500);

    lcd->clear();
  }
}


// ========================================
// PROGRAMA PRINCIPAL
// ========================================

void loop() {

  // Leitura dos sensores
  int soloBruto = analogRead(PIN_SOLO);
  int luzBruta = analogRead(PIN_LDR);

  // Conversão para porcentagem
  int soloPct = constrain(
    map(soloBruto, 0, SOLO_MAX, 0, 100),
    0, 100
  );

  int luzPct = map(luzBruta, 0, 1023, 0, 100);

  // Atualização dos componentes
  atualizaSolo(soloBruto);

  atualizaCobertura(luzBruta);

  atualizaDisplay(soloBruto, soloPct, luzPct);

  // Monitor Serial
  Serial.print("Solo: ");
  Serial.print(soloBruto);

  Serial.print(" (");
  Serial.print(soloPct);
  Serial.print("%)");

  Serial.print(" | Luz: ");
  Serial.print(luzBruta);

  Serial.print(" (");
  Serial.print(luzPct);
  Serial.print("%)");

  Serial.print(" | Cobertura: ");

  Serial.println(
    coberturaFechada ? "Fechada" : "Aberta"
  );

  delay(200);
}


// ========================================
// CONTROLE DA UMIDADE DO SOLO
// ========================================

void atualizaSolo(int solo) {

  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(LED_AMARELO, LOW);
  digitalWrite(LED_VERDE, LOW);

  if (solo < SOLO_SECO) {

    // Solo seco
    digitalWrite(LED_VERMELHO, HIGH);

    bipaSoloSeco();

  }

  else if (solo <= SOLO_UMIDO) {

    // Solo em condição intermediária
    digitalWrite(LED_AMARELO, HIGH);

    noTone(PIN_BUZZER);
    buzzerLigado = false;

  }

  else {

    // Solo úmido
    digitalWrite(LED_VERDE, HIGH);

    noTone(PIN_BUZZER);
    buzzerLigado = false;

  }
}


// ========================================
// BUZZER - AVISO DE SOLO SECO
// ========================================

void bipaSoloSeco() {

  unsigned long agora = millis();

  if (agora - ultimoBip >= 500) {

    ultimoBip = agora;

    buzzerLigado = !buzzerLigado;

    if (buzzerLigado) {

      tone(PIN_BUZZER, 1000);

    } else {

      noTone(PIN_BUZZER);

    }
  }
}


// ========================================
// CONTROLE DA COBERTURA PELO LDR
// ========================================

void atualizaCobertura(int luz) {

  // Se houver muita luminosidade,
  // fecha a cobertura apenas uma vez.

  if (luz > LUZ_EXCESSIVA && !coberturaFechada) {

    servoCobertura.write(COBERTURA_FECHADA);

    coberturaFechada = true;

    Serial.println("Sol forte! Fechando cobertura.");
  }

  // Se a luminosidade diminuir,
  // mantém a cobertura na posição atual.
}


// ========================================
// DISPLAY LCD 16x2
// ========================================

void atualizaDisplay(int soloBruto, int soloPct, int luzPct) {

  if (lcd == nullptr) return;

  const char *status;

  if (soloBruto < SOLO_SECO) {

    status = "SECO";

  }

  else if (soloBruto <= SOLO_UMIDO) {

    status = "IDEAL";

  }

  else {

    status = "UMIDO";

  }

  char linha1[17];
  char linha2[17];

  snprintf(
    linha1,
    sizeof(linha1),
    "Solo:%3d%% %-6s",
    soloPct,
    status
  );

  snprintf(
    linha2,
    sizeof(linha2),
    "Luz:%3d%% Cob:%s",
    luzPct,
    coberturaFechada ? "FEC" : "ABR"
  );

  lcd->setCursor(0, 0);
  lcd->print(linha1);

  lcd->setCursor(0, 1);
  lcd->print(linha2);
}
