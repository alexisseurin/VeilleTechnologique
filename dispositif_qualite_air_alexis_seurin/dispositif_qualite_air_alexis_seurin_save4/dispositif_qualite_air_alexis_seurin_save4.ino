
  /*********************************************************************\
  |*                                                                   *|
  |*  Dispositif d'analyse de la qualité de l'air                      *|
  |*  ------------------------------------------------                 *|
  |*                                                                   *|
  |*  |/////////////////////////////|                                  *|
  |*  | Réalisation : Alexis Seurin |                                  *|
  |*  | Date :        Avril 2020    |                                  *|
  |*  |/////////////////////////////|                                  *|
  |*                                                                   *|
  |*  Matériels utilisés :                                             *|
  |*  - Arduino Uno (carte électronique intégrant un microcontrôleur)  *|
  |*  - Base Shield (coeur du système Grove)                           *|
  |*  - Capteur de gaz multicanal (MiCS-6814)                          *|
  |*  - Capteur de CO2, température et d'humidité (SCD30)              *|
  |*  - Câble USB pour Arduino                                         *|
  |*  - 2 câbles Grove avec 4 broches                                  *|
  |*                                                                   *|
  |*                                                                   *|
  |*  Version du logiciel Arduino : Arduino 1.8.12                     *|
  |*                                                                   *|
  \*********************************************************************/


/* Instruction : Consigne exprimée dans un langage de programmation.           *        
 *                                                                             *
 * Le point-virgule est utilisé et obligatoire à la fin d'une instruction.     *
 * Oublier le point virgule en fin de ligne donnera une erreur de compilation. */


/* ----------------------------------------------------------------------
    include [Instruction]
    
    Utilisée pour insérer des bibliothèques
    (libraries, en anglais) dans notre programme.

   Pour plus d'informations : https://www.arduino.cc/en/reference/libraries
   ---------------------------------------------------------------------- */

#include <Wire.h>                   /* Bibliothèque permettant la communication avec le bus I2C (Inter-Integrated Circuit) */
#include <MutichannelGasSensor.h>   /* Bibliothèque - Capteur de gaz multicanal (MiCS-6814) */
#include <SCD30.h>                  /* Bibliothèque - Capteur de CO2, température et d'humidité (SCD30) */


/* ----------------------------------------------------------------------
    define [Instruction]
    
    Permet au programmateur de donner un nom à une constante
    avant que le programme soit compilé.

    /!\ Les constantes ne prennent aucune place supplémentaire
        en mémoire dans le microcontrôleur.

    Contrairement à une variable, une constante est
    un identificateur associé à une valeur fixe.
   ---------------------------------------------------------------------- */

#define LED 13 /* Les constantes sont des variables prédéfinies. Ici, il s'agit de la constante LED */
#define SENSOR_ADDR 0X04  /* SENSOR_ADDR sera par défaut sur 0x04 */


/* ----------------------------------------------------------------------
    Variables globales
   ---------------------------------------------------------------------- */

/* Déclaration du tableau contenant les noms des particules à analyser */
const String data[11] = {"CO2", "NO2", "NH3", "CO", "C3H8", "C4H10", "CH4", "H2", "C2H5OH", "Temp", "Hum"};


/* ----------------------------------------------------------------------
    Fonctions implémentées obligatoires dans tout programme Arduino
    
    _______________________________________
    
    void setup()
    
    Elle initialise des variables, des bibliothèques utilisées et le sens
    des broches au démarrage du programme.
    La fonction setup n'est exécutée qu'une seule fois, après chaque mise
    sous tension ou reset (réinitialisation) de la carte Arduino.
    _______________________________________
    
    void loop()
    
    Elle s'exécute en boucle.
    _______________________________________
   ---------------------------------------------------------------------- */

void setup() /* fonction setup - Début de l'exécution du programme */
/* Le mot-clé "void" indique au compilateur que l'on s'attend à ce que la fonction ne retourne aucune donnée */
{
  
  /* Initialisation de la LED (Light-Emitting Diode - OU DEL, diode électroluminescente, en français) *
   * qui signifie qu'il y a une erreur dans l'analyse des particules dans l'air                       */

  /* Configure la broche spécifiée pour qu'elle se comporte soit en entrée, soit en sortie. */
  pinMode(LED, OUTPUT); /* pinMode(broche, mode); INPUT = 1  OUTPUT = 0 */

  /* L'instruction digitalWrite avec la constante LOW met à niveau la broche à 0 volt */
  digitalWrite(LED, LOW); /* digitalWrite(broche, valeur);  HIGH = 1  LOW = 0 */

  /**************************************************************************************** 
   * Fixe le débit de communication en nombre de caractères par seconde (unité : baud)    *
   * pour la communication série.                                                         *
   *                                                                                      *
   * Baud : Unité de mesure utilisée dans les domaines de l'informatique et               *
   *        des télécommunications. Le terme "baud" provient du patronyme d'Emile Baudot, *
   *        l'inventeur du code Baudot utilisé en télégraphie.                            *
   ****************************************************************************************/
   

  Serial.begin(115200);   /* Serial.begin(débit); Plus le débit est élevé, plus la communication est rapide. */

  /* Initialise la bibliothèque Wire et se connecte au bus I2C en tant que maître ou esclave. *
   * Cette bibliothèque vous permet de communiquer avec les composants utilisant le protocole *
   * I2C / TWI (communication série sur 2 fils).                                              */
  Wire.begin();           /* Wire.begin(adresse); en mode maitre */

 /***************************************************************************************************************
  * I2C est un bus série synchrone bidirectionnel (half-duplex), où plusieurs équipements, maîtres ou esclaves, *
  * peuvent être connectés au bus. Il permet de relier facilement un microprocesseur et différents circuits.    *
  * Les échanges ont toujours lieu entre un seul maître et un (ou tous les) esclave(s), toujours à l'initiative *
  * du maître.                                                                                                  *
  *                                                                                                             *
  * La connexion est réalisée par l'intermédiaire de deux lignes :                                              *
  * - SDA (Serial Data Line) : ligne de données bidirectionnelle,                                               *
  * - SCL (Serial Clock Line) : ligne d'horloge de synchronisation bidirectionnelle.                            *
  ***************************************************************************************************************/

  /* Initialise la bibliothèque MutichannelGasSensor (Capteur de gaz multicanal) *
   * et se connecte au bus I2C en tant que maître ou esclave.                    */
  gas.begin(SENSOR_ADDR); /* L'adresse I2C par défaut de l'esclave (slave) est 0x04 */

  /* Initialise la bibliothèque SCD30 (Capteur de CO2, température et d'humidité) */
  scd30.initialize();    /* Démarre des mesures périodiques */
  
}


void loop() /* La fonction loop est exécutée en boucle dès que la fonction setup a été exécutée  */
{
  
  int i;                  /* Variable i (integer) qui sera notamment utilisée pour la boucle for      */
  bool v = 0;             /* Variable v (booléen) qui permet de savoir si le capteur SCD30 est activé */
  float result[3] = {0};  /* Tableau pour le capteur de CO2, température et d'humidité (SCD30)        */
  float c;                /* Concentration d'un polluant, unité : ppm (partie par million) 10^-6      */


  /* ----------------------------------------------------------------------
      if [Opérateur logique de comparaison]
     
      Cet opérateur permet de tester si une condition est vraie.
     ---------------------------------------------------------------------- */

  /* Vérification si le capteur SCD30 est disponible */
  if (scd30.isAvailable()) {
    
    /* Récupération des résultats grâce à la fonction getCarbonDioxideConcentration */
    scd30.getCarbonDioxideConcentration(result);
    v = 1; /* La variable v est égale à 1 (ou TRUE). */
    
  }

  /* Si v est égal à 1, alors la condition est vraie. */
  if (v == 1) {

    /* ----------------------------------------------------------------------
        for [Instruction] - Très utile pour toutes les opérations répétitives
    
        Elle est utilisée pour répéter l'exécution d'un bloc d'instructions
        regroupées entre des accolades. Un compteur incrémental est utilisé
        pour incrémenter etfinir la boucle.
       ---------------------------------------------------------------------- */

    /* Boucle for incrémentant la variable i de 0 à 10, de 1 en 1 *
     * for (initialisation; condition; incrémentation) {}         */
    for (i = 0; i < 11; i++) {

      /* Si i=0, affichage de {"CO2":
         Utilisation de \" qui permet de mettre un guillemet dans le texte affiché */
      Serial.print("{\""); Serial.print(data[i]); Serial.print("\":");


      /* ----------------------------------------------------------------------
          Switch / case
          [Instruction] Tout comme l'instruction if, il contrôle le déroulement
                        des programmes. Il construit une liste de possibilités
                        à l'intérieur d'accolades.
        ---------------------------------------------------------------------- */
      switch (i) {
        
        case 0: /* Faire quelque chose quand la variable i est égal à 0 */
          c = result[0]; /* Concentration de dioxyde de carbone (CO2) - Elément fondamental du cycle du carbone sur notre planète. */
          break; /* L'instruction break indique à l'instruction switch d'arrêter de rechercher des conditions vraies.  */

        case 1:
          c = gas.measure_NO2(); /* Concentration de dioxyde d'azote (NO2) - Polluant majeur de l'atmosphère terrestre *
                                  * produit par les moteurs à combustion interne et les centrales thermiques.          */
          break;

        case 2:
          c = gas.measure_NH3(); /* Concentration de d'ammoniac (NH3) - En excès dans l'air, c'est un polluant *
                                  * acidifiant de l'environnement.                                             */
          break;

        case 3:
          c = gas.measure_CO(); /* Concentration de monoxyde de carbone (CO) - Gaz incolore, inodore, insipide et non irritant. */
          break;

        case 4:
          c = gas.measure_C3H8(); /* Concentration de propane (C3H8) - Combustible et carburant. */
          break;

        case 5:
          c = gas.measure_C4H10(); /* Concentration de butane (C4H10) - Gaz utilisé comme combustible (gazinière, chauffe-eau, etc.). */
          break;

        case 6:
          c = gas.measure_CH4(); /* Concentration de méthane (CH4) - Naturellement présent dans l'atmosphère terrestre. */
          break;

        case 7:
          c = gas.measure_H2(); /* Concentration de dihydrogène (H2) - Gaz transparent aux conditions normales de température et de pression. */
          break;

        case 8:
          c = gas.measure_C2H5OH(); /* Concentration d'éthanol (C2H5OH) - Composé chimique utilisé comme solvant dans l'industrie chimique. */
          break;

        case 9:
          c = result[1]; /* Température (en °C) */
          break;

        case 10:
          c = result[2]; /* Humidité de l'air (en %) */
          break;
          
      }

      /* Affichage de la valeur c en ppm  0.42} */
      Serial.print(c); 
      Serial.print("}");
      Serial.println("");

    } /* Fin de la boucle for */
    
  }
  else {
    /* Si v est égal à 0, alors la condition est fausse. *
     *                                                   *
     * En cas d'erreur, la LED rouge s'allume.           */
     
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);  
    delay(500);
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    
  }
  
  /* Fin de l'analyse */
  
  delay(2000); /* 2 secondes de délai avant la nouvelle analyse */
}
