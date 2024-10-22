//Nommé tout les attributs pertinent pour la suite 
constexpr char T_ARROSAGE_ATTR[] = "t_arro"; //temps d'arrosage
constexpr char BTN_MANUEL_ATTR[]="btn_manuel"; //variable du bouton arrosage manuel
constexpr char BTN_AUTO_ATTR[]= "btn_auto";//variable du bouton type de 


//Valeurs par défauts de nos attributs. 
volatile bool btn_manuel =false;//Le mode manuel l'arrosage ne se lance pas avant l'input utilisateur. 
volatile bool btn_auto= true;// Ici on souhaite que le mode auto soit mis par défaut
volatile uint16_t t_arro=1000U; //temps par défaut d'arrosage de 1ssec,  surtout un place holder qui sera à modif par le jardinier. 

// On défini les limite des temps d'arrosage possible afin d'éviter des temps absurde à cause de faute de frappe (encore une fois place  holder à modifié après consulation des jardiniers)
constexpr uint16_t T_ARROSAGE_MS_MIN = 10U;
constexpr uint16_t T_ARROSAGE_MS_MAX = 60000U;

// Bool utile pour stocker si on a changer un attributs ou pas. 
volatile bool attributesChanged = false;tes
// et constante qui servira à compter le temps entre les changement d'état.
uint32_t previousStateChange;

// Constante pour la  télémétrie 
constexpr int16_t telemetrySendInterval = 2000U; //Temps entre les envoie de télémétrie
uint32_t previousDataSend; // Temps depuis la dernière télémétrie. 


// List of shared attributes for subscribing to their updates
constexpr std::array<const char *, 3U> SHARED_ATTRIBUTES_LIST = {
  T_ARROSAGE_ATTR,
  BTN_MANUEL_ATTR,
  BTN_AUTO_ATTR
};
