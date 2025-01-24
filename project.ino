#include <SoftwareSerial.h> // Só usar se precisar de porta Serial extra

// Definição de pinos
const int LDR_PIN          = A4; // LDR
const int FLAME_SENSOR_PIN = 2;  // Sensor de chama
const int VIB_SENSOR_PIN   = 3;  // Sensor de vibração
const int BUZZER_PIN       = 9;  // Buzzer

// LEDs (iluminação pública)
const int LED_PINS[5]      = {4, 5, 6, 7, 8};

// Pinos do HC-05 ou HC-06 via SoftwareSerial
const int BT_RX = 10; // RX do Arduino (liga no TX do módulo Bluetooth)
const int BT_TX = 11; // TX do Arduino (liga no RX do módulo Bluetooth)

// Se for usar a Serial padrão (pinos 0 e 1), comente as linhas abaixo e use Serial diretamente.
SoftwareSerial BTSerial(BT_RX, BT_TX);

// Estados (flags)
bool luzesLigadas         = false;
bool sismoDetectado       = false;
bool fogoDetectado        = false;

// Limites
const int LUZ_THRESHOLD   = 500; // Valor de corte para o LDR (ajuste se necessário)
const int VIB_THRESHOLD   = 100; // (Não está a ser usado diretamente no digital)

// -------------------------------------------------------------------
// Função para enviar mensagens Bluetooth (e terminar com nova linha)
void enviarMensagemBT(const String &mensagem) {
  BTSerial.print(mensagem);
  BTSerial.print("\n");  // Importante para que o app possa ler até '\n'
}

// -------------------------------------------------------------------
// Buzzer: alerta para sismo (tom grave, modo sirene durante 5s)
void buzzerSomSismo() {
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {
    tone(BUZZER_PIN, 400, 500); // Tocar 500 ms a 400 Hz
    delay(600);                 // Pausa 600 ms
    noTone(BUZZER_PIN);
  }
}

// Buzzer: alerta para fogo (tom agudo, modo sirene durante 5s)
void buzzerSomFogo() {
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {
    tone(BUZZER_PIN, 800, 500); // Tocar 500 ms a 800 Hz
    delay(600);
    noTone(BUZZER_PIN);
  }
}

// -------------------------------------------------------------------
// Função para ligar luzes gradualmente, exibindo mensagens durante e após
void ligarLuzes() {
  // Mensagem inicial
  enviarMensagemBT("A ligar luzes...");
  Serial.println("A ligar luzes...");

  // Liga cada LED com um pequeno atraso
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PINS[i], HIGH);
    delay(800); // delay para simular ligação escalonada
  }

  // Estado atualizado
  luzesLigadas = true;

  // Mensagem final
  enviarMensagemBT("Luzes ligadas");
  Serial.println("Luzes ligadas");
}

// Função para desligar luzes gradualmente, exibindo mensagens durante e após
void desligarLuzes() {
  // Mensagem inicial
  enviarMensagemBT("A desligar luzes...");
  Serial.println("A desligar luzes...");

  // Desliga cada LED em ordem inversa
  for (int i = 4; i >= 0; i--) {
    digitalWrite(LED_PINS[i], LOW);
    delay(600); // delay para simular desligamento escalonado
  }

  // Estado atualizado
  luzesLigadas = false;

  // Mensagem final
  enviarMensagemBT("Luzes desligadas");
  Serial.println("Luzes desligadas");
}

// -------------------------------------------------------------------
void setup() {
  // Configuração de pinos
  pinMode(LDR_PIN, INPUT);
  pinMode(FLAME_SENSOR_PIN, INPUT);
  pinMode(VIB_SENSOR_PIN, INPUT);
  
  pinMode(BUZZER_PIN, OUTPUT);
  
  // LEDs como saída
  for (int i = 0; i < 5; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }

  // Serial normal (USB) para o monitor
  Serial.begin(9600);
  
  // Iniciar porta SoftwareSerial para Bluetooth
  BTSerial.begin(9600);

  // Mensagens iniciais
  enviarMensagemBT("Sistema Iniciado");
  Serial.println("Sistema de Gestao Ambiental Iniciado!");
}

// -------------------------------------------------------------------
void loop() {
  // --- 1) Leitura do LDR para acender/apagar luzes ---
  int ldrValue = analogRead(LDR_PIN);

  // Se está mais escuro que o limiar e as luzes ainda não estão ligadas...
  if (ldrValue < LUZ_THRESHOLD && !luzesLigadas) {
    ligarLuzes(); 
  } 
  // Se está mais claro que o limiar e as luzes estão ligadas...
  else if (ldrValue >= LUZ_THRESHOLD && luzesLigadas) {
    desligarLuzes();
  }

  // --- 2) Sensor de chama ---
  int flameValue = digitalRead(FLAME_SENSOR_PIN);
  // Supondo que LOW = chama detetada (depende do sensor)
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

  // --- 3) Sensor de vibração ---
  int vibValue = digitalRead(VIB_SENSOR_PIN);
  // Supondo HIGH = sismo detetado
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

  // Pausa para estabilidade da leitura
  delay(200);
}
