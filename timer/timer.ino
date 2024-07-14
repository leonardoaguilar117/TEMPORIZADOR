/*
  TEMPORIZADOR AJUSTABLE PARA ACTIVACIÓN DE RELEVADOR POR n TIEMPO
  x DIAS DE LA SEMANA, A LAS yy:yy Y CON FINALIZACION A LAS ZZ:ZZ

  REALIZADO POR: LEONARDO AGUILAR MARTÍNEZ
  PARA: REAL PROTECTION 
  CON FECHA: 07/07/2024
  GITHUB: https://github.com/leonardoaguilar117
*/

/*LIBRERIAS USADAS */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <EEPROM.h>

/*OBJETOS USABLES*/
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 20, 4);

#define button1 0 /*IZQUIERDA*/
#define button2 1 /*DERECHA*/
#define button3 2 /*INTRO*/
#define button4 3 /*REGRESAR*/
#define button5 5 /*MOSTRAR RELOJ*/
#define relayAlarm 4 /*PIN PARA RELEVADOR ABRIR*/
#define relayAlarmB 6 /*PIN PARA RELEVADOR CERRAR*/
#define manualKey 7 /*ENCEDIDO POR LLAVE*/

/*VARIABLES ADICIONALES*/
bool startMenu = true;
int count = 0;
int numberOfalarms = 0;
int dayCount = 0;
int auxHour = 0;
int auxMinute = 0;
int auxEndHour = 0;
int auxEndMinute = 0;

/*BANDERAS*/
bool exitAssignHour = false;
bool exitAssignEnd = false;

/*ESTRUCTURA DE ALARMA*/
struct Alarm {
  int activeDay;  // DÍAS ACTIVOS (0-6, DONDE 0 = DOMINGO)
  int startHour;       // HORA DE INICIO DE LA ALARMA
  int startMinute;     // MINUTO DE INICIO DE LA ALARMA
  int endHour;         // HORA DE FIN DE LA ALARMA
  int endMinute;       // MINUTO DE FIN DE LA ALARMA
};

/*7 ALARMAS DIFERENTES*/
Alarm alarms[20];

/*--------CONFIGURACION--------------*/
void setup(){
  lcd.begin(20, 4);
  lcd.backlight();
  pinMode(button1, INPUT_PULLUP); /*IZQUIERDA*/
  pinMode(button2, INPUT_PULLUP); /*DERECHA*/
  pinMode(button3, INPUT_PULLUP); /*INTRO*/
  pinMode(button4, INPUT_PULLUP); /*REGRESAR*/
  pinMode(button5, INPUT_PULLUP); /*MOSTRAR RELOJ*/
  pinMode(manualKey, INPUT);
  pinMode(relayAlarm, OUTPUT); /*SALIDA PARA RELEVADOR*/
  pinMode(relayAlarmB, OUTPUT); /*SALIDA PARA RELEVADOR */

  /*APAGAR AMBOS RELÉS INICIALMENTE*/
  digitalWrite(relayAlarm, HIGH);
  digitalWrite(relayAlarmB, HIGH);
  
  /*CONFIGURACION DEL MODULO RTC EN CASO DE SER LA PRIMERA VEZ DE USO*/
  if (!rtc.begin()) {
    lcd.print("ERROR CON RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(2024, 07, 12, 10, 32, 00));
  }
}

/*--------REPETICION----------------*/
void loop(){
  
  if (startMenu){
    rpSolutions();
    startMenu = false;
  }
  mainMenu();
  activateAlarm();

  if (digitalRead(button5) == LOW) {
    showCurrentTime();
  }

  //ENCENDIDO MANUAL 
  if(digitalRead(button5) == LOW && digitalRead(button3) == LOW){
    lcd.clear();
    lcd.setCursor(4, 1);
    lcd.print("ABRIENDO");
    digitalWrite(relayAlarm, HIGH);
    digitalWrite(relayAlarmB, LOW);
    delay(1000);
  }

  //APAGADO MANUAL
  if(digitalRead(button5) == LOW && digitalRead(button4) == LOW){
    lcd.clear();
    lcd.setCursor(4, 1);
    lcd.print("CERRANDO");
    digitalWrite(relayAlarm, LOW);
    digitalWrite(relayAlarmB, HIGH);
    delay(1000);
  }

  
}

 

/*---------MOSTRAR REAL PROTECTION---------------*/
void rpSolutions(){
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Real");
  lcd.setCursor(6, 2);
  lcd.print("Protection");
  delay(1500);
}

/*---------MENU INICIAL----------------*/
void mainMenu(){
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("Crear");
  lcd.setCursor(1, 2);
  lcd.print("Rutina");

  lcd.setCursor(12, 1);
  lcd.print("Borrar");
  lcd.setCursor(12, 2);
  lcd.print("Rutinas");
  
  /* CONTADOR AUMENTA O DISMINUYE
     (ALTERNA ENTRE CREAR O BORRAR)
  */
  if (digitalRead(button1) == LOW) {
    if (count < 1) {
      count++;
    }
    delay(100); 
  }

  if (digitalRead(button2) == LOW) {
    if (count > 0) {
      count--;
    }
    delay(100); 
  }

  /*ENTRAR A CREAR RUTINA O BORRAR RUTINAS DEPENDIENDO COUNT*/
  if (digitalRead(button3) == LOW && count == 0){
    delay(1000);
    howManyAlarms();
  }

  if (digitalRead(button3) == LOW && count == 1){
    delay(1200);
    reset();
  }
  
  /*ITERADOR PARA CURSOR (SOLO IMPRESION)*/
  lcd.setCursor(count == 0 ? 3 : 15, 3);
  lcd.print("o");
  delay(600);
}

/*--------SELECCIONAR DIA DE LA SEMANA----------------*/
void daysOfWeek(){
  while (true) { /*SE REPITE HASTA QUE SE DE BACK (RETURN)*/
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Seleccione el dia");
    lcd.setCursor(3, 2);
    lcd.print("D L M M J V S");

    /*CONTADORES PARA OPCION DE DIA DE LA SEMANA*/
    if (digitalRead(button1) == LOW) {
      if (dayCount < 6) {  
        dayCount++;
      }
      delay(110); 
    }

    if (digitalRead(button2) == LOW) {
      if (dayCount > 0) {
        dayCount--;
      }
      delay(110); 
    }

    
    if(digitalRead(button3) == LOW){
      delay(1000);
      assignHour();
      if (exitAssignHour) {
        return;
      }
    }
   
    lcd.setCursor(dayCount * 2 + 3, 3); 
    lcd.print("o");
    delay(600); 
  }
}

void howManyAlarms(){
  while(true){
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Cuantas alarmas");
    lcd.setCursor(3,1);
    lcd.print("se agregan? ");
    lcd.print(numberOfalarms);

    /*CONTADORES PARA CANTIDAD DE ALARMAS */
    if (digitalRead(button1) == LOW) {
      if (count < 19) {
       count++;
      }
      delay(80); 
    }

    if (digitalRead(button2) == LOW) {
      if (count > 0) {
       count--;
      }
      delay(80); 
    }

    numberOfalarms = count;

    if(numberOfalarms == 0 && digitalRead(button3) == LOW){
      return;
    }
    /*SI ACEPTA PROCEDEMOS A LOS DIAS DE LA SEMANA, SI NO, REGRESAMOS*/
    if (digitalRead(button3) == LOW){
      delay(800);
      addAlarm(numberOfalarms);
      return;
    }

    if (digitalRead(button4) == LOW){
      return;
    }

    lcd.setCursor(3, 13);
    lcd.print("Aceptar (Intro)");
    delay(600);
  }
}

/*FUNCION PARA AÑADIR ALARMAS A ARRAY DE ALARMS*/
void addAlarm(int numberOfAlarms){
  int direccion = 0;
  for(int i=0; i<numberOfAlarms; i++){

    /*ASIGNACIÓN DE VALORES PARA CADA ALARMA (BUCLE PASA POR TODAS 
      LAS ALARMAS)*/
    daysOfWeek();
    alarms[i].activeDay = dayCount;
    alarms[i].startHour = auxHour;
    alarms[i].startMinute = auxMinute;
    alarms[i].endHour = auxEndHour;
    alarms[i].endMinute = auxEndMinute;

    EEPROM.put(direccion, alarms[i]);
    direccion += sizeof(Alarm);

    lcd.clear();
    lcd.setCursor(1,0);
    lcd.println("Alarma agregada a");
    lcd.setCursor(4, 1);
    lcd.println("las: " + (auxHour < 10 ? "0" + String(auxHour) : String(auxHour)) + ":" + (auxMinute < 10 ? "0" + String(auxMinute) : String(auxMinute)));
    lcd.setCursor(1, 3);
    lcd.println("Concluye: " + (auxEndHour < 10 ? "0" + String(auxEndHour) : String(auxEndHour)) + ":" + (auxEndMinute < 10 ? "0" + String(auxEndMinute) : String(auxEndMinute)));
    delay(2000); // TIEMPO PARA MOSTRAR EL MENSAJE DE ALARMA AGREGADA
  }

  lcd.clear();
  lcd.setCursor(2,1);
  lcd.print("Todas las alarmas");
  lcd.setCursor(2,2);
  lcd.print("han sido agregadas");
  delay(2000); 
  return;
}

/*--------SELECCIONAR DIA DE LA SEMANA----------------*/
void assignHour(){
  exitAssignHour = false;
  while (true) {
    lcd.clear();
    lcd.setCursor(1, 1);
    lcd.print("Activar a");
    lcd.setCursor(1, 2);
    lcd.print("las: ");
    lcd.setCursor(7, 2);
    lcd.print(auxHour < 10 ? "0" : "");
    lcd.print(auxHour);
    lcd.print(":");
    lcd.print(auxMinute < 10 ? "0" : "");
    lcd.print(auxMinute);

    if (digitalRead(button2) == LOW) {
      auxHour = (auxHour + 1) % 24; // AUMENTAR LA HORA Y REINICIAR A 0 SI LLEGA A 24
      delay(100); 
    }

    if (digitalRead(button1) == LOW) {
      auxMinute = (auxMinute + 1) % 60; // AUMENTAR EL MINUTO Y REINICIAR A 0 SI LLEGA A 60
      delay(100); 
    }

    if (digitalRead(button3) == LOW) {
      delay(1000);
      assignEnd();
      if (exitAssignEnd) {
        exitAssignHour = true;
        return;
      }
    }

    delay(500); 
  }
}

/*FUNCION PARA AGREGAR EL TIEMPO PARA TERMINAR EL TEMPORIZADOR*/
void assignEnd() {
  exitAssignEnd = false;
  while (true) {
    lcd.clear();
    lcd.setCursor(1, 1);
    lcd.print("Terminar a");
    lcd.setCursor(1, 2);
    lcd.print("las: ");
    lcd.setCursor(7, 2);
    lcd.print(auxEndHour < 10 ? "0" : "");
    lcd.print(auxEndHour);
    lcd.print(":");
    lcd.print(auxEndMinute < 10 ? "0" : "");
    lcd.print(auxEndMinute);

    if (digitalRead(button2) == LOW) {
      auxEndHour = (auxEndHour + 1) % 24; // AUMENTAR LA HORA Y REINICIAR A 0 SI LLEGA A 24
      delay(100); 
    }

    if (digitalRead(button1) == LOW) {
      auxEndMinute = (auxEndMinute + 1) % 60; // AUMENTAR EL MINUTO Y REINICIAR A 0 SI LLEGA A 60
      delay(100); 
    }

    if (digitalRead(button4) == LOW) {
      lcd.clear();
      exitAssignEnd = true;
      return;
    }

    if (digitalRead(button3) == LOW) {
      exitAssignEnd = true;
      return;
    }

    delay(500); 
  }
}

/*--------OPCION DE BORRADO DE RUTINAS-----------*/
void reset(){
  while(true){
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Estas");
    lcd.setCursor(9,0);
    lcd.print("seguro?");

    lcd.setCursor(11, 2);
    lcd.print("No (Back)");

    lcd.setCursor(0, 2);
    lcd.print("Si (Intro)");

    /*CONTADORES PARA OPCION BORRAR O SALIR*/
    if (digitalRead(button1) == LOW) {
      if (count < 1) {
       count++;
      }
      delay(100); 
    }

    if (digitalRead(button2) == LOW) {
      if (count > 0) {
       count--;
      }
      delay(100); 
    }

    /* BORRAR / SALIR */
    if (digitalRead(button4) == LOW) {
      return;
    }  

    if (digitalRead(button3) == LOW) {
      lcd.clear();
      lcd.setCursor(6,1);
      lcd.print("BORRADO");
      delay(900);

      // LIMPIAR TODAS LAS ALARMAS
      for (int i = 0; i < EEPROM.length() ; i++) {
        EEPROM.write(i, 0);
      }
      return;
    }

    delay(600);
  }
}

void activateAlarm(){
  int direccion = 0;
    DateTime now = rtc.now();

    int currentDay = now.dayOfTheWeek(); // OBTENER EL DÍA ACTUAL (0-6)
    int currentHour = now.hour();        // OBTENER LA HORA ACTUAL
    int currentMinute = now.minute();    // OBTENER EL MINUTO ACTUAL

    /*ITERA POR TODAS LAS ALARMAS Y COMPRUEBA SI HAY UNA ALARMA A ESTA HORA, ACTIVA
      DE LO CONTRARIO APAGA*/
    for (int i = 0; i < 7; i++) {
      EEPROM.get(direccion, alarms[i]);
      direccion += sizeof(Alarm);
        if (alarms[i].activeDay == currentDay) {
            if (currentHour == alarms[i].startHour && currentMinute == alarms[i].startMinute) {
                digitalWrite(relayAlarm, HIGH);
                digitalWrite(relayAlarmB, LOW);
                lcd.clear();
                lcd.setCursor(3, 1);
                lcd.print("ALARMA ACTIVA");
                delay(1000);
            }
            if (currentHour == alarms[i].endHour && currentMinute == alarms[i].endMinute) {
              digitalWrite(relayAlarm, LOW);
              digitalWrite(relayAlarmB, HIGH);
              lcd.clear();
            }
        }
    }
}

/* FUNCION PARA MOSTRAR LA HORA ACTUAL */
void showCurrentTime() {
  DateTime now = rtc.now();

  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("HORA ACTUAL: ");
  lcd.setCursor(7, 2);
  lcd.print(now.hour() < 10 ? "0" : "");
  lcd.print(now.hour());
  lcd.print(":");
  lcd.print(now.minute() < 10 ? "0" : "");
  lcd.print(now.minute());
  
  delay(2000); 
}
