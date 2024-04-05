#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <RBDdimmer.h>
#include <Wire.h>
#include <Arduino.h>
#include "ACS712.h"
#include <PZEM004Tv30.h>

#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif
#if !defined(PZEM_SERIAL)
#define PZEM_SERIAL Serial2
#endif
#if defined(ESP32)
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
#elif defined(ESP8266)
#else
PZEM004Tv30 pzem(PZEM_SERIAL);
#endif

ACS712 arus_masukpin(ACS712_30A, 36);
#define tegangan_masukpin 39
ACS712 arus_bateraipin(ACS712_30A, 34);
#define tegangan_bateraipin 35
ACS712 arus_keluarpin(ACS712_30A, 32);
#define tegangan_keluarpin 33


// arus pembangkit listrik
float arus_masuk;

// tegangan pembangkit listrik
float tegangan_masuk;
float R1 = 30000.0;
float R2 = 7500.0;
float ref_voltage = 3.3;

// Perhitungan Daya Masuk
float daya_masuk;

// baterai
float arus_baterai;
float tegangan_baterai;
float daya_baterai;
float presentase_baterai;

// arus keluar
float arus_keluar;
float tegangan_keluar;
float daya_keluar;

// AC AC
    float arus_ac=0;
    float tegangan_ac=0;
    float daya_aktif=0;
    float daya_reaktif=0;
    float daya_semu=0;
    float faktor_daya=0;

//Output
int relay1=13;
int relay2=22;
int relay3=21;
int relay4=19;
int relay5=18;
int relay6=4;

const char* ssid = "Kuman";
const char* password = "senyumdulu";
String token = "dnp3yRZiwt69CYx5";
String HWID = "9Vq8boHL";
String email = "achmadzildan15@gmail.com";
String linkGET = "https://iot.mansidoarjo.sch.id/rest/bacajason/" + HWID;
String kirim_server = "https://iot.mansidoarjo.sch.id/rest/kirimdatasensor";


void setup() {
  Serial.begin(115200);
  pzem.resetEnergy();
  int zero1 = arus_masukpin.calibrate(); 
  pinMode (tegangan_masukpin,INPUT);
  int zero2 = arus_bateraipin.calibrate(); 
  pinMode (tegangan_bateraipin,INPUT);
  int zero3 = arus_keluarpin.calibrate(); 
  pinMode (tegangan_keluarpin,INPUT);
  pinMode (relay1,OUTPUT);
  pinMode (relay2,OUTPUT);
  pinMode (relay3,OUTPUT);
  pinMode (relay4,OUTPUT);
  pinMode (relay5,OUTPUT);
  pinMode (relay6,OUTPUT);

  Serial.println("Zero point for this ACS1 = " + zero1);
  Serial.println("Zero point for this ACS2 = " + zero2);
  Serial.println("Zero point for this ACS3 = " + zero3);


  // Hubungkan ke WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Gagal terhubung ke WiFi. Mengulangi...");
  }
  Serial.println("Terhubung ke WiFi!");

}

void loop() {
   baca_sensor();
   baca_jason();
   //delay(1000);
}

void baca_sensor(){   
  // Pembangkit Listrik
  // Arus Masuk
  arus_masuk = ((arus_masukpin.getCurrentDC())*1000);
      if (arus_masuk<0){arus_masuk = arus_masuk*-1;}  
      //Serial.print(arus_masuk);
      //Serial.println("mA");
      
   // Tegangan Masuk
   tegangan_masuk=analogRead(tegangan_masukpin);
   tegangan_masuk=(tegangan_masuk*ref_voltage)/4095.0;
   tegangan_masuk = (tegangan_masuk / (R2/(R1+R2)));
   //Serial.print(tegangan_masuk);
   //Serial.println("V");
   // Perhitungan Daya Masuk
   daya_masuk=tegangan_masuk*(arus_masuk/1000); //P=VxI
   //Serial.print(daya_masuk);
   //Serial.println("Watt");

   // Charge Controller
  // Arus Baterai
  arus_baterai = ((arus_bateraipin.getCurrentDC())*1000);
      if (arus_baterai<0){arus_baterai = arus_baterai*-1;}  
      //Serial.print(arus_baterai);
      //Serial.println("mA");

   // Tegangan Baterai
   tegangan_baterai=analogRead(tegangan_bateraipin);
   tegangan_baterai=(tegangan_baterai*ref_voltage)/4095.0;
   tegangan_baterai = (tegangan_baterai / (R2/(R1+R2)));
   //Serial.print(tegangan_baterai);
   //Serial.println("V");
   // Perhitungan Daya Baterai
   daya_baterai=tegangan_baterai*(arus_baterai/1000); //P=VxI
   //Serial.print(daya_baterai);
   //Serial.println("Watt");
   // Perhitungan Presentase Daya Baterai
   presentase_baterai= ((tegangan_baterai - 10.7)/(14.4-10.7))*100; // ((Tegangan saat ini−Tegangan minimum)/(Tegangan maksimum−Tegangan minimum))*100%
   if (presentase_baterai<0){presentase_baterai=presentase_baterai*-1;}
   //Serial.print(presentase_baterai);
   //Serial.println("%");

    // Arus Keluar
  arus_keluar = ((arus_keluarpin.getCurrentDC())*1000);
      if (arus_keluar<0){arus_keluar = arus_keluar*-1;}  
      //Serial.print(arus_keluar);
      //Serial.println("mA");

   // Tegangan Keluar
   tegangan_keluar=analogRead(tegangan_keluarpin);
   tegangan_keluar=(tegangan_keluar*ref_voltage)/4095.0;
   tegangan_keluar = (tegangan_keluar / (R2/(R1+R2)));
   //Serial.print(tegangan_keluar);
   //Serial.println("V");
   // Perhitungan Daya Keluar
   daya_keluar=tegangan_keluar*(arus_keluar/1000); //P=VxI
   //Serial.print(daya_keluar);
   //Serial.println("Watt");

   // AC AC
    float voltage = pzem.voltage();
    float current = pzem.current()-0.03;
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();
        // Print the values to the Serial console
        /*Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
        Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
        Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
        Serial.print("Energy: ");       Serial.print(energy,3);     Serial.println("kWh");
        Serial.print("Frequency: ");    Serial.print(frequency, 1); Serial.println("Hz");*/
        if (isnan(voltage)){
                  //Serial.println("Error reading voltage");
                      voltage = 0;
                      current = 0;
                      power = 0;
                      energy = 0;
                      frequency = 0;
                      pf = 0;
                  }
              // Arus AC
              arus_ac = current *1000;
              //Serial.print(arus_ac);
              //Serial.println("mA");
              // Tegangan AC
              tegangan_ac=voltage;
              //Serial.print(tegangan_ac);
              //Serial.println("V");
              // Daya Aktif
              daya_aktif=power;
              //Serial.print(daya_aktif);
              //Serial.println("Watt");
              // Daya Semu
              daya_semu=voltage*current;
              //Serial.print(daya_semu);
              //Serial.println("VA");
              // Daya Reaktif
              daya_reaktif=sqrt(pow(daya_semu, 2) - pow(daya_aktif, 2));
              //Serial.print(daya_reaktif);
              //Serial.println("VAR");
              // Faktor Daya
              faktor_daya= pf;
              //Serial.print(faktor_daya);
              //Serial.println("Choss Phi");

   // Kirim data sensor
  if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      String serverPath = kirim_server + "?&&token=" + token + "&&HWID=" + HWID + "&&arus_masuk=" + arus_masuk + "&&tegangan_masuk=" + tegangan_masuk + "&&daya_masuk=" + daya_masuk +"&&arus_baterai=" + arus_baterai +"&&tegangan_baterai=" + tegangan_baterai +"&&daya_baterai=" + daya_baterai +"&&presentase_baterai=" + presentase_baterai +"&&arus_keluar=" + arus_keluar +"&&tegnagan_keluar=" + tegangan_keluar +"&&daya_keluar=" + daya_keluar +"&&arus_ac=" + arus_ac +"&&tengangan_ac=" + tegangan_ac +"&&daya_aktif=" + daya_aktif +"&&daya_reaktif=" + daya_reaktif +"&&daya_semu=" + daya_semu +"&&faktor_daya=" + faktor_daya +"&&email=" + email;
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      if (httpResponseCode>0) {
        //Serial.print("HTTP Response code: ");
        //Serial.println(httpResponseCode);
        String payload = http.getString();
        //Serial.println(payload);
      }
      else {
        //Serial.print("Error code: ");
        //Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
       // else {Serial.println("WiFi Disconnected");}

  }


void baca_jason(){
// Mengambil Data Jason
  // Buat koneksi HTTP
  HTTPClient http;
  http.begin(linkGET);

  // Lakukan permintaan GET
  int httpCode = http.GET();

  // Jika permintaan berhasil
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    //Serial.println(payload);

    // Parse data JSON
    DynamicJsonDocument doc(4096);
    deserializeJson(doc, payload);
    JsonObject data = doc["Data"][0];

    // Ambil data yang diperlukan
    String sumber_listrik = data["sumber_listrik"];
    String faktor_daya2 = data["faktor_daya"];
    String output_dc = data["output_dc"];
    String stopkontak_1 = data["stopkontak_1"];
    String stopkontak_2 = data["stopkontak_2"];
    String lampu = data["lampu"];
    
//sumber_listrik
if (sumber_listrik == "1"){
  digitalWrite(relay1, LOW);
  //Serial.println("ON");
  }
else {
  digitalWrite(relay1, HIGH);
  //Serial.println("OFF");
  }
  
//faktor_daya
if (faktor_daya2 == "1" && faktor_daya >= 0.9 ){
  digitalWrite(relay2, LOW);
  //Serial.println("ON");
  }
else {
  digitalWrite(relay2, HIGH);
  //Serial.println("OFF");
  }


//output_dc
if (output_dc == "1"){
  digitalWrite(relay3, LOW);
  //Serial.println("ON");
  }
else {
  digitalWrite(relay3, HIGH);
  //Serial.println("OFF");
  }


  //stopkontak_1
if (stopkontak_1 == "1"){
  digitalWrite(relay4, LOW);
  //Serial.println("ON");
  }
else {
  digitalWrite(relay4, HIGH);
  //Serial.println("OFF");
  }

  //stopkontak_2
if (stopkontak_2 == "1"){
  digitalWrite(relay5, LOW);
  //Serial.println("ON");
  }
else {
  digitalWrite(relay5, HIGH);
  //Serial.println("OFF");
  }

    //lampu
if (lampu == "1"){
  digitalWrite(relay6, LOW);
  //Serial.println("ON");
  }
else {
  digitalWrite(relay6, HIGH);
  //Serial.println("OFF");
  }

  
  } else {
    Serial.println("Gagal melakukan permintaan HTTP");
  }
  http.end();  // Putuskan koneksi HTTP
  }
  
