#include <Bluepad32.h>

const int M1_RPWM = 32;
const int M1_LPWM = 33;
const int M2_RPWM = 25;
const int M2_LPWM = 26;

const int frec = 5000;       
const int res = 8;    

const int canal_M1_R = 0; 
const int canal_M1_L = 1;
const int canal_M2_R = 2; 
const int canal_M2_L = 3;

float velocidades[] = {0.50, 0.80, 0.90}; 
int limitePWM = 0; 


const float velocidad_motor_1 = 1.00; 
const float velocidad_motor_2 = 1.00; 

GamepadPtr joystick;

void cambiarRGB() {
    if (!joystick) return;

    if (limitePWM == 0) {
        joystick->setColorLED(0, 0, 255);
    } else if (limitePWM == 1) {
        joystick->setColorLED(0, 255, 0);
    } else if (limitePWM == 2) {
        joystick->setColorLED(255, 0, 0);
    }
}

void onConnectedGamepad(GamepadPtr gp) { 
    joystick = gp; 
    cambiarRGB();
}

void onDisconnectedGamepad(GamepadPtr gp) { joystick = nullptr; }

void setup() {
Serial.begin(115200);
  ledcSetup(canal_M1_R, frec, res); ledcSetup(canal_M1_L, frec, res);
  ledcSetup(canal_M2_R, frec, res); ledcSetup(canal_M2_L, frec, res);

  ledcAttachPin(M1_RPWM, canal_M1_R); ledcAttachPin(M1_LPWM, canal_M1_L);
  ledcAttachPin(M2_RPWM, canal_M2_R); ledcAttachPin(M2_LPWM, canal_M2_L);

  BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
}

void loop() {
    BP32.update();

    if (joystick && joystick->isConnected()) {
        
       if (joystick->r1()) {
            limitePWM++; 
            if (limitePWM > 2) limitePWM = 0;             
            cambiarRGB();
          
            Serial.print("modos de velocidad :");
            Serial.print(limitePWM +1);
            Serial.print(velocidades[limitePWM]*100);
          
            delay(250);
       }

       int stickY_M1 = -joystick->axisY();  
       int stickY_M2 = -joystick->axisRY(); 

Serial.print("izq:");
Serial.print(stickY_M1);
Serial.print("der:");
Serial.println(stickY_M2);

        if (abs(stickY_M1) < 30) stickY_M1 = 0;
        if (abs(stickY_M2) < 30) stickY_M2 = 0;

        float velocidadActual = velocidades[limitePWM];
        int maxPWM = 255 * velocidadActual;

       int pwmM1 = map(stickY_M1, -512, 511, -maxPWM, maxPWM) * velocidad_motor_1;
       int pwmM2 = map(stickY_M2, -512, 511, -maxPWM, maxPWM) * velocidad_motor_2;

        if (pwmM1 > 0) {
            ledcWrite(canal_M1_R, pwmM1); 
            ledcWrite(canal_M1_L, 0);

        } else if (pwmM1 < 0) {
            ledcWrite(canal_M1_R, 0);    
             ledcWrite(canal_M1_L, abs(pwmM1));

        } else {
            ledcWrite(canal_M1_R, 0);    
             ledcWrite(canal_M1_L, 0);
        }

        if (pwmM2 > 0) {
            ledcWrite(canal_M2_R, pwmM2); 
            ledcWrite(canal_M2_L, 0);
        
        } else if (pwmM2 < 0) {
            ledcWrite(canal_M2_R, 0);     
            ledcWrite(canal_M2_L, abs(pwmM2));
        
        } else {
            ledcWrite(canal_M2_R, 0);     
            ledcWrite(canal_M2_L, 0);
  }

    } else {
        ledcWrite(canal_M1_R, 0); 
        ledcWrite(canal_M1_L, 0);
        
        ledcWrite(canal_M2_R, 0); 
        ledcWrite(canal_M2_L, 0);
       }
    
    delay(30); 
}