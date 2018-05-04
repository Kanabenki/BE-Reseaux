# Readme - Bureau d'étude Réseaux, TP2

## Compilation 

Commande : `make all` depuis le dossier mictp.

## Implémentation

Ajout de la gestion des pertes en mode Stop and Wait dans _mic\_tcp\_send_ et _process\_received\_PDU_

* _mic\_tcp\_send_: 
Après envoi d'un paquet, se mettre en attente d'un paquet contenant le flag ACK et le bon numéro de séquence avant d'envoyer un autre paquet. 

* _process\_received\_PDU_:
Si un paquet reçu n'est pas un paquet ACK ni SYN mais bien un paquet de transfet de données, alors on vérifie que son numéro de séquence est celui attendu et on le traite en conséquence. Ensuite on construit en on envoie un packet ACK qui porte le même numéro de séquence.

## A faire au prochain TP

Implémenter le Stop and Wait pour la partie connexion, dans_mic\_tcp\_connect_ et _mic\_tcp\_accept_
