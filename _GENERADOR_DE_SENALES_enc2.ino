///////////////////////////////////////////////////////////
////////// GENERADOR DE SEÑALES AD9833 CON ArduinoOS
////////// OLED I2C 4 PINES + BOTONES +SERIAL + ANALOGO
////////// NESTOR PALOMINOS 2019
//////////
//////////FORMA DE USO: [forma de onda S,C,T][frecuencia]
//////////              S2600
//////////
//////////BOTONES DE CONTROL:DEL/ENTER,
//////////PROCESADO CON AMBOS BOTONES A LA VEZ
//////////////////////////////////////////////////////////

#include "U8glib.h"
#include <stdio.h>
#include <AD9833.h>        
#include <SPI.h> 

///////////////////////////////////////////////////////////
#define FNC_PIN       10 
#define LED_PIN       13 
#define PERIOD (40)
/////////////////////////////////////////////////////////////

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);

//VARIABLES BUFFER SERIAL
String   string;
char     serie[20];
int      cont=0;

//VARIABLES VALOR ANALOGO
int      ana_0=0; 
int      val=0;
char     valor[6];

//VARIABLE LISTA DE OPCIONES
String   label[5];

int      opt_val[5];
int      opt_stat=0;
int      mul_stat=0;

int      opt_max=2;
int      mul_max=3;

int      enc_stat=0;

int mult=10;

String   f="S";

//VARIABLES BOTONES
int b1_pin=2; int b1_state1=0; int b1_state2=0; //FORMA
int b2_pin=3; int b2_state1=0; int b2_state2=0; //MULTIPLICADOR
int b3_pin=4; int b3_state1=0; int b3_state2=0; //ESC
int b4_pin=5; int b4_state1=0; int b4_state2=0; //DOWN

int clk_pin=6; int clk_state1=0; int clk_state2=0; //ENCODER
int dta_pin=7; int dta_state1=0; int dta_state2=0; //ENCODER

String botones="";

//VARIABLES MORSE
//        0123456789012345678901234567890123456
//char c[]="abcdefghijklmnopqrstuvwxyz0123456789 ";
char c[]="SCT0123456789 ";
int buzzer=12;

//GENERADOR DE FUNCIONES
AD9833 gen(FNC_PIN);
WaveformType waveType;

//LINEAS Y CARACTERES
String   linea[5];
int      k=0;
int      ncar=13; //36;
String   txt;
String   txt2;
int flag_enter=0;

/////////////////////////////////////////////
////////////// SETUP , LOOP
/////////////////////////////////////////////

void setup(void) {
  
  Serial.begin(9600);
  
  gen.Begin();
  gen.EnableOutput(true);
  
  ConfigOLED();
  limpiar();
  //ini_labels();
  Ini();
  
  pinMode(b1_pin, INPUT);
  pinMode(b2_pin, INPUT);
  pinMode(b3_pin, INPUT);
  pinMode(b4_pin, INPUT);

  pinMode(clk_pin, INPUT);
  pinMode(dta_pin, INPUT);

  clk_state2 = digitalRead(clk_pin);
}


void loop(void) {

  ProcesarEncoder();
  //ProcesarSerie();
  //LeerAnalogo();
  ini_labels();
  ProcesarBoton();
  Display();
  
}

/////////////////////////////////////////////
////////////// FUNCIONES
/////////////////////////////////////////////
void Display() {
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_fixed_v0);
    
    linea[0]="SGN_GEN NP2019";
    linea[1]=">"+txt;
    linea[2]=" "+txt2;
    linea[3]="--------";
    linea[4]="--------";
    
    for(int i=0;i<5;i++){
      //itoa(linea[i],valor,10);
      linea[i].toCharArray(valor,10);
      u8g.drawStr( 1, 15+i*10, valor);
    }
    
    
  } while( u8g.nextPage() );
}


void ProcesarEncoder(){

  clk_state1 = digitalRead(clk_pin);
  if(clk_state1!=clk_state2){
    if(digitalRead(dta_pin)!=clk_state1){
      enc_stat++;
    }
    else{    
      enc_stat--;
    }
    //Serial.println(enc_stat);
    //Display();
  }  
  clk_state2=clk_state1;
}

void ProcesarSerie(){
 if( Serial.available()){
    char inChar =Serial.read();
    Serial.print(inChar);
    string+=inChar;
    serie[cont]=inChar;
    cont++; 
    if( inChar == '\r'){
       string="";
       cont=0;      
    }
  }
}

void limpiar(){
 for(int i=0;i<5;i++){
     opt_val[i]=0;
 } 
}

void LeerAnalogo(){
  ana_0=analogRead(A0); 
  //val = map(ana_0,0,1023,0,1000);
  itoa(ana_0,valor,4);
  //delay(50);
  //Serial.println(ana_0);
}

void ini_labels(){
  label[0]="form ";
  label[1]="mult ";
  label[2]="frec ";
}


/////////////////////////////////////////////Generador de funciones

void Procesar(String txt){

 String tipo=txt.substring(0,1);

 if(tipo=="S" || tipo=="C" || tipo=="T"){
   txt2=txt.substring(1,txt.length());
   long frec=txt2.toInt();
   Serial.println(frec);
   GenerarSenal(tipo,frec);
 }

}

void GenerarSenal(String tipo,long frec){
 
  //gen.Reset();
  
  if(tipo=="S"){
    waveType = SINE_WAVE;
  }
  
  if(tipo=="C"){
    waveType = SQUARE_WAVE;
  }

  if(tipo=="T"){
    waveType = TRIANGLE_WAVE;
  }  
  
  //long frec=enc_stat*mult;
  
  gen.SetFrequency(REG0,frec);    gen.SetFrequency(REG1,frec);
  gen.SetWaveform(REG0,waveType); gen.SetWaveform(REG1,waveType);
  gen.SetOutputSource(REG0);      gen.SetOutputSource(REG1); 
}

void IncrementFrequencyTest ( void ) {

    float startHz = 1000, stopHz = 5000, incHz = 1, sweepTimeSec = 5.0;
 
    // Calculate the delay between each increment.
    uint16_t numMsecPerStep = (sweepTimeSec * 1000.0) / ((uint16_t)((stopHz - startHz) / incHz) + 1);
    if ( numMsecPerStep == 0 ) numMsecPerStep = 1;

    // Apply a signal to the output. If phaseReg is not supplied, then
    // a phase of 0.0 is applied to the same register as freqReg
    gen.ApplySignal(SINE_WAVE,REG1,startHz);

    while ( true ) {
      
        gen.SetFrequency(REG1,startHz-incHz);

        for ( float i = startHz ; i <= stopHz; i += incHz ) {
         
            gen.IncrementFrequency(REG1,incHz);
            delay(numMsecPerStep); 
        }
    }
}

////////////////////Procesamiento

void ProcesarBoton(){

  b1_state1 = digitalRead(b1_pin);
  b2_state1 = digitalRead(b2_pin);
  b3_state1 = digitalRead(b3_pin);
  b4_state1 = digitalRead(b4_pin);
  
  //AMBOS BOTONES DE CONTROL PRESIONADOS EJECUTAN LA ACCION
  if(b3_state1==1 && b4_state1==1 && b3_state2==0 && b4_state2==0){
    Procesar(txt);
    tone(buzzer, 6000,20);
  }
  
  //boton AZ
  if(b1_state1==0 && b1_state2==1){
    
    
    if(flag_enter==1){
      txt=txt.substring(0,txt.length()-1)+c[k];
      //Codificar(c[k]);
    }
    if(flag_enter==0){
     txt=txt+c[k];
     //Codificar(c[k]);
     flag_enter=1;
    }
    k++;
    if(k>ncar){
      k=0;
    }
    
  }
  
  //boton ZA
  if(b2_state1==0 && b2_state2==1){
    
    if(flag_enter==1){
      txt=txt.substring(0,txt.length()-1)+c[k];
      //Codificar(c[k]);
    }
    if(flag_enter==0){
     txt=txt+c[k];
     //Codificar(c[k]);
     flag_enter=1;
    }
    k--;
    if(k<0){
      k=ncar;
    }
  }  
  
  //boton enter
  if(b3_state1==0 && b3_state2==1){
    txt=txt+c[k];
    Enviar(txt,txt.length());
  }  
  
  //boton del
  if(b4_state1==0 && b4_state2==1){
    txt=txt.substring(0,txt.length()-1);
    //Enviar(txt,10);
  }  
  
  //botones=String(b1_state1)+String(b2_state1)+String(b3_state1)+String(b4_state1);
  
  b1_state2=b1_state1;
  b2_state2=b2_state1;
  b3_state2=b3_state1;
  b4_state2=b4_state1;
  
}

//           (COLUMNA) (FILA)
void Mostrar(int posx, int posy, char* txt) {
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_fixed_v0);
    u8g.drawStr( posx, posy, txt);
  } while( u8g.nextPage() );
}
void ConfigOLED(){
  u8g.setColorIndex(100);
}

//////////////////////////////////////////////MORSE


void Alta(){
 digitalWrite(buzzer, HIGH);
 tone(buzzer, 2600);
}

void Baja(){
 digitalWrite(buzzer, LOW); 
 noTone(buzzer);
}

void writeMorse(char *code) {
  int i = 0;
  while (true) {
    char currentChar = code[i];
    if (currentChar == '\0') {
      break;
    } 
    else if (currentChar == '.') {
      Alta();
      delay(PERIOD);
      Baja();
      delay(PERIOD);
    } 
    else if (currentChar == '-') {
      Alta();
      delay(PERIOD * 2);
      Baja();
      delay(PERIOD);
    } 
    else if (currentChar == ' ') {
      Baja();
      delay(PERIOD * 4);
    } 
    else if (currentChar == '/') {
      Baja();
      delay(PERIOD * 7);
    }
    i += 1;
  }
}

void Ini(){ //inicio
  for(int i =0;i<3;i++){
   tone(buzzer, 3500,100);
   delay(50);
   noTone(buzzer);
   delay(50);
  }
}


void Enviar(String txt,int n){
  char tmp[n];
  txt.toCharArray(tmp,n);

  //MORSE
  //for(int i=0;i<n;i++){ 
  //  Codificar (valor[i]);
  //}
  
  tone(buzzer, 4000,50);
}

void serialEvent () {
  char c = Serial.read();
  Codificar(c);
}

void Codificar (char c) {
  delay(500);
  switch (c) {
    case 'A':
    case 'a':
      writeMorse(".- "); break;
    case 'B':
    case 'b':
      writeMorse("-... "); break;
    case 'C':
    case 'c':
      writeMorse("-.-. "); break;
    case 'D':
    case 'd':
      writeMorse("-.. "); break;
    case 'E':
    case 'e':
      writeMorse(". "); break;
    case 'F':
    case 'f':
      writeMorse("..-. "); break;
    case 'G':
    case 'g':
      writeMorse("--. "); break;
    case 'H':
    case 'h':
      writeMorse(".... "); break;
    case 'I':
    case 'i':
      writeMorse(".. "); break;
    case 'J':
    case 'j':
      writeMorse(".--- "); break;
    case 'K':
    case 'k':
      writeMorse("-.- "); break;
    case 'L':
    case 'l':
      writeMorse(".-.. "); break;
    case 'M':
    case 'm':
      writeMorse("-- "); break;
    case 'N':
    case 'n':
      writeMorse("-. "); break;
    case 'O':
    case 'o':
      writeMorse("--- "); break;
    case 'P':
    case 'p':
      writeMorse(".--. "); break;
    case 'Q':
    case 'q':
      writeMorse("--.- "); break;
    case 'R':
    case 'r':
      writeMorse(".-. "); break;
    case 'S':
    case 's':
      writeMorse("... "); break;
    case 'T':
    case 't':
      writeMorse("- "); break;
    case 'U':
    case 'u':
      writeMorse("..- "); break;
    case 'V':
    case 'v':
      writeMorse("...- "); break;
    case 'W':
    case 'w':
      writeMorse(".-- "); break;
    case 'X':
    case 'x':
      writeMorse("-..- "); break;
    case 'Y':
    case 'y':
      writeMorse("-.-- "); break;
    case 'Z':
    case 'z':
      writeMorse("--.. "); break;
    case ' ':
      writeMorse("/"); break;
    case '1':
      writeMorse(".---- "); break;
    case '2':
      writeMorse("..--- "); break;
    case '3':
      writeMorse("...-- "); break;
    case '4':
      writeMorse("....- "); break;
    case '5':
      writeMorse("..... "); break;
    case '6':
      writeMorse("-.... "); break;
    case '7':
      writeMorse("--... "); break;
    case '8':
      writeMorse("---.. "); break;
    case '9':
      writeMorse("----. "); break;
    case '0':
      writeMorse("----- "); break;
    default:
      break;
  }
}
