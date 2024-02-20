#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
#endif

#include <ESP_Mail_Client.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define SMTP_HOST ""
#define SMTP_PORT esp_mail_smtp_port_587 

#define AUTHOR_EMAIL ""
#define AUTHOR_PASSWORD ""

#define RECIPIENT_EMAIL_1 ""
#define RECIPIENT_EMAIL_2 ""
#define RECIPIENT_EMAIL_3 ""

#define HWT_STATUS_PIN 0
#define WIFI_STATUS_PIN 4
#define RS_STATUS_PIN 15
#define LWF_STATUS_PIN 16

#define RS_INPUT_PIN 14
#define HWT_INPUT_PIN 12
#define LWF_INPUT_PIN 13


SMTPSession smtp;
void smtpCallback(SMTP_Status status);
int sent_status = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println();
  pinMode(RS_STATUS_PIN, OUTPUT);
  pinMode(HWT_STATUS_PIN , OUTPUT);
  pinMode(LWF_STATUS_PIN, OUTPUT);
  pinMode(WIFI_STATUS_PIN, OUTPUT);
  digitalWrite(RS_STATUS_PIN, LOW);
  digitalWrite(HWT_STATUS_PIN , LOW);
  digitalWrite(LWF_STATUS_PIN, LOW);
  digitalWrite(WIFI_STATUS_PIN, LOW);
  pinMode(RS_INPUT_PIN , INPUT);
  pinMode(HWT_INPUT_PIN, INPUT);
  pinMode(LWF_INPUT_PIN, INPUT);
  String textMsg = "Message: ";

  delay(1000);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    digitalWrite(WIFI_STATUS_PIN, HIGH);
    delay(600);
    digitalWrite(WIFI_STATUS_PIN, LOW);
    delay(600);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  digitalWrite(WIFI_STATUS_PIN, HIGH);
  Serial.println();

  while (sent_status == 0){
    delay(5000);
    if (WiFi.status() == WL_CONNECTED){
      digitalWrite(WIFI_STATUS_PIN, HIGH);
    } else {
      digitalWrite(WIFI_STATUS_PIN, LOW);
    }
    
    if (digitalRead(RS_INPUT_PIN) == LOW){
      sent_status = 1;
      digitalWrite(RS_STATUS_PIN  , HIGH);
      //String textMsg = "\n";
      textMsg += "The compressor stopped running.\n";
      //if (!MailClient.sendMail(&smtp, &message)){
      //  MailClient.printf("Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
      //}
      Serial.println("Pin 14 is High");
    }
    if (digitalRead(HWT_INPUT_PIN) == HIGH){
      sent_status = 1;
      digitalWrite(HWT_STATUS_PIN , HIGH);
      //String textMsg = "Message 2\n";
      textMsg += "High Water Temperature Detected.\n";
      //if (!MailClient.sendMail(&smtp, &message)){
      //  MailClient.printf("Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
      //}
      Serial.println("Pin 12 is High");
    }
    if (digitalRead(LWF_INPUT_PIN) == HIGH){
      sent_status = 1;
      digitalWrite(LWF_STATUS_PIN, HIGH);
      //String textMsg = "Message 3\n";
      textMsg += "Low Water Flow Detected.\n";
      //if (!MailClient.sendMail(&smtp, &message)){
      //  MailClient.printf("Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
      //}
      Serial.println("Pin 14 is High");
    }
  }
  MailClient.networkReconnect(true);
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the Session_Config for user defined session credentials */
  Session_Config config;

  /* Set the session config */
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = F("");

  // config.secure.mode = esp_mail_secure_mode_nonsecure;

  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = -5;
  config.time.day_light_offset = 0;

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = F("Teslatron Compressor");
  message.sender.email = AUTHOR_EMAIL;

  String subject = F("Alarm");
  message.subject = subject;

  message.addRecipient(F(""), RECIPIENT_EMAIL_1);
  message.addRecipient(F(""), RECIPIENT_EMAIL_2);
  message.addRecipient(F(""), RECIPIENT_EMAIL_3);

  message.text.content = textMsg;
  message.text.transfer_encoding = "base64"; // recommend for non-ASCII words in message.
  message.text.charSet = F("utf-8"); // recommend for non-ASCII words in message.
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  

  /* Connect to the server */
  if (!smtp.connect(&config))
  {
    MailClient.printf("Connection error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn())
  {
    Serial.println("Not yet logged in.");
  }
  else
  {
    if (smtp.isAuthenticated())
      Serial.println("Successfully logged in.");
    else
      Serial.println("Connected with no Auth.");
  }
  int sent_status = 0;
  delay(500);

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message)){
    MailClient.printf("Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  }
  
  // to clear sending result log
  // smtp.sendingResult.clear();

}

void loop()
{
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    // MailClient.printf used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    MailClient.printf("Message sent success: %d\n", status.completedCount());
    MailClient.printf("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)

      MailClient.printf("Message No: %d\n", i + 1);
      MailClient.printf("Status: %s\n", result.completed ? "success" : "failed");
      MailClient.printf("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      MailClient.printf("Recipient: %s\n", result.recipients.c_str());
      MailClient.printf("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}
