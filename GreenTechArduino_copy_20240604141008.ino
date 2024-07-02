#include <SoftwareSerial.h>

const int pinoRX = 2;
const int pinoTX = 3;
const int pinoRele = 8;
const int pino5V = 7;
const int pinoAnalog = A0;

int dadoBluetooth = 0;

SoftwareSerial bluetooth(pinoRX, pinoTX);

int ValAnalogIn;
int Porcento;

const int ledR = 11;
boolean estadoledR = false;
boolean plantaIrrigada = false;

boolean estadoRele = false; // Indica se o relé está ligado ou desligado
boolean comandoRecebido = false; // Indica se o comando foi recebido

unsigned long tempoInicioIrrigacao = 0; // Tempo de início da irrigação
const unsigned long tempoIrrigacao = 5000; // Tempo em milissegundos (5 segundos)

void setup() {
    Serial.begin(9600);
    bluetooth.begin(9600);
    bluetooth.print("$$$");
    delay(100);

    pinMode(pinoRele, OUTPUT);
    pinMode(pino5V, OUTPUT);
    pinMode(ledR, OUTPUT);
    digitalWrite(pino5V, HIGH);
    digitalWrite(pinoRele, HIGH); // Relé desligado inicialmente
    digitalWrite(ledR, HIGH);
}

void loop() {
    ValAnalogIn = analogRead(pinoAnalog);

    // Verifica se a leitura do sensor é válida
    if (ValAnalogIn >= 0 && ValAnalogIn <= 1023) {
        Porcento = map(ValAnalogIn, 1023, 0, 0, 100);
    } else {
        // Tratar leitura inválida (ex: exibir mensagem de erro)
        Serial.println("Leitura do sensor inválida!");
        return;
    }

    // Comando via Bluetooth para forçar irrigação
    if (bluetooth.available()) {
        dadoBluetooth = bluetooth.read();
        if (dadoBluetooth == '1' && !comandoRecebido) {
            estadoRele = true;
            digitalWrite(pinoRele, LOW); // Liga o relé
            plantaIrrigada = false;
            comandoRecebido = true;
            tempoInicioIrrigacao = millis(); // Armazena o tempo de início da irrigação forçada
            Serial.println("Irrigação Forçada - Relé Ativado!");
        }
    }

    // Se o relé estiver ligado e o comando já foi recebido, desliga o relé após o tempo definido
    if (estadoRele && comandoRecebido) {
        if (millis() - tempoInicioIrrigacao >= tempoIrrigacao) {
            digitalWrite(pinoRele, HIGH); // Desliga o relé
            estadoRele = false;
            comandoRecebido = false;
            Serial.println("Irrigação Finalizada - Relé Desligado!");
        }
    }

    // Controle automático do relé baseado na umidade
    if (Porcento < 45 && !comandoRecebido) {
        if (!estadoRele) {
            estadoRele = true;
            digitalWrite(pinoRele, LOW); // Liga o relé
            Serial.println("Umidade baixa - Relé Ativado!");
        }
    } else if (Porcento >= 45 && !comandoRecebido) {
        if (estadoRele) {
            estadoRele = false;
            digitalWrite(pinoRele, HIGH); // Desliga o relé
            Serial.println("Umidade suficiente - Relé Desligado!");
        }
    }

    enviaStatus(Porcento, estadoRele);
    delay(1000);
}

void enviaStatus(int porcentagem, boolean estadoRele) {
    if (estadoRele) {
        bluetooth.print("irrigando;");
        Serial.print("irrigando;");
    } else {
        bluetooth.print("irrigado;");
        Serial.print("irrigado;");
    }
    bluetooth.println(porcentagem);
    Serial.println(porcentagem);
    delay(1500);
}
