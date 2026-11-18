
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// ========================================
// HORTA INTELIGENTE AUTOMATIZADA
// ========================================


// ===== SENSORES =====

const int pinoUmidade = A0;
const int pinoLDR = A1;


// ===== PINOS DOS LEDS =====

const int ledVermelho = 4;
const int ledAmarelo = 3;
const int ledVerde = 2;


// ===== PINOS DOS ATUADORES =====

const int pinoRele = 7;
const int pinoServo = 9;


// ===== DISPLAY LCD =====

// LCD 16 colunas e 2 linhas

LiquidCrystal_I2C lcd_1(0x27, 16, 2);


// ===== SERVOMOTOR =====

Servo cobertura;


// ===== LIMITES DOS SENSORES =====

const int limiteSoloSeco = 600;
const int limiteSoloUmido = 300;

const int limiteSolForte = 700;
const int limiteSolNormal = 600;


// ===== POSICOES DA COBERTURA =====

const int coberturaAberta = 0;
const int coberturaFechada = 60;


// ===== VARIAVEIS =====

int leituraUmidade = 0;
int leituraLuz = 0;

bool estadoCobertura = false;
bool bombaLigada = false;


// ========================================
// CONFIGURACAO INICIAL
// ========================================

void setup() {

  Serial.begin(9600);

  // LEDs

  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVerde, OUTPUT);

  // Rele

  pinMode(pinoRele, OUTPUT);

  digitalWrite(pinoRele, LOW);

  // Servomotor

  cobertura.attach(pinoServo);

  cobertura.write(coberturaAberta);

  // DISPLAY LCD

  lcd_1.init();

  lcd_1.backlight();

  lcd_1.setCursor(0, 0);
  lcd_1.print("HORTA INTELIGENTE");

  lcd_1.setCursor(0, 1);
  lcd_1.print("Iniciando...");

  delay(2000);

  lcd_1.clear();

  Serial.println("HORTA INTELIGENTE INICIADA!");

}


// ========================================
// CONTROLE DA UMIDADE
// ========================================

void controlarUmidade() {

  if (leituraUmidade > limiteSoloSeco) {

    // SOLO SECO

    digitalWrite(ledVermelho, HIGH);
    digitalWrite(ledAmarelo, LOW);
    digitalWrite(ledVerde, LOW);

    // Liga a bomba

    digitalWrite(pinoRele, HIGH);

    bombaLigada = true;

    Serial.println("SOLO SECO - BOMBA LIGADA");

  }

  else if (leituraUmidade >= limiteSoloUmido) {

    // SOLO IDEAL

    digitalWrite(ledVermelho, LOW);
    digitalWrite(ledAmarelo, HIGH);
    digitalWrite(ledVerde, LOW);

    // Desliga a bomba

    digitalWrite(pinoRele, LOW);

    bombaLigada = false;

    Serial.println("SOLO IDEAL - BOMBA DESLIGADA");

  }

  else {

    // SOLO MUITO UMIDO

    digitalWrite(ledVermelho, LOW);
    digitalWrite(ledAmarelo, LOW);
    digitalWrite(ledVerde, HIGH);

    // Desliga a bomba

    digitalWrite(pinoRele, LOW);

    bombaLigada = false;

    Serial.println("SOLO MUITO UMIDO - BOMBA DESLIGADA");

  }

}


// ========================================
// CONTROLE DA COBERTURA
// ========================================

void controlarCobertura() {

  // SOL FORTE: FECHA A COBERTURA LENTAMENTE

  if (leituraLuz > limiteSolForte &&
      estadoCobertura == false) {

    Serial.println("SOL FORTE - FECHANDO COBERTURA");

    // Move o servo de 0 até 60 graus

    for (int angulo = coberturaAberta;
         angulo <= coberturaFechada;
         angulo++) {

      cobertura.write(angulo);

      delay(30); // Controla a velocidade

    }

    estadoCobertura = true;

    Serial.println("COBERTURA FECHADA");

  }

  // POUCA LUZ: ABRE A COBERTURA LENTAMENTE

  else if (leituraLuz < limiteSolNormal &&
           estadoCobertura == true) {

    Serial.println("POUCA LUZ - ABRINDO COBERTURA");

    // Move o servo de 60 até 0 graus

    for (int angulo = coberturaFechada;
         angulo >= coberturaAberta;
         angulo--) {

      cobertura.write(angulo);

      delay(30);

    }

    estadoCobertura = false;

    Serial.println("COBERTURA ABERTA");

  }

}


// ========================================
// ATUALIZACAO DO DISPLAY LCD
// ========================================

void atualizarDisplay() {

  // PRIMEIRA LINHA: SENSORES

  lcd_1.setCursor(0, 0);

  lcd_1.print("U:");
  lcd_1.print(leituraUmidade);

  lcd_1.print(" L:");
  lcd_1.print(leituraLuz);

  lcd_1.print("   ");


  // SEGUNDA LINHA: STATUS DO SISTEMA

  lcd_1.setCursor(0, 1);

  if (leituraUmidade > limiteSoloSeco) {

    lcd_1.print("Solo:SECO B:ON ");

  }

  else if (leituraUmidade >= limiteSoloUmido) {

    lcd_1.print("Solo:IDEAL B:OFF");

  }

  else {

    lcd_1.print("Solo:UMIDO B:OFF");

  }

}


// ========================================
// MONITOR SERIAL
// ========================================

void monitorarSistema() {

  Serial.println("-------------------------");

  Serial.print("Umidade: ");
  Serial.println(leituraUmidade);

  Serial.print("Luminosidade: ");
  Serial.println(leituraLuz);

  Serial.print("Bomba: ");

  if (bombaLigada) {
    Serial.println("LIGADA");
  }

  else {
    Serial.println("DESLIGADA");
  }

  Serial.print("Cobertura: ");

  if (estadoCobertura) {
    Serial.println("FECHADA");
  }

  else {
    Serial.println("ABERTA");
  }

}


// ========================================
// LOOP PRINCIPAL
// ========================================

void loop() {

  // LEITURA DOS SENSORES

  leituraUmidade = analogRead(pinoUmidade);

  leituraLuz = analogRead(pinoLDR);


  // CONTROLE DA IRRIGACAO

  controlarUmidade();


  // CONTROLE DA COBERTURA

  controlarCobertura();


  // ATUALIZACAO DO DISPLAY

  atualizarDisplay();


  // MONITORAMENTO SERIAL

  monitorarSistema();


  delay(500);

}
-