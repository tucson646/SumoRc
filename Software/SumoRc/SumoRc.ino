#include <Bluepad32.h>

const int M1_RPWM = 32;
const int M1_LPWM = 33;
const int M2_RPWM = 25;
const int M2_LPWM = 26;

const int M1_EN  = 27;
const int M2_EN  = 14;

const int PWM_50 = 127; 
const int PWM_70 = 178; 
const int PWM_90 = 229;

const int led = 2;

int PWMactual = PWM_50;
bool botonbloqueo = false;


void setup(){

}

void loop() {
  BP32.update();

  //joystick desconectado
  if (miMando == nullptr) {
    detenerMotores();
    
   unsigned long tiempoActual = millis();
    if (tiempoActual - tiempoAnteriorLed >= 200) { 
      tiempoAnteriorLed = tiempoActual;
      estadoLed = !estadoLed;
      digitalWrite(LED_PIN, estadoLed);
    }
    return; 
  }

  //joystick conectado
  if (miMando->isConnected()) {
    digitalWrite(LED_PIN, HIGH); 
    
    digitalWrite(M1_EN, HIGH);
    digitalWrite(M2_EN, HIGH);
    
    //cambiar pwm con R1
    uint16_t botones = miMando->buttons();

    if (botones & 0x0002) { //r1
      if (!botonBloqueo) { 
        botonBloqueo = true; 
        
        if (PWMactual == PWM_50) { 
          PWMactual = PWM_70; 
        }
        else if (PWMactual == PWM_70) { 
          PWMactual = PWM_90; 
        }
        else { 
          PWMactual = PWM_50; 
        }
      }
    } else {
      botonBloqueo = false; 
    }

    // --- CONTROL DE MOTORES ---
    int joystickIzquierdoY = -miMando->axisY();  
    int joystickDerechoY   = -miMando->axisRY(); 

    procesarmotor(joystickIzquierdoY, M1_RPWM, M1_LPWM);
    procesarMotor(joystickDerechoY, M2_RPWM, M2_LPWM);

    delay(10); 
  } else {
    detenerMotores();
  }
}

void procesarMotor(int valorJoystick, int pinRPWM, int pinLPWM) {
  if (abs(valorJoystick) < 50) {
    analogWrite(pinRPWM, 0);
    analogWrite(pinLPWM, 0);
    return;
  }

  int pwmCalculado = map(abs(valorJoystick), 50, 511, 0, PWMactual);

  if (valorJoystick > 0) { 
    analogWrite(pinRPWM, pwmCalculado);
    analogWrite(pinLPWM, 0);
  } else { 
    analogWrite(pinRPWM, 0);
    analogWrite(pinLPWM, pwmCalculado);
  }
}

void detenerMotores() {
  analogWrite(M1_RPWM, 0);
  analogWrite(M1_LPWM, 0);
  analogWrite(M2_RPWM, 0);
  analogWrite(M2_LPWM, 0);
  
  digitalWrite(M1_EN, LOW);
  digitalWrite(M2_EN, LOW);
}