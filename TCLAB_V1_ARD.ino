/*
  Firmware de Controle de Temperatura TCLab
  Jeffrey Kantor, Versão Inicial
  John Hedengren, Modificado
  Out 2017

  Este firmware é carregado no Arduino do Laboratório de Controle de Temperatura para
  fornecer uma interface de alto nível para o Laboratório de Controle de Temperatura. O firmware
  verifica a porta serial em busca de comandos que não diferenciam maiúsculas de minúsculas:

  Q1        define o Aquecedor 1, faixa de 0 a 100% sujeito a limite (0-255 int)
  Q2        define o Aquecedor 2, faixa de 0 a 100% sujeito a limite (0-255 int)
  T1        obtém a Temperatura T1, retorna °C como string
  T2        obtém a Temperatura T2, retorna °C como string
  VER       obtém a versão do firmware como string
  X         para, entra no modo de espera

  Limites no aquecedor podem ser configurados com as constantes abaixo.
*/

// constantes
const String vers = "1.01";    // versão deste firmware
const int baud = 9600;         // taxa de transmissão serial
const char sp = ' ';           // separador de comando
const char nl = '\n';          // terminador de comando

// números dos pinos correspondentes aos sinais na Placa TC Lab
const int pinT1   = 0;         // Pino AD T1
const int pinT2   = 2;         // Pino AD T2
const int pinQ1   = 3;         // Pino PWM Q1
const int pinQ2   = 5;         // Pino PWM Q2
const int pinLED  = 9;         // Pino PWM LED

// variáveis globais
char Buffer[64];               // buffer para análise da entrada serial
String cmd;                    // comando
float pv;                      // valor do pino
float level;                   // nível do LED (0-100%)
float Q1 = 0;                  // valor escrito no pino Q1
float Q2 = 0;                  // valor escrito no pino Q2
int iwrite = 0;                // valor inteiro para escrita
float dwrite = 0;              // valor float para escrita
int n = 10;                    // número de amostras para cada medição de temperatura

void parseSerial(void) {
  int ByteCount = Serial.readBytesUntil(nl, Buffer, sizeof(Buffer));
  String read_ = String(Buffer);
  memset(Buffer, 0, sizeof(Buffer));
   
  // separar comando dos dados associados
  int idx = read_.indexOf(sp);
  cmd = read_.substring(0, idx);
  cmd.trim();
  cmd.toUpperCase();

  // extrair dados. toInt() retorna 0 em caso de erro
  String data = read_.substring(idx + 1);
  data.trim();
  pv = data.toFloat();
}

// soma da saída = 300
// Q1_max = 200
// Q2_max = 100
void dispatchCommand(void) {
  if (cmd == "Q1") {
    Q1 = max(0.0, min(100.0, pv));
    iwrite = int(Q1 * 2.0); // 2.55 max
    iwrite = max(0, min(255, iwrite));    
    analogWrite(pinQ1, iwrite);
    Serial.println(Q1);
  }
  else if (cmd == "Q2") {
    Q2 = max(0.0, min(100.0, pv));
    iwrite = int(Q2 * 1.0); // 2.55 max
    iwrite = max(0, min(255, iwrite));
    analogWrite(pinQ2, iwrite);
    Serial.println(Q2);
  }
  else if (cmd == "T1") {
    float mV = 0.0;
    float degC = 0.0;
    for (int i = 0; i < n; i++) {
      mV = (float) analogRead(pinT1) * (3300.0/1024.0);
      degC = degC + (mV - 500.0)/10.0;
    }
    degC = degC / float(n);   
    Serial.println(degC);
  }
  else if (cmd == "T2") {
    float mV = 0.0;
    float degC = 0.0;
    for (int i = 0; i < n; i++) {
      mV = (float) analogRead(pinT2) * (3300.0/1024.0);
      degC = degC + (mV - 500.0)/10.0;
    }
    degC = degC / float(n);
    Serial.println(degC);
  }
  else if ((cmd == "V") or (cmd == "VER")) {
    Serial.println("Versão do Firmware TCLab " + vers);
  }
  else if (cmd == "LED") {
    level = max(0.0, min(100.0, pv));
    iwrite = int(level * 0.5);
    iwrite = max(0, min(50, iwrite));    
    analogWrite(pinLED, iwrite);
    Serial.println(level);
  }  
  else if (cmd == "X") {
    analogWrite(pinQ1, 0);
    analogWrite(pinQ2, 0);
    Serial.println("Parar");
  }
}

// verificar temperatura e desligar aquecedores se acima do limite máximo
void checkTemp(void) {
    float mV = (float) analogRead(pinT1) * (3300.0/1024.0);
    float degC = (mV - 500.0)/10.0;
    if (degC >= 100.0) {
      Q1 = 0.0;
      Q2 = 0.0;
      analogWrite(pinQ1, 0);
      analogWrite(pinQ2, 0);
      Serial.println("Temperatura Alta 1 (>100C): ");
      Serial.println(degC);
    }
    mV = (float) analogRead(pinT2) * (3300.0/1024.0);
    degC = (mV - 500.0)/10.0;
    if (degC >= 100.0) {
      Q1 = 0.0;
      Q2 = 0.0;
      analogWrite(pinQ1, 0);
      analogWrite(pinQ2, 0);
      Serial.println("Temperatura Alta 2 (>100C): ");
      Serial.println(degC);
    }
}

// inicialização do arduino
void setup() {
  analogReference(EXTERNAL);
  Serial.begin(baud); 
  while (!Serial) {
    ; // aguarde a conexão da porta serial.
  }
  analogWrite(pinQ1, 0);
  analogWrite(pinQ2, 0);
}

// loop principal do arduino
void loop() {
  parseSerial();
  dispatchCommand();
  checkTemp();
}
