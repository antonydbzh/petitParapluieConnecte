///////////////
// PARAPLUIE //
///////////////

/*  Un programme pour récupérer les données météo du site http://openweathermap.org/
 *  puis les afficher sur le moniteur série (partie de programme inspiré de http://educ8s.tv/esp8266-weather-display/)
 *  et ouvre ou ferme un parapluie chinois suivant les précipitations d'un lieu choisis (inspiré de http://julienlevesque.net/little-umbrella/ ) 
 *  Cette version intègre une page web de test du parapluie en hackant des bout de codes trouvés ici : https://www.ulasdikme.com/projects/esp8266/esp8266_ajax_websocket.php
 *  Antony Le Goïc-Auffret
 *  sous licence CC-By-SA - dimanche 1er août 2020 - Brest- Bretagne - France - Europe - Terre - Système solaire - Voie Lactée.

                                                                                        _
                                                                                      _| |_
                                                                                   _/ /  \  \_
                                                                                _/   /    \    \_ 
                                                                             _/     /      \      \_ 
                                                                          _/  _._  /  _._   \ _._    \_
                                                                        / \_/     \_/  |_|\_/     \_/  \ 
                                                                                      /|#|
                                                                                     | | |
                                                                                     | | |
                                                                                     | | |
                                                                                     | | |
                                                                                     | | |  
                                                                                     | | |
                                                                                 ____|_\_/_
                                                                                |    |     |
                                     BROCHAGE                                   |    |     |             
                               _________________                          ______|____|_____|__________
                              /     D1 mini     \                        |           |               |
              Non Attribué - |[ ]RST        TX[ ]| - Non Attribué        |   ___     |               |
              Non Attribué - |[ ]A0  -GPIO  RX[ ]| - Non Attribué        |  |_°_|    |               |
              Non Attribué - |[ ]D0-16    5-D1[ ]| - Non Attribué        | |     |   |               |
              Non Attribué - |[ ]D5-14    4-D2[ ]| - Non Attribué        | |     |   |               |
              Non Attribué - |[ ]D6-12    0-D3[X]| - gestion servo < _   | |   __|___|               |
              Non Attribué - |[ ]D7-13    2-D4[X]| - LED_BUILTIN       \ | | (o)_____/               |
              Non Attribué - |[ ]D8-15     GND[X]| - alim servo <-      || |Servo|                   |
              Non Attribué - |[ ]3V3 .      5V[X]| - alim servo <_|_    || |_____|                   |
                             |       +---+       |                |  \   \  |_°_|                    |
                             |_______|USB|_______|                  \  \ |\__/||                     |
              Le wemos est connecté en USB à votre ordinateur,        \ \|___/ |                     |
                        le moniteur série ouvert                       \_|____/                      |
                                                                         |___________________________|

*/
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Servo.h> 
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);                         //declaration du serveur web sur le port 80

const char* nomDuReseau = "xxxxxxxxx";               // Nom du réseau wifi local (Ce qu'on appelle le SSID).
const char* motDePasse = "yyyyyyyyyyyyy";            // mot de passe du réseau wifi local
String cledAPI = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; // clé de l'API du site http://openweathermap.org/
// il faudra vous créer un compte et récupérer votre clé d'API, une formalité !
// Sur le site, vous trouvez les identifiants de toutes les villes du monde.
String identifiantVille = "6427109";                 // indiquez l'identifiant d'une ville (CityID), ici Caen en France
/*Quelques identifiants d'autres villes françaises : 
 * 3030300, Brest
 * 6431033, Quimper
 * 6430976, Morlaix
 * 6453798, Lannion
 * 6453805, Saint-Brieuc
 * 6432801, Rennes
 * 6437298, Lorient
 * 2970777, Vannes
 * 6434483, Nantes
 * 6456407, Le Mans
 * 6427109, Caen
 * 6452361, Angers
 * 6456577, La Roche sur Yon
 * 3021411, Dieppe
 */

WiFiClient client;
char nomDuServeur[] = "api.openweathermap.org"; // Serveur distant auquel on va se connecter
String resultat;

int webClic = 0;
unsigned long dateDernierChangement = 0;
unsigned long dateCourante;
unsigned long intervalle;

Servo monservo;   // créer un objet "monservo" pour le contrôler

String descriptionMeteo = "";
String lieuMeteo = "";
String Pays;
float Temperature;
float Humidite;
float Pression;
int para = 100;
int parastock;
int ferme = 90;  // angle pour fermer le parapluie
int ouvre = 170; // angle pour ouvrir le parapluie
bool pluie = 1;  // enregistre si il pleut ou pas.

String webSite,javaScript,XML; //déclaration de variables
int start=0; // variable start

void buildWebsite(){ // Fonction qui écrit le code html du site web
  buildJavascript(); // appel de la fonction qui contruit le code javascript
  webSite="<!DOCTYPE HTML>\n";
  webSite+=javaScript; // insertion du javascript dans la page
  webSite+="<HTML>\n";
  webSite+="<style>\n";
  webSite+="#button {\n";
  webSite+="background-color: #E6E6FA;\n";
  webSite+="border: none;\n";
  webSite+="color: white;\n";
  webSite+="padding: 32px;\n";
  webSite+=" text-align: center;\n";
  webSite+=" text-decoration: none;\n";
  webSite+="display: inline-block;\n";
  webSite+="font-size: 168px;\n";
  webSite+="display:block;\n";
  webSite+="margin:0 auto;\n";
  webSite+="margin-top:130px;\n";
  webSite+="cursor: pointer;\n";
  webSite+="width:524px;\n";
  webSite+="height:400px;\n";
  webSite+="}\n";
  
  webSite+="p.thicker{font-weight:900;}\n";
  webSite+="#runtime{font-weight:900; font-size: 147%; color:RED;}\n";
  webSite+="</style>\n";
  webSite+="<BODY bgcolor='#E6E6FA' onload='process()'>\n";

  webSite+="<button onClick='RunButtonWasClicked()' id='button'></button>\n";
  webSite+="</BODY>\n";
  webSite+="</HTML>\n";
}

void buildJavascript(){ // Fonction qui contruit le code javascript
  javaScript="<SCRIPT>\n";
  
  javaScript+="var xmlHttp=createXmlHttpObject();\n";
  javaScript+="function createXmlHttpObject(){\n";
  javaScript+=" if(window.XMLHttpRequest){\n";
  javaScript+="    xmlHttp=new XMLHttpRequest();\n";
  javaScript+=" }else{\n";
  javaScript+="    xmlHttp=new ActiveXObject('Microsoft.XMLHTTP');\n";
  javaScript+=" }\n";
  javaScript+=" return xmlHttp;\n";
  javaScript+="}\n";
  
  javaScript+="var click;\n";
  
  javaScript+="function handleServerResponse(){\n";
  javaScript+="   xmlResponse=xmlHttp.responseXML;\n";
  javaScript+="   xmldoc = xmlResponse.getElementsByTagName('response');\n";
  javaScript+="   message = xmldoc[0].firstChild.nodeValue;\n";
  javaScript+="if(message == 1){click = 1; message = 'ouvert'; document.getElementById('button').style.background='#FFA200';}else{click=0; message='ferm&eacute;'; document.getElementById('button').style.background='#111111';}\n";
  javaScript+="   document.getElementById('button').innerHTML=message;\n";
  javaScript+="}\n";

  javaScript+="function process(){\n";
  javaScript+="   xmlHttp.open('PUT','xml',true);\n";
  javaScript+="   xmlHttp.onreadystatechange=handleServerResponse;\n"; // no brackets?????
  javaScript+="   xmlHttp.send(null);\n";
  javaScript+=" setTimeout('process()',200);\n";
  javaScript+="}\n";

  javaScript+="function process2(){\n";
  javaScript+="    xmlHttp.open('SET','set1ESPval?Start='+click,true);\n";
  javaScript+="    xmlHttp.send(null);\n";
  javaScript+=" setTimeout('process2()',400);\n";
  javaScript+="}\n";

  javaScript+="function RunButtonWasClicked(){\n";
  javaScript+="click = (click==1)?0:1;\n";
  javaScript+="    xmlHttp.open('SET','set1ESPval?Start='+click,true);\n";
  javaScript+="    xmlHttp.send(null);\n";
  javaScript+="}\n";       

  javaScript+="</SCRIPT>\n";
}

uint16_t x; 
String data; // variable data qui sert à ?

void buildXML(){
  XML="<?xml version='1.0'?>";
  XML+="<response>";
  XML+=data;
  XML+="</response>";
}


void handleWebsite(){ // génère le site web
  buildWebsite();     // écriture du html
  server.send(200,"text/html",webSite); // mise en ligne du site
}

void handleXML(){ // gère le xml (description de l'état du boutton)
  buildXML();
  server.send(200,"text/xml",XML);
}

void handle1ESPval(){ 
  start = server.arg("Start").toFloat();
}

int start2=0;
int inc=0;

void setup() {
  Serial.begin(9600);
  Serial.println();
  connexion();
  delay(100);
  server.on("/",handleWebsite);
  server.on("/xml",handleXML);
  server.on("/set1ESPval",handle1ESPval);
  
  server.begin(); 
  prendDonneesMeteo();
  parapluie (); 
}

void loop() {
  dateCourante = millis();
  intervalle = dateCourante - dateDernierChangement; // interval de temps depuis la dernière mise à jour du parapluie
  
  if (intervalle >= 600000) // Récupère de nouvelles données toutes les 10 minutes
    {
      dateDernierChangement = millis();
      prendDonneesMeteo();   // récupère les données météo
      parapluie();           // met à jour le parapluie
    }
    
   if (!start == webClic)   // si l'état du bouton web à changé
     {
     parastock = para;      // stock la valeur "para" dans "parastock"
     if (start)
       {
        para = 100;         // triche sur la valeur "para" pour un test pluie
        parapluie();        // met à jour le parapluie
        Serial.println("parapluie fermé ");
       }
     if (!start)
       {
        para = 900;        // triche sur la valeur "para" pour un test pluie
        parapluie();       // met à jour le parapluie
        Serial.println("parapluie ouvert ");
       }
     webClic = start;      // met à jour webClic
     para = parastock;     // redonne à para sa valeur initiale
     }

   data =(String)start;
   server.handleClient();

   if ((intervalle%6000) == 0){ // toutes les 6 secondes, j'écris ces infos sur le moniteur série 
     ecritMeteoGeneral(lieuMeteo, descriptionMeteo);
     Serial.print("interval modulo 6000 : "); Serial.println((intervalle%60));
     Serial.print("interval : "); Serial.println(intervalle);
     Serial.print("date Courante : ");Serial.println(dateCourante);
     Serial.print("date du Dernier Changement : ");Serial.println(dateDernierChangement);
   }
}

void prendDonneesMeteo() //Fonction qui utilise le client web du D1 mini pour envoyer/recevoir des données de requêtes GET.
{
  if (client.connect(nomDuServeur, 80)) {  // Démarre la connexion du client, recherche les connexions
    client.println("GET /data/2.5/weather?id=" + identifiantVille + "&units=metric&lang=fr&APPID=" + cledAPI);
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  }
  else {
    Serial.println("Echec de la connexion"); //message d'erreur si la connexion échoue
    Serial.println();
  }

  while (client.connected() && !client.available()) delay(1); // attend les données
  while (client.connected() || client.available()) {          // soit le client est connecté, soit il a des données disponibles
    char c = client.read();  // récupère les données
    resultat = resultat + c; // les agrège dans la chaine de caractère "resultat" 
  }

  client.stop(); // stoppe le client
  resultat.replace('[', ' ');
  resultat.replace(']', ' ');
  Serial.println(resultat); // écrit la chaine de caractère en entier sur le moniteur série

  char tableauJson [resultat.length() + 1];
  resultat.toCharArray(tableauJson, sizeof(tableauJson));
  tableauJson[resultat.length() + 1] = '\0';

 StaticJsonDocument<1024> doc;

 DeserializationError error = deserializeJson(doc, tableauJson);

if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
}

  String lieu = doc["name"];
  String pays = doc["sys"]["country"];
  float temperature = doc["main"]["temp"];
  float humidite = doc["main"]["humidity"];
  String meteo = doc["weather"]["main"];
  String description = doc["weather"]["description"];
  String id = doc["weather"]["id"];                   //récupère le chiffre identifiant "id" de l'état météo sous forme de texte.
  float pression = doc["main"]["pressure"];

  descriptionMeteo = description;
  lieuMeteo = lieu;
  Pays = pays;
  Temperature = temperature;
  Humidite = humidite;
  Pression = pression; 
  para =id.toInt(); //transforme le texte "id" en entier.

}

void ecritMeteoGeneral(String lieu, String description)
{
  Serial.println("------------------");
  Serial.print(lieu);
  Serial.print(", ");
  Serial.print(Pays);
  Serial.print(", ");
  Serial.print(description);
  Serial.print(", ");
  Serial.println(para);
}

void parapluie ()
{
  
if (para<600) {            // Si la valeur de l'indicateur météo est inférieur à 600 c'est qu'il pleut.
  if (pluie == 0) {        // Si avant ça il ne pleuvait pas
    monservo.attach(0);    // brancher le servomoteur sur la broche D3 (GPIO 0)
    monservo.write(ouvre); // ouvre le parapluie
    Serial.print("ouvre à : ");Serial.println(ouvre);
    pluie = 1;             // note qu'il pleut
    Serial.print("pluie à : ");Serial.println(pluie);
  }
}
else {                     // si il ne pleut pas
  if (pluie == 1) {        // et que juste avant il pleuvait (le parapluie était donc ouvert).
    monservo.attach(0);    // brancher le servomoteur sur la broche D3 (GPIO 0)
    monservo.write(ferme); // ferme le parapluie
    Serial.print("ferme à : ");Serial.println(ferme);
    pluie = 0;             // note bien qu'il ne pleut pas
    Serial.print("pluie à : ");Serial.println(pluie);
  }
}
delay (200);
monservo.detach();     // débrancher le servomoteur de la broche D3 (GPIO 0)
}

void connexion() {
  WiFi.mode(WIFI_STA); // Le D1 mini est en mode station wifi.
  Serial.println("Connexion");
  WiFi.begin(nomDuReseau, motDePasse);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connecté");
  Serial.print("Station IP address: ");
  Serial.println(WiFi.localIP());
  pinMode(LED_BUILTIN, OUTPUT);   // Configure la broche D4 (GPIO 2) sur la quelle est branchée la LED_BUILTIN en sortie
  digitalWrite(LED_BUILTIN, LOW); // Allume la LED_BUILTIN  
}
