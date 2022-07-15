
#include <LiquidCrystal.h>
#include <Keypad.h>
#include "AsyncTaskLib.h"
#include <math.h>
#include <EEPROM.h>

//Pantalla
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 8, en = 7, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

/*  The circuit:
 * LCD RS(4) pin to digital pin 8
 * LCD Enable(6) pin to digital pin 7
 * LCD D4(11) pin to digital pin 5
 * LCD D5(12) pin to digital pin 4
 * LCD D6(13) pin to digital pin 3
 * LCD D7(14) pin to digital pin 2
 * LCD R/W(5) pin to ground
 * LCD VSS(1) pin to ground
 * LCD VCC(2) pin to 5V
 * LCD 15 pin to 5v
 * LCD 16 pin to gnd
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 * 
  */

//Keypad
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {30, 32, 34, 36}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {31,33,35,37}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
int pos=0;

//RGB
const int rojo=11 ;
const int verde=12;
const int azul=13;

//Temperatura https://learn.sunfounder.com/lesson-10-analog-temperature-sensor-2/
#define analogPin A0 //the thermistor attach to
#define beta 4090 //the beta of the thermistor
#define resistance 10 //the value of the pull-down resistorvoid setup()

//Luz https://learn.sunfounder.com/lesson-21-photoresistor-sensor/
#define photocellPin         A1

//Buzzer
#define buzzerPin 10

//menu
const char frames[6][16]={{"UTempHigh      "},{"UTempLow       "},{"ULuz           "},{"mostrar        "},{"guardar y salir "},{"reset           "}};
int page=1;
int frame=0;
int MenuItem=0;
bool salir=false;
int lastMenuItem=5;

//Programa
const int pass_size=10;
char pass[pass_size]={'0','1','2','3','4','5','6','7','8','9'};
char contrasenia[pass_size]={'0','0','0','0','0','0','0','0','0','0'};

int UTempHigh=25;
int UTempLow=18;
int ULuz=500;

//Async
float read_temperature();
long read_photoresistor();
void imprimirLecturas();

AsyncTask asyncTaskTemp(1000,true,read_temperature);
AsyncTask asyncTaskPhoto(1000,true,read_photoresistor);
AsyncTask asyncImprimir(1000,true,imprimirLecturas);


//Emprom
const int addr_UTempHigh=0;
const int addr_UTempLow=1;
const int addr_ULuz=2;
const int centenas_ULuz=3;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  pinMode(rojo,OUTPUT);
  pinMode(verde,OUTPUT);
  pinMode(azul,OUTPUT);
  pinMode(buzzerPin,OUTPUT);
  asyncTaskTemp.Start();
  asyncTaskPhoto.Start();
  UTempHigh=EEPROM.read(addr_UTempHigh);
  UTempLow=EEPROM.read(addr_UTempLow);
  ULuz=EEPROM.read(addr_ULuz)+EEPROM.read(centenas_ULuz)*100;
  
}

void loop() {
 
  // put your main code here, to run repeatedly:
  apagar();
  verificarPass();
  salir=false;
}


char leerCaracter()
{
  char key;
  do
  {
    key = keypad.getKey();
  }
  while(key==0);
  if(key)
  {
    lcd.setCursor(pos++,1);
    lcd.print(key);
  }
  if(pos>=pass_size)
    pos=0;
  return key;
}
char leerCaracterSinImprimir()
{
  char key;
  do
  {
    key = keypad.getKey();
  }
  while(key==0);
  return key;
}
char leerCaracterImprimir()
{
  char key;
  do
  {
    key = keypad.getKey();
  }
  while(key==0);
  if(key)
  {
    lcd.print(key);
  }
  return key;
}




//Contraseña
void leerPass()
{
  for(int i=0;i<pass_size;i++)
  {
    contrasenia[i]=leerCaracter();
  }
}
boolean verificarCaracteres()
{
  for(int i=0;i<pass_size;i++)
  {
    if(contrasenia[i]!=pass[i])
      return false;
  }
  return true;
}
void verificarPass()
{
  boolean bandera=false;
  for(int i=0;i<3;i++)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Ingrese la pass");
    leerPass();
    if(verificarCaracteres())
    {
      passCheck();
      bandera=true;
      break;
    }
    else
      passWrong();
  }
  if (!bandera)
    sistemaBloqueado();
}
void passCheck()
{
    encenderVerde();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Pass correcta :)");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("welcome");
    delay(1000);
    apagar();
    menu();
}
void passWrong()
{
    encenderAzul();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Pass incorrecta");
    delay(2000);
    apagar();
}
void sistemaBloqueado()
{
  encenderRojo();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Sistema bloqueado");
  delay(10000);
  apagar();//Tiempo muerto 10 segundos
  //Secuencia de leds
}




//Menu
void menu()
{
  do
  {
    imprimirFrame();
    char key=leerCaracterSinImprimir();
    switch(key)
    {
      case 'A':
        if(frame>0)
          frame--;
        if(MenuItem>0)
          MenuItem--;
        Serial.print("Menu Item: ");
        Serial.println(MenuItem);
        break;
      case 'B':
        if(frame<lastMenuItem)
            frame++;
        if(MenuItem<lastMenuItem)
          MenuItem++;
        Serial.print("Menu Item: ");
        Serial.println(MenuItem);
        break;
      case '*':
        Serial.println("i'm here");
        opcion();
        break;
    }
  }while(salir==false);
}

void imprimirFrame()
{
  apagar();
  lcd.clear();
  lcd.setCursor(0,0);
  if(MenuItem%2==0)
  {
    lcd.print(frames[frame]);
    lcd.setCursor(0,1);
    lcd.print(frames[frame+1]);
    lcd.setCursor(15,0);
    lcd.print("<");
  }
  else
  {
    lcd.print(frames[frame-1]);
    lcd.setCursor(0,1);
    lcd.print(frames[frame]);
    lcd.setCursor(15,1);
    lcd.print("<");
  }
}

void opcion()
{
  Serial.println("Entre a opcion");
  Serial.print("Menu Item: ");
  Serial.println(MenuItem);
  lcd.clear();
  switch(MenuItem)
  {
    case 0:
      UTempHigh=opcion("UTempHigh", UTempLow+3, 60, UTempHigh);
      Serial.print("Entro al: ");
      Serial.println(MenuItem);
      break;
    case 1:
      UTempLow=opcion("UTempLow", 0, UTempHigh, UTempLow);
      Serial.print("Entro al: ");
      Serial.println(MenuItem);
      break;
    case 2:
      ULuz=opcion("ULuz", 0, 999, ULuz);
      Serial.print("Entro al: ");
      Serial.println(MenuItem);
      break;
    case 3:
      Serial.print("Entro al: ");
      Serial.println(MenuItem);
      lcd.clear();
      char key = 0;
      int contador=0;
      asyncImprimir.Start();
      while(key==0)
      {
        asyncImprimir.Update();
        key = keypad.getKey();
      }
      asyncImprimir.Stop();
      break;
    default:
      Serial.print("default");
      break;
  }
  if(MenuItem==4)
  {
    Serial.print("Entro al: ");
    Serial.println(MenuItem);
    guardarSalir();
  }
  if(MenuItem==5)
  {
      Serial.print("Entro al: ");
      Serial.println(MenuItem);
      UTempHigh=25;
      UTempLow=18;
      ULuz=500;
      lcd.print("values reset");
      delay(1500);
  }
}
int opcion(char nombre[], int rango_low, int rango_high, int variable)
{
    lcd.setCursor(0,0);
    lcd.print(nombre);
    lcd.setCursor(12,0);
    lcd.print(variable);
    lcd.setCursor(0,1);
    lcd.print("[");
    lcd.print(rango_low);
    lcd.print(" - ");
    lcd.print(rango_high);
    lcd.print("]C: ");

    int value=armarEntero();
    if (comprobarRango(rango_low,rango_high,value))
      return value;
    return variable;
}
void guardarSalir()
{
    if(ULuz>=100)
    {
      EEPROM.write(centenas_ULuz, trunc(ULuz/100));
      EEPROM.write(addr_ULuz, ULuz-trunc(ULuz/100)*100);
    }
    else
    {
      EEPROM.write(addr_UTempHigh, UTempHigh);
      EEPROM.write(addr_UTempLow, UTempLow);
      EEPROM.write(addr_ULuz, ULuz);
      EEPROM.write(centenas_ULuz, 0);
    }
    lcd.setCursor(0,0);
    lcd.print("values save");
    delay(1500);
    lcd.clear();
    lcd.print("Good bye :)");
    delay(1500);
    lcd.clear();
    salir=true;
}

boolean comprobarRango(int low, int high, int value)
{
  lcd.clear();
  lcd.setCursor(0,0);
  if(value<=high && value>=low)
  {
    lcd.print("Value ");
    lcd.print(value);
    lcd.print(" save");
    delay(1500);
    return true;
  }
  lcd.print("Valor no valido");
  delay(1500);
  return false;
}

int armarEntero()
{
  int numero=-100;
  int orden=0;
  char key;
  int contador=0;
  while(contador<3)
  {
    key=leerCaracterImprimir();
    if(key=='*')
      break;
    if(contador==0)
      numero=0;
    numero=numero*10;
    numero=numero+(key-'0');
    contador++;
  }
  
  return numero;
}


//Lecturas
float read_temperature()
{
  //read thermistor value
  long a =1023.0 - analogRead(analogPin);
  //the calculating formula of temperature
  float tempC = beta /(log((1025.0 * 10.0 / a - 10.0) / 10.0) + beta / 298.0) - 273.0;
  Serial.print(millis()/1000);
  Serial.print(",read_temp,");
  Serial.println(tempC);
  if(tempC<UTempLow)
    encenderAzul();
  else if(tempC>UTempHigh)
    encenderRojo();
  else
    encenderVerde();
  return tempC;
}
void imprimirLecturas()
{
  asyncTaskPhoto.Update();
  asyncTaskTemp.Update();
  float tempC=read_temperature();
  long photo_value=read_photoresistor();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(tempC);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Photocell:");
  lcd.setCursor(11, 1);
  lcd.print(photo_value);
}
long read_photoresistor()
{
  long photo_value = analogRead(photocellPin);
  if(photo_value>=ULuz)
    pitar();
  Serial.print(millis()/1000);
  Serial.print(",read_photo,");
  Serial.println(photo_value);
  return photo_value;
}




//Leds
void encenderRojo()
{
  analogWrite(rojo,255);
  analogWrite(verde,0);
  analogWrite(azul,0);
}

void encenderVerde()
{
  analogWrite(rojo,0);
  analogWrite(verde,255);
  analogWrite(azul,0);
}

void encenderAzul()
{
  analogWrite(rojo,0);
  analogWrite(verde,0);
  analogWrite(azul,255);
}
void apagar()
{
  analogWrite(rojo,0);
  analogWrite(verde,0);
  analogWrite(azul,0);
}


//Buzzer
void pitar()
{
  for(int i =0;i <= 8;i++) //frequence loop from 200 to 800
  {
    tone(buzzerPin,500,100); //turn the buzzer on
  }
}
