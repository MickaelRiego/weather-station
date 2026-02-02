C'est la meilleure façon d'apprendre : avoir la carte sous les yeux mais tenir le volant soi-même.

Voici ton **TP "Bare Metal"**. Je vais te guider à travers la datasheet du **1.54inch e-Paper**  pour que tu écrives ton propre pilote. Oublie les copier-coller, on va chercher les informations à la source.

---

### **Objectif du TP**

Créer une station météo (DHT11 + Écran E-Ink) en codant "from scratch".
Tu as déjà le code du DHT11 dans ton fichier `main.cpp`. Nous allons nous concentrer sur l'écran.

---

### **Étape 1 : Câblage et Configuration SPI**

Avant de coder, il faut comprendre comment l'ESP32 parle à l'écran.

1. 
**Le Protocole** : Ouvre la **Page 6** de la datasheet ("Input/Output Terminals").


* Regarde la broche **BS1** (Pin 8). La note 5-5  explique comment choisir le mode SPI.


* *Question 1 :* À quel niveau logique (LOW/HIGH) dois-tu connecter BS1 pour être en mode "4-line SPI" ? (C'est le mode standard utilisé par l'ESP32).
* *Action :* Câble ton écran en conséquence.


2. **Les Broches de Commande** :
* Regarde les broches **CS#** (Pin 12) et **D/C#** (Pin 11) dans le tableau.


* 
*Question 2 :* Selon la **Page 14** (Table 7-2), quelle combinaison de **CS#** et **D/C#** est nécessaire pour envoyer une **COMMANDE** ? Et pour une **DONNÉE** ?
*Réponse :* 
**COMMANDE :** **CS#** = LOW, **D/C#** = LOW.
**DONNÉES :** **CS#** = LOW, **D/C#** = HIGH.



3. **Code (Setup)** :
* Configure tes broches ESP32 (CS, DC, RST, BUSY) en `OUTPUT` ou `INPUT` selon leur rôle.
* Initialise le bus SPI (utilise `SPI.begin()` de base, mais tu vas gérer CS et DC manuellement).

**BUSY** : PIN 25
**RST** : PIN 33
**DC** : PIN 32
**CS** : PIN 35



---

### **Étape 2 : L'implémentation Bas Niveau (La couche "Transport")**

Tu dois créer les deux fonctions fondamentales qui serviront tout le long du projet. Sans elles, rien ne marche.

Aide-toi du chronogramme "4-wire Serial Peripheral Interface" à la **Page 15** (Figure 7-1).

* **Défi 1 : Écrire `void sendCommand(uint8_t reg)**`
* Cette fonction doit :
1. Mettre la broche D/C à l'état correspondant à une "Commande".
2. Activer l'écran (CS LOW).
3. Envoyer l'octet via SPI (`SPI.transfer(reg)`).
4. Désactiver l'écran (CS HIGH).




* **Défi 2 : Écrire `void sendData(uint8_t data)**`
* C'est la même logique, mais l'état de la broche D/C change. Regarde bien la table 7-2.




* **Défi 3 : La fonction `waitBusy()**`
* L'écran est lent. Regarde la description de la broche **BUSY** (Pin 9) à la **Page 7** (Note 5-4).


* Quand BUSY est HIGH, le chip travaille. Écris une fonction bloquante `while` qui attend que cette broche repasse à LOW.



---

### **Étape 3 : La Séquence d'Initialisation (Le "Boot")**

C'est ici que tu vas manger de la datasheet. Le contrôleur de l'écran a besoin d'une séquence précise pour démarrer.
Va à la **Page 20** : "Typical Operating Sequence".

Suis le diagramme :

1. **Power On & Reset** :
* Implémente le toggle de la broche RST (LOW pendant 200ms, puis HIGH). N'oublie pas d'attendre (`waitBusy`) après.


2. **Configuration des registres** :
Le diagramme dit "Define the display size...". Pour savoir *comment* faire, tu dois consulter la **Command Table** (Page 8 à 12).
Voici les commandes essentielles à trouver et à envoyer (dans l'ordre) :
* 
**Driver Output Control (0x01)**:


* Il faut envoyer 3 octets de données après cette commande.
* Les deux premiers définissent la hauteur (MUX). Pour 200 lignes, quelle valeur hexadécimale correspond à 199 (0xC7) ?


* 
**Data Entry Mode (0x11)**:


* Cela définit comment le curseur avance quand tu envoies des pixels.
* Regarde la description (Page 9). On veut souvent que ça avance en X puis en Y (`11` en binaire = 0x03).


* 
**Set RAM X/Y Address Start/End (0x44 et 0x45)**:


* Tu dois définir la fenêtre de travail.
* X va de 0 à (200/8) - 1.
* Y va de 0 à 199.
* *Attention :* Regarde bien l'ordre des octets (LSB/MSB) pour Y dans la description de la commande 0x45.





---

### **Étape 4 : Afficher quelque chose (Write RAM)**

Une fois initialisé, l'écran attend une image.

1. **Se placer au début** :
* Trouve les commandes pour "Set RAM X Address Counter" et "Set RAM Y Address Counter" (Indices : 0x4E et 0x4F page 12 ). Mets-les à 0.




2. **Écrire les pixels** :
* Trouve la commande "Write RAM" (Page 11, commande 0x24 ).


* Après avoir envoyé cette commande, tu dois envoyer tes données.
* *Calcul :* L'écran fait 200x200 pixels. 1 bit = 1 pixel. Combien d'octets dois-tu envoyer au total via `sendData` ? (Boucle `for`).
* *Test :* Envoie `0xFF` (blanc) ou `0x00` (noir) pour voir si ça réagit.


3. **Rafraîchir (Le moment magique)** :
* Écrire dans la RAM ne suffit pas. Il faut déclencher l'update physique ("Master Activation").
* Regarde la commande **0x22** (Display Update Control 2) à la **Page 10**.


* La documentation suggère souvent la valeur `0xF7` (Load LUT + Display) suivie de la commande **0x20** (Master Activation).


* **Important :** Appelle `waitBusy()` juste après !



---

### **Étape 5 : Intégration (Le Final)**

1. Reprends ton code `readDHT11`.
2. Dans ta boucle `loop()` :
* Lis la température.
* Prépare un tableau `uint8_t buffer[5000]`.
* Remplis ce buffer manuellement (commence par faire une ligne noire : mets les 25 premiers octets à 0x00).
* Envoie le buffer à l'écran avec ta fonction de l'étape 4.
* Mets l'écran en veille (Deep Sleep, commande 0x10 ).





**À toi de jouer !** Si tu bloques sur une commande hexadécimale ou un timing, dis-moi laquelle et on regardera la page correspondante ensemble.