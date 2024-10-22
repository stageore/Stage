
//Listes des librairies utile au fonctionnement des  object connecté à la  carte. 
#include <FastLED.h>// Utile pour contrôle les ruban de LED adressable
#include <Adafruit_BMP280.h> // Utile  pour extraire les infos du BME-BMP 280. 
//------------------------------------------------------------------------------
// Liste des librairies utile au fonctionnement de la connexion à internet et aux choses liées. 
#include <WiFi.h>// Permet la connexion au Wi-Fi
#include <Arduino_MQTT_Client.h> // Permet d'utilisé le protocole MQTT pour comuniquer via les ports MQTT de ThingsBoard (TB) 
#include <ThingsBoard.h> // Permet de communiquer avec le serveur TB une fois ce  dernier 
#include <HTTPClient.h> // Permet de se connecter à un site internet (ici API météo)
#include <ArduinoJson.h> // Permet d'extraire et de traiter les informations de l'API météo
//------------------------------------------------------------------------------
//Noms et  mot de passe du Wi-Fi sur lequel on souhaite se connecter (décomenter celui utile)
//Wi-Fi ORE de base. 
/*constexpr char WIFI_SSID[]="Association_ORE";
constexpr char WIFI_PASSWORD[]="Association0r3";

// Wi-Fi privé de ORE

constexpr char WIFI_SSID[]="ORE_PRIVE";
constexpr char WIFI_PASSWORD[]="ORE199421800@";*/

// Place pour un autre WiFi au besoin. 

constexpr char WIFI_SSID[]="Galaxy_S21";
constexpr char WIFI_PASSWORD[]="12345678";

//------------------------------------------------------------------------------
// Nom du serveur TB que l'on souhaite, décommenter le noms du  serveur voulu et commenter les autres. 
//serveur TB cloud utilisé pendant la création du code car les serveur TB de ORE marchais pas 
//constexpr char THINGSBOARD_SERVER[] = "thingsboard.cloud";
//Ancien serveur TB ORE:  (tjr down ce  jour )
//constexpr char THINGSBOARD_SERVER[] = "192.168.1.174";
//Nouveau Serveur 1
//constexpr char THINGSBOARD_SERVER[] = "192.168.10.29";
//nouveau Serveur 2
constexpr char THINGSBOARD_SERVER[] = "192.168.117.186";
//nouveau Serveur 3
//constexpr char THINGSBOARD_SERVER[] = "192.168.192.227";
//------------------------------------------------------------------------------
//Token de notre device sur le serveur voulu, décommenter le noms du  serveur voulu et commenter les autres. 
//constexpr char TOKEN[] = "tl45UzWSZCg5U77JQLA9";
//token ancien serveur TB de ore  
//constexpr char TOKEN[]="07qe5WIL8ZgA0lHy64UI";//device : test pompe TB simon , dashboard: aucun car pas  de connexion possible.msg d'erreur type  [TB]: connection failed
//token  serveur TB de ore 1
//constexpr char TOKEN[]="dFtaQsHw6k2zFcwOuu1c"; //device: test pompe TB simon  , dashboard: pareil qu'au dessus , msg d'erreur en français interne au code de l'IDE "echec de la connexion"
//token  serveur TB de ore 2 
constexpr char TOKEN[]="8RRD8oh4s06dsIzLkQm2";//device: pompe test simon , dashboard: pareil qu'au dessus , msg d'erreur en français interne au code de l'IDE "echec de la connexion"
//token  serveur TB de ore 3
//constexpr char TOKEN[]="5UeQ34ezRJWT6ctywCvZ";//device:pompe simple tuto  , dashboard:
//------------------------------------------------------------------------------
// Port MQTT utilisé pour  connecter à TB, le  1883 est celui non crypté par défaut. 
constexpr uint16_t THINGSBOARD_PORT = 1883U;
// Taille maximale des packets échanger entre serveur et carte
//bien vérifier qu'on ne dépasse la dépasse pas par la suite afin d'avoir toute les données envoyée/reçue.  
constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;
// Baude rate  utile pour avoir le moniteur série lisible afin de debug si besoin. 
constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;
// Nbr d'attribute d'un type pour lesquels on souhaite s'inscrire à TB 
constexpr size_t MAX_ATTRIBUTES = 3U;                           
//------------------------------------------------------------------------------
//// Constantes pertinentes pour l'API
String api_key = "48af8fee764ae56aa1395fe51afba45c"; // clefs
String api_place="Quetigny"; // Lieux des infos voulues
String api_country="FR"; //Code à 2 lettres du pays de l'endroit voulu.  
String url = "http://api.openweathermap.org/data/2.5/forecast?q="+api_place+","+api_country+"&APPID=" + api_key;
// lien complet de l'API pour checker  la forme envoyée, au cas ou elle évolurais ou de nouvelle infos serais interessante à aller cherchée(control+click pour ouvrir)
//http://api.openweathermap.org/data/2.5/forecast?q=Quetigny,FR&APPID=48af8fee764ae56aa1395fe51afba45c

//Definition des proba pour chaque jour parmis les  5 jours dispo sur l'API
float p_J1=1.0; //p sont les proba intermédiaires de "non pluies" qui seront cumulées
float pop_J1=0.0;// pop sont les proba de pluies  calculé comme  1-proba non pluies à la fin
float p_J2=1.0 ;
float pop_J2=0.0;
float p_J3=1.0 ;
float pop_J3=0.0;
float p_J4= 1.0;
float pop_J4=0.0;
float p_J5= 1.0;
float pop_J5=0.0;

// temps depuis le dernier call de l'API
unsigned long lastTime = 0;
// temps de  delay entre les requete API.
unsigned long timerDelay = 10000; // ICI  10sec pour les phases de test mais vu que l'api donne les donnée par tranches de 3h on peut réduire à une requête par 3h par la suite. 

// On défini le futur buffer  utile dans notre code. 
 String jsonBuffer;

//------------------------------------------------------------------------------
// Constante importante pour les rubans de LEDs.
#define humstrippin 17 // pin du  1er ruban de LED pour humidité 
#define temptrippin 18 // pin du 2nd ruban de LED pour temperature.
#define LED_TYPE WS2812B // type de LED des rubans 
#define NUM_LED 10 // nombre de led par rubans 
#define BRIGHTNESS 10 // valeur de la luminosité des leds
#define COLOR_ORDER GRB // ordre dans lesquels on va Imputs rouge/vert/bleu (trouvé par expérience)
//------------------------------------------------------------------------------
//Definition du pins du capteur d'humidité (si capacitif faire attention à ce que ce soit un pin analogique)
 #define moisturePin 34
 //Definition du pin du relais permettant d'activation de la pompe. 
 #define relayPin 19
 //Definition du  taux d'humidité critique necessitant l'arrosage et sa valeur par défaut.  
 int humidityRate=80; 
//------------------------------------------------------------------------------
//Déclaration du client TB en 3 étapes 
// Initialisation du client Wi-Fi
WiFiClient wifiClient;
// Initalisation du client MQTT
Arduino_MQTT_Client mqttClient(wifiClient);
// Intitialisation de TB avec la taille de buffer maximale. 
ThingsBoardSized<Default_Fields_Amount, Default_Subscriptions_Amount, MAX_ATTRIBUTES> tb(mqttClient, MAX_MESSAGE_SIZE);

//déclaration du BMP connecter à la carte. 
Adafruit_BMP280 bmp;

// Déclaration des rubans de LED connecter avec  2 nombre de LED , ici les 2 rubans on le même nombre de LED.
CRGB led_humi[NUM_LED];
CRGB led_temp[NUM_LED];
