#include <Arduino.h>
#include <Bluepad32.h>

// --- ASIGNACIÓN DE PINES (BTS7960) ---
const int PIN_M1_RPWM = 32; // Adelante Motor 1
const int PIN_M1_LPWM = 33; // Atrás Motor 1

const int PIN_M2_RPWM = 25; // Adelante Motor 2
const int PIN_M2_LPWM = 26; // Atrás Motor 2

// --- CONFIGURACIÓN DE PWM ---
const int PWM_FREQ = 5000;
const int PWM_RES = 8; // 0 a 255

const int CH_M1_RPWM = 0;
const int CH_M1_LPWM = 1;
const int CH_M2_RPWM = 2;
const int CH_M2_LPWM = 3;

// --- CALIBRACIÓN DE VELOCIDAD (Por si un motor va más rápido) ---
float compensacionMotor1 = 1.0; 
float compensacionMotor2 = 1.0; 

// --- LÍMITE DE VELOCIDAD INICIAL (90% = 230) ---
int maxPWM = 230; 

bool botonR1PresionadoAntes = false;
ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// Función de control para el BTS7960
void controlarMotorBTS(int ch_rpwm, int ch_lpwm, int valorEje, float compensacion) {
    if (abs(valorEje) < 30) {
        ledcWrite(ch_rpwm, 0);
        ledcWrite(ch_lpwm, 0);
        return;
    }

    int velocidad = map(abs(valorEje), 30, 512, 0, maxPWM);
    velocidad = (int)(velocidad * compensacion);
    velocidad = constrain(velocidad, 0, maxPWM);

    if (valorEje < 0) { 
        ledcWrite(ch_lpwm, 0);         
        ledcWrite(ch_rpwm, velocidad); 
    } else { 
        ledcWrite(ch_rpwm, 0);         
        ledcWrite(ch_lpwm, velocidad); 
    }
}

void onConnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            myControllers[i] = ctl;
            Serial.printf("\n[CONEXIÓN] Controlador conectado en índice: %d\n", i);
            break;
        }
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            myControllers[i] = nullptr;
            Serial.printf("\n[DESCONEXIÓN] Controlador desconectado del índice: %d\n", i);
            break;
        }
    }
}

void procesarGamepad(ControllerPtr ctl) {
    // --- MONITOREO DE PULSACIÓN: BOTÓN R1 ---
    if ((ctl->buttons() & BUTTON_SHOULDER_R) && !botonR1PresionadoAntes) {
        
        // Rotación de velocidad: 50% (128) -> 70% (179) -> 90% (230)
        if (maxPWM == 128) {
            maxPWM = 179;        
        } 
        else if (maxPWM == 179) {
            maxPWM = 230;        
        } 
        else if (maxPWM == 230) {
            maxPWM = 128;        
        }

        // Imprime de forma inmediata la pulsación y el cambio de estado
        Serial.printf("\n=============================================\n");
        Serial.printf("[BOTÓN R1 PRESIONADO] -> Nuevo límite PWM: %d\n", maxPWM);
        Serial.printf("=============================================\n\n");
        
        botonR1PresionadoAntes = true;
    } else if (!(ctl->buttons() & BUTTON_SHOULDER_R)) {
        botonR1PresionadoAntes = false;
    }

    // --- MONITOREO DE STICKS (Cada 250 milisegundos) ---
    static uint32_t ultimoTiempoPrint = 0;
    if (millis() - ultimoTiempoPrint > 250) { 
        // %4d alinea los números visualmente aunque pasen de positivos a negativos
        Serial.printf("Sticks -> Izquierdo Y (M1): %4d | Derecho Y (M2): %4d | Limite Max PWM: %d\n", 
                      ctl->axisY(), ctl->axisRY(), maxPWM);
        ultimoTiempoPrint = millis();
    }

    // Control real de los motores
    controlarMotorBTS(CH_M1_RPWM, CH_M1_LPWM, ctl->axisY(), compensacionMotor1);
    controlarMotorBTS(CH_M2_RPWM, CH_M2_LPWM, ctl->axisRY(), compensacionMotor2);
}

void setup() {
    // Inicializar puerto serie a 115200 baudios
    Serial.begin(115200);
    Serial.println("\n--- INICIANDO SISTEMA ESP32 + BTS7960 ---");

    // Configuración de PWM
    ledcSetup(CH_M1_RPWM, PWM_FREQ, PWM_RES);
    ledcSetup(CH_M1_LPWM, PWM_FREQ, PWM_RES);
    ledcSetup(CH_M2_RPWM, PWM_FREQ, PWM_RES);
    ledcSetup(CH_M2_LPWM, PWM_FREQ, PWM_RES);
    
    ledcAttachPin(PIN_M1_RPWM, CH_M1_RPWM);
    ledcAttachPin(PIN_M1_LPWM, CH_M1_LPWM);
    ledcAttachPin(PIN_M2_RPWM, CH_M2_RPWM);
    ledcAttachPin(PIN_M2_LPWM, CH_M2_LPWM);

    // Inicializar Bluepad32
    BP32.setup(&onConnectedController, &onDisconnectedController);
    Serial.println("Bluetooth iniciado. Buscando controles vinculados...");
}

void loop() {
    BP32.update();

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        ControllerPtr myController = myControllers[i];
        if (myController && myController->isConnected() && myController->isGamepad()) {
            procesarGamepad(myController);
        }
    }
    delay(10);
}