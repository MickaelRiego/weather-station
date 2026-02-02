# Station Météo DHT11 (ESP32)

## Description

Ce projet consiste en une station météo simplifiée utilisant un microcontrôleur ESP32 et un capteur DHT11. L'objectif est de lire les données de température et d'humidité ambiante, de vérifier leur intégrité, et de les transmettre via une liaison série.

## Matériel utilisé

* **Microcontrôleur** : ESP32 (AZ-Delivery DEV KIT C V4).
* **Capteur** : DHT11 (connecté sur la broche GPIO 14).
* **Bread board**

## Techniques et Implémentation

Le projet utilise le framework Arduino sous l'environnement PlatformIO. Contrairement à l'utilisation de bibliothèques tierces, la communication avec le capteur est implémentée manuellement pour démontrer le fonctionnement du protocole One-Wire spécifique au DHT11.

### Protocole de communication

L'implémentation dans `src/main.cpp` suit les étapes suivantes :

1. **Signal de départ** : Le microcontrôleur force la ligne de données à l'état bas pendant 18ms minimum, puis repasse en mode écoute.
2. **Lecture temporelle** : Le capteur répond par une série d'impulsions. La distinction entre un bit "0" et un bit "1" se fait par la mesure de la durée de l'état haut (environ 26-28µs pour un 0, contre 70µs pour un 1).
3. **Structure des données** : Les 40 bits reçus sont stockés dans une structure `DhtData` comprenant :
* L'humidité (partie entière et décimale).
* La température (partie entière et décimale).
* Un octet de contrôle (Checksum).



### Vérification de l'intégrité

Une validation par somme de contrôle est effectuée à chaque lecture. Le programme additionne les quatre premiers octets de données et compare le résultat au cinquième octet transmis par le capteur. Si les valeurs ne correspondent pas, les données sont marquées comme invalides.

## Structure du projet

* `src/` : Contient le code source principal (`main.cpp`).
* `include/` : Destiné aux fichiers d'en-tête du projet.
* `lib/` : Répertoire pour les bibliothèques privées.
* `platformio.ini` : Configuration de l'environnement de build et de la carte.

## Utilisation

1. Connecter le capteur DHT11 à la broche 14 de l'ESP32.
2. Compiler et téléverser le code via PlatformIO.
3. Ouvrir le moniteur série à une vitesse de 9600 bauds pour visualiser les mesures.

## Travail sur affichage sur écran e-ink
https://www.waveshare.com/w/upload/7/77/1.54inch_e-Paper_Datasheet.pdf?srsltid=AfmBOopG8CxWWoCqWxTgIcdSl8F6E3AAnUci6lZO3Pe4QD5E7xlGouz7
