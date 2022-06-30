

#define BLYNK_TEMPLATE_ID           "TMPLSAHCk0IH"
#define BLYNK_DEVICE_NAME           "IOTPETFEEDER"
#define BLYNK_AUTH_TOKEN            "d5zvgCnpgcNgnem1uc9BjO8dLvVVSk1p"


//#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Servo.h>
#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"


#define DELAY_BEFORE_MUSIC (30*60*1000)  // 30 mins * 60 sec * 1000 milli, chaque 30 mins pour appeler/Scanner notre chien
#define DELAY_FOR_CALLING_PET (5*60*1000) // 5 mins * 60 seconds * 1000 milli,  5 mins pour lire on loop l'appelle
#define DELAY_FOR_CHECKING_ON_PET (15*60*1000) // 15 mins si rien ete detecte, notification/email sera envoye apres 15 mins

#define SERVO_SWING_DELAY 2000

#define PIR_SENSOR 23
#define PIR_NOT_DETECTED LOW
#define PIR_DETECTED HIGH

static const int servoPin = 26;

Servo feeder_servo;

uint32_t time_passed = 0;

int pet_step = 0;


char auth[] = BLYNK_AUTH_TOKEN;


char ssid[] = "Huawei";
char pass[] = "qwerty0987654321";


HardwareSerial hwSerial(1);
DFRobotDFPlayerMini dfPlayer;
int volume = 10;

BLYNK_CONNECTED()
{
  Blynk.syncAll();
}

BLYNK_WRITE(V0)
{
  volume = param.asInt(); 
  dfPlayer.volume(volume);  

  Serial.print("adjusted volume is: ");
  Serial.println(volume);
}
BLYNK_WRITE(V1)
{
  int servo_switch = param.asInt();
  if (servo_switch)
  {
    Serial.println("Servo activated from Blynk");
    pet_step = 3;
    Blynk.virtualWrite(V1, LOW);
  }
}
BLYNK_WRITE(V2)
{
  int music_on_switch = param.asInt(); 
  if (music_on_switch)
  {
    Serial.println("Music activated from Blynk");
    dfPlayer.volume(volume);  //Set volume value (0~30).
    dfPlayer.play(1);  //Play the mp3 file
    Blynk.virtualWrite(V2, LOW);
  }
}
BLYNK_WRITE(V3)
{
  int music_off_switch = param.asInt(); 
  if (music_off_switch)
  {
    Serial.println("Music deactivated from Blynk");
    dfPlayer.stop();  //Stop the mp3 file
    Blynk.virtualWrite(V3, LOW);
  }
}

void setup()
{
  btStop(); 
  hwSerial.begin(9600, SERIAL_8N1, 16, 17); 
  Serial.begin(115200);

  feeder_servo.attach(servoPin);

  Blynk.begin(auth, ssid, pass);

  dfPlayer.begin(hwSerial);  
  dfPlayer.setTimeOut(500); 
  dfPlayer.volume(volume);  
  dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
  dfPlayer.outputDevice(DFPLAYER_DEVICE_SD);

}

void loop() {
  Blynk.run();

  if (time_passed == 0 && pet_step == 0)
  {
    time_passed = millis() + DELAY_BEFORE_MUSIC;
    Serial.println("1er etape");
  }

  if (time_passed <= millis() && pet_step == 0)
  {
    time_passed = millis() + DELAY_FOR_CALLING_PET; // 5min
    pet_step = 1;
    dfPlayer.play(1);  //Lire le 1er mp3
    Serial.println("terminer");
    Serial.println("2eme etape");
  }

  if (pet_step == 1 && digitalRead(PIR_SENSOR) == PIR_DETECTED)
  {
    feed_the_pet();
    Serial.println("3eme etape");
  }

  if (time_passed <= millis() && pet_step == 1)
  {
    time_passed = millis() + DELAY_FOR_CHECKING_ON_PET; // 15 mins d'attente
    pet_step = 2;
    Serial.println("4eme etape");
  }
  if (time_passed > millis() && pet_step == 2 && digitalRead(PIR_SENSOR) == PIR_DETECTED)
  {
    feed_the_pet();
    Serial.println("5eme etape");
  }

  if (time_passed <= millis() && pet_step == 2)
  {
    time_passed = 0;
    pet_step = 0;
    // Blynk.email("autoiotpetfeeder@gmail.com", "Alert", "there was no movement detected after the sound has been played");
    Blynk.logEvent("chien_pas_detecte", "Il n'y'a pas de mouvement a été detectée");
    Serial.println("6eme etape");
  }
  if (pet_step == 3)
  {
    feed_the_pet();
  }

}
void feed_the_pet()
{


  feeder_servo.write(0);
  delay(SERVO_SWING_DELAY);
  feeder_servo.write(90);
  delay(SERVO_SWING_DELAY);
  feeder_servo.write(0);
  delay(SERVO_SWING_DELAY);
  feeder_servo.write(90);
  delay(SERVO_SWING_DELAY);
  feeder_servo.write(0);
  delay(SERVO_SWING_DELAY);
  feeder_servo.write(90);
  delay(SERVO_SWING_DELAY);
  feeder_servo.write(0);

  time_passed = 0;
  pet_step = 0;
  Serial.println("Nourriture a été servi");
}

String printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      return "Time Out!";
      break;
    case WrongStack:
      return "Stack Wrong!";
      break;
    case DFPlayerCardInserted:
      return "Card Inserted!";
      break;
    case DFPlayerCardRemoved:
      return "Card Removed!";
      break;
    case DFPlayerCardOnline:
      return "Card Online!";
      break;
    case DFPlayerPlayFinished:
      return "Play Finished!";
      break;
    case DFPlayerError:
      switch (value) {
        case Busy:
          return "Error: Card not found";
          break;
        case Sleeping:
          return "Error: Sleeping";
          break;
        case SerialWrongStack:
          return "Error: Get Wrong Stack";
          break;
        case CheckSumNotMatch:
          return "Error: Check Sum Not Match";
          break;
        case FileIndexOut:
          return "Error: File Index Out of Bound";
          break;
        case FileMismatch:
          return "Error: Cannot Find File";
          break;
        case Advertise:
          return "Error: In Advertise";
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}
