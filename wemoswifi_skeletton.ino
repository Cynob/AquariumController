#include <ESP8266WiFi.h>
#ifndef UNIT_TEST
#include <Arduino.h>
#endif

#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

int LEDPin = 2;
// Benutzername und Passwort für Acess Point
String ap_ssid = "Wasserspiele";
String ap_pswd = "Wasserspiele";
    
String ap_nw_name[18]; // Liste gefundener APs
String inittest;
String ap_on;

ESP8266WebServer server(80);


/*
 * 
 * EEPROM Funktionen
 * 
 */
int wordpos_ss[] = {0, 0};
 
int * eeprom_get_pos(String type) {
    
    int startpos_st_paswd = 32;
    int stoppos_st_paswd = 95;
    int startpos_ap_ssid = 96;
    int stoppos_ap_ssid = 127;
    int startpos_ap_paswd = 128;
    int stoppos_ap_paswd = 191;
    int startpos_is_init = 192;
    int stoppos_is_init = 200;
    int startpos_st_ssid = 201;
    int stoppos_st_ssid = 234;
    int startpos_ap_on = 235;
    int stoppos_ap_on = 244;
        
    if ( type == "st_paswd" ) {
        wordpos_ss[0] = startpos_st_paswd;
        wordpos_ss[1] = stoppos_st_paswd;
    }
    if ( type == "st_paswd" ) {
        wordpos_ss[0] = startpos_st_paswd;
        wordpos_ss[1] = stoppos_st_paswd;
    }
    if ( type == "ap_ssid" ) {
        wordpos_ss[0] = startpos_ap_ssid;
        wordpos_ss[1] = stoppos_ap_ssid;
    }
    if ( type == "ap_paswd" ) {
        wordpos_ss[0] = startpos_ap_paswd;
        wordpos_ss[1] = stoppos_ap_paswd;
    }
    if ( type == "is_init" ) {
        wordpos_ss[0] = startpos_is_init;
        wordpos_ss[1] = stoppos_is_init;
    }
    if ( type == "st_ssid" ) {
        wordpos_ss[0] = startpos_st_ssid;
        wordpos_ss[1] = stoppos_st_ssid;
    }
    if ( type == "ap_on" ) {
        wordpos_ss[0] = startpos_ap_on;
        wordpos_ss[1] = stoppos_ap_on;
    }

    return wordpos_ss;
    
}

void clear_eeprom() {
    //Serial.println("clearing eeprom");
    EEPROM.begin(1024);
    for (int i = 0; i < 1024; ++i) { EEPROM.write(i, '\0'); }
    EEPROM.commit();
    EEPROM.end();
}

void write_eeprom(String type, String val) {
    EEPROM.begin(1024);
    int wordpos_start = eeprom_get_pos(type)[0];
    int wordpos_stop = eeprom_get_pos(type)[1];
    int wordpos_len = wordpos_stop - wordpos_start;
    for (int i = 0; i < wordpos_len; ++i) {
        EEPROM.write(wordpos_start+i, '\0');
    }
    for (int i = 0; i < val.length(); ++i) {
        EEPROM.write(wordpos_start+i, val[i]);
    }
    EEPROM.commit();
    EEPROM.end();
}

String read_eeprom(String type) {
    EEPROM.begin(1024);
    String readed;
    int wordpos_start = eeprom_get_pos(type)[0];
    int wordpos_stop = eeprom_get_pos(type)[1];
    for (int i = wordpos_start; i < wordpos_stop; ++i) {
        readed += char(EEPROM.read(i));
    }
    EEPROM.end();
    //Serial.println(readed);
    return readed;
}


/*
 * 
 * Acess Point
 * 
 */

void handleAP() {
    if (server.hasArg("passwd")) {
        handleAPSubmit();
    } else if(server.hasArg("done")) {
        server.send(200, "text/html", "<h1>Zugangsdaten Gespeichert <br> Bitte warten <br>...wird neu gestartet.</h1>");
        write_eeprom("is_init", "True");
        write_eeprom("ap_on", "False");
        delay(1000);
        ESP.restart();
    } else {
        server.send(200, "text/html", AP_INDEX_HTML());
    }
}

void handleAPSubmit() {
    String st_passwd = server.arg("passwd");
    String st_ssid = server.arg("ssid");
    write_eeprom("st_ssid", st_ssid);
    write_eeprom("st_paswd", st_passwd);
    server.send(200, "text/html", AP_DONE_HTML());

}

String AP_INDEX_HTML() {
    String a = "<!DOCTYPE HTML>";
    a += "<html>";
    a += "<head>";
    a += "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">";
    a += "<title>";
    a += ap_ssid;
    a += "</title>";
    a += "<style>";
    a += "\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\"";
    a += "</style>";
    a += "</head>";
    a += "<body>";
    a += "<h1>Verbinde dich mit deinem Heimnetzwerk</h1>";
    a += "<FORM action=\"/\" method=\"post\">";
    a += "<P>";
    a += "<label>SSID:";
    a += "<select name=\"ssid\" size=\"6\">";
    for (int i = 0; i < 17; ++i) { 
        a += "<option>";
        a += ap_nw_name[i];
        a += "</option>";
    }
    a += "</select>";
    a += "</label><br><br>";
    a += "Passwort:<br>";
    a += "<INPUT type=\"text\" name=\"passwd\"<BR>";
    a += "<INPUT type=\"submit\" value=\"Speichern\"</INPUT>";
    a += "</P>";
    a += "</FORM>";
    a += "</body>";
    a += "</html>";
    return a;
}

String AP_DONE_HTML() {
    String a = "<!DOCTYPE HTML>";
    a += "<html>";
    a += "<body>";
    a += "<h1>Zugangsdaten Gespeichert <br> Bitte auf Neustart klicken.</h1>";
    a += "<FORM action=\"/\" method=\"post\">";
    a += "<P>";
    a += "<INPUT type=\"text\" style=\"height: 10px; width: 10px;\" name=\"done\"<BR>";
    a += "<INPUT type=\"submit\" style=\"height: 150px; width: 600px; font-size:33pt; margin-left: 200px;\"  value=\"jetzt neu starten\"</INPUT>";
    a += "</P>";
    a += "</FORM>";
    a += "</body>";
    a += "</html>";
    return a;
}


/*
 * 
 * Station Mode
 * 
 */

void handleST() {
    if (server.hasArg("ButtonName")) {
        server.send(200, "text/html", "<h1>tutwasessolllllll <br>...wird neu gestartet.</h1>");
    } else if(server.hasArg("done")) {
        server.send(200, "text/html", "<h1>Zugangsdaten Gespeichert <br> Bitte warten <br>...wird neu gestartet.</h1>");
    } else {
        server.send(200, "text/html", ST_INDEX_HTML());
    }
}

String ST_INDEX_HTML() {
    String a = "<!DOCTYPE HTML>";
    a += "<html>";
    a += "<form action=\"/\" method=\"POST\">";
    a += "<input name=\"ButtonName\" type=\"submit\" value=\"lookout\">";
    a += "</form>";
    a += "</html>";
    
    return a;
}




void setup() {
    Serial.begin(115200);
    pinMode(LEDPin, OUTPUT);
    digitalWrite(LEDPin, LOW);
    Serial.println("STARTUP");
    //write_eeprom("is_init", "false");
    inittest = read_eeprom("is_init");
    Serial.println(inittest);
    if ( inittest != "True" ) {
        Serial.println("startinit");
        clear_eeprom();
        delay(200);
        write_eeprom("is_init", "True");
        write_eeprom("ap_on", "True");
    }
    ap_on = read_eeprom("ap_on");
    if (ap_on == "True") {
        WiFi.mode(WIFI_STA);
        int numberOfNetworks = WiFi.scanNetworks();
        Serial.println(numberOfNetworks);
        for(int i =0; i<numberOfNetworks; i++){
            ap_nw_name[i] = WiFi.SSID(i);
            //Serial.println(ap_nw_name[i]);
            if (i == 17) {
              break;
            }
        }
        WiFi.disconnect();
        delay(1000);
        WiFi.mode(WIFI_AP);
        WiFi.softAP(ap_ssid, ap_pswd);
        server.on("/", handleAP);
        server.begin();
        Serial.println("ap beginn");
        digitalWrite(LEDPin, LOW);
    } else {
        String st_ssid = read_eeprom("st_ssid");
        String st_pswd = read_eeprom("st_paswd");
        WiFi.mode(WIFI_STA);
        WiFi.begin(st_ssid, st_pswd);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
        }
        server.on("/", handleST);
        server.begin();
        //timeClient.begin();
    }
    digitalWrite(LEDPin, HIGH);
}


void loop() {
    server.handleClient();
}
