#include <SoftwareSerial.h> // Só usar se precisar de porta Serial extra

// Defenir Pins
const int LDR_PIN          = A4; // LDR
const int FLAME_SENSOR_PIN = 2;  // Sensor de chama
const int VIB_SENSOR_PIN   = 3;  // Sensor de vibração
const int BUZZER_PIN       = 9;  // Buzzer

// LEDs
const int LED_PINS[5]      = {4, 5, 6, 7, 8};

// Pinos do HC-05 via SoftwareSerial
const int BT_RX = 10; // RX do Arduino (liga no TX do HC-05)
const int BT_TX = 11; // TX do Arduino (liga no RX do HC-05)

// Se for usar a Serial padrão (pinos 0 e 1), comente as linhas abaixo e use Serial diretamente.
SoftwareSerial BTSerial(BT_RX, BT_TX);

// Estados
bool luzesLigadas         = false;
bool sismoDetectado       = false;
bool fogoDetectado        = false;

// Limites
const int LUZ_THRESHOLD   = 500; // LDR
const int VIB_THRESHOLD   = 100;
const int DEBOUNCE_DELAY  = 100; // Evitar leituras falsas

// Função para enviar mensagens Bluetooth
void enviarMensagemBT(const String &mensagem) {
  BTSerial.print(mensagem);
  BTSerial.print("\n");
}

// Funções buzzer
void buzzerSomSismo() {
  // Tom: Grave 
  tone(BUZZER_PIN, 400, 500); 
  delay(600);
  noTone(BUZZER_PIN);
}

void buzzerSomFogo() {
  // Tom: Agudo
  tone(BUZZER_PIN, 800, 500);
  delay(600);
  noTone(BUZZER_PIN);
}

// Ligar de Forma Gradual
void ligarLuzes() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PINS[i], HIGH);
    delay(600); // Delay entre LEDs
  }
  luzesLigadas = true;
  enviarMensagemBT("Luzes ON");
}

// Ligar de Forma Gradual
void desligarLuzes() {
  for (int i = 4; i >= 0; i--) {
    digitalWrite(LED_PINS[i], LOW);
    delay(600); // Delay entre LEDs
  }
  luzesLigadas = false;
  enviarMensagemBT("Luzes OFF");
}

void setup() {
  pinMode(LDR_PIN, INPUT);
  pinMode(FLAME_SENSOR_PIN, INPUT);
  pinMode(VIB_SENSOR_PIN, INPUT);
  
  pinMode(BUZZER_PIN, OUTPUT);
  
  //Iniciar os varios LEDs
  for (int i = 0; i < 5; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }

  // Iniciar Serial
  Serial.begin(9600);
  
  // Iniciar Bluetooth
  BTSerial.begin(9600);
  // Mensagem de Display Inicial APP
  enviarMensagemBT("Sistema Iniciado");

  // Mensagem de Display Inicial Serial
  Serial.println("Sistema de Gestao Ambiental Iniciado!");
}

void loop() {
  //Leitura LDR
  int ldrValue = analogRead(LDR_PIN);
  
  if (ldrValue < LUZ_THRESHOLD && !luzesLigadas) {
    ligarLuzes();
  } 
  else if (ldrValue >= LUZ_THRESHOLD && luzesLigadas) {
    desligarLuzes();
  }
  
  //Leitura de Chama
  int flameValue = digitalRead(FLAME_SENSOR_PIN);
  if (flameValue == LOW) {
    if (!fogoDetectado) {
      fogoDetectado = true;
      Serial.println("Fogo detectado!");
      enviarMensagemBT("Fogo Detectado!");
      buzzerSomFogo();
    }
  } else {
    fogoDetectado = false;
  }
  
  //Leitura de Vibração
  int vibValue = digitalRead(VIB_SENSOR_PIN);
  if (vibValue == HIGH) {
    if (!sismoDetectado) {
      sismoDetectado = true;
      Serial.println("Possivel sismo detectado!");
      enviarMensagemBT("Sismo Detectado!");
      buzzerSomSismo();
    }
  } else {
    sismoDetectado = false;
  }
  
  delay(200); // Estabilidade de Leitura
}
