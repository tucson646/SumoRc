#include <Bluepad32.h>

// Puntero global para el mando
GamepadPtr miMando = nullptr;

// Función que se ejecuta automáticamente cuando el mando se conecta
void onConnectedGamepad(GamepadPtr gp) {
    if (miMando == nullptr) {
        Serial.println("\n--------------------------------------------------");
        Serial.println("¡¡CONEXIÓN EXITOSA!!");
        Serial.print("Mando detectado: ");
        Serial.println(gp->getModelName()); // Nos dirá si entró como Switch, Xbox, etc.
        Serial.println("--------------------------------------------------");
        miMando = gp;
    }
}

// Función que se ejecuta automáticamente cuando el mando se desconecta
void onDisconnectedGamepad(GamepadPtr gp) {
    if (miMando == gp) {
        Serial.println("\n--------------------------------------------------");
        Serial.println("¡ALERTA: Mando desconectado del ESP32!");
        Serial.println("--------------------------------------------------");
        miMando = nullptr;
    }
}

void setup() {
  // Inicializar puerto serie a 115200 baudios
  Serial.begin(115200);
  while (!Serial) {
    ; // Esperar a que el monitor serie se abra
  }

  Serial.println("\n=== INICIANDO PRUEBA DE JOYSTICK (v4.1.0) ===");

  // Configurar Bluepad32 con las funciones de conexión
  BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
  
  // Limpia cualquier emparejamiento previo para forzar una sincronización limpia
  BP32.forgetBluetoothKeys(); 

  Serial.println("ESP32 listo y buscando Bluetooth...");
  Serial.println("👉 Pon tu GameSir en modo emparejamiento (Prueba con Y + HOME).");
}

void loop() {
  // OBLIGATORIO: Actualiza el motor de Bluepad32 en cada ciclo
  BP32.update();

  // Si el mando está conectado, leemos sus valores y los mandamos a la terminal
  if (miMando != nullptr && miMando->isConnected()) {
    
    // 1. Leer Joysticks (Rango de -512 a 511)
    int joyIzquierdoY = miMando->axisY();  // Eje vertical izquierdo
    int joyDerechoY   = miMando->axisRY(); // Eje vertical derecho

    // 2. Leer Botones (Usamos máscaras de bits directas para evitar errores de nombres)
    uint16_t botones = miMando->buttons();
    
    // En Bluepad32 v4, el bit 0x0002 corresponde siempre al botón superior derecho (R1 / RB)
    bool botonR1_Presionado = (botones & 0x0002); 

    // 3. Imprimir en el Monitor Serie de forma ordenada
    Serial.print("Eje Y Izq: ");
    Serial.print(joyIzquierdoY);
    
    Serial.print("\t | Eje Y Der: ");
    Serial.print(joyDerechoY);
    
    Serial.print("\t | Botón R1/RB: ");
    if (botonR1_Presionado) {
      Serial.println("🟢 PRESIONADO");
    } else {
      Serial.println("❌ SUELTO");
    }

    // Retraso de 100ms para que la terminal sea legible y no vaya a toda velocidad
    delay(100); 
  }
}