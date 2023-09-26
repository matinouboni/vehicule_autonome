#ifndef STRUCTURES_H
#define STRUCTURES_H

#define MAX_RUES 1000
#define MAX_NOEUDS 1000

// Pour Dijsktra
#define BLANC 0  //pas encore traité 
#define GRIS 1   // en cours de traitement 
#define NOIR 2   //déjà traité


// Etat vehicule
#define LIBRE 0
#define OCCUPE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Structure pour représenter une rue
typedef struct
{
    int start; // Point de départ de la rue
    int end;   // Point d'arrivée de la rue
    int time;  // Temps de parcours en secondes
} Street;

// Structure pour représenter un appel client
typedef struct
{
    int time;        // Temps de réception de l'appel
    char name[26];   // Nom du client
    int origin;      // Point d'origine de l'itinéraire demandé
    int destination; // Point de destination de l'itinéraire demandé
    int minTime;     // Heure minimale de départ
    int maxTime;     // Heure maximale d'arrivée
    int prime;       // Prime applicable si besoin
} Call;

// Structure voyages
typedef struct
{
    int time;      // Temps de réception de l'appel
    char name[26]; // Nom du client
    int duree;     // duree du voyage origine (o) -> destianation (d)
    int *noeuds;   // noeuds parliste des indices des points visité le long de l’itinéraire (o -> d)
} Voyage;

typedef struct
{
    int nombre_noeuds; // nombre noeuds dans le resauu routier
    int nombre_rues;   // nombre de rues du reseau
    Street *rue;
} Reseau;

// Structure pour représenter les caractéristiques des véhicules
typedef struct
{
    int nbVehicules;       // Nombre de véhicules disponibles
    int intervalleService; // Intervalle de service en secondes
    int autonomieBatterie; // Durée de vie de la batterie en secondes
    int tempsRecharge;     // Durée de recharge en secondes
} CaracteristiquesVehicules;

//

// Structure pour représenter l'etat d'un vehicule
typedef struct
{
    int numero;
    int position;
    int charge;
    int etat;
} Vehicule;

// Evenement
enum EventType
{
    CALL,          // appel d’un client
    END_SERVICE,   // l’arrivée à destination d’un client
    RETURN_CHARGE, // rentrée d’un véhicule pour la charge
    END_CHARGE     // fin de la recharge d’un véhicule
};
typedef struct event
{
    int time;
    enum EventType type;
    int vehicule;
    Call *appel; // Appel du client si type call
    struct event *next;
    struct event *prev;
} Event;

// declaration de fonction utilisateur

int lire_fichier_reseau(const char *fichier_reseau, Reseau *reseau);
int lire_fichier_vehicules(const char *fichier_vehicules, CaracteristiquesVehicules *vehicules);
int lire_fichier_appels(const char *fichier_appels, Call **appels, int *nombre_appels);
void trier_clients(Call *appels, int nombre_appels);
void afficher_clients(Call *appels, int nombre_appels);

// Trouver le meilleur chemin avec l'algorithme de Dijkstra

/********************************************************************************************/
/*A la fin d'execution de la fonction dijkstra, les tableaux precdecesseur et distance sont */
/*rempli par les predesseur du pint origine fourni dans dijkstra. la fonction trouver trajet*/
/*trajet dijkstra nous donne les trajet pour aller d'un point de depart à un point d'arrivé,*/
/*et les durée. La fonction durée min est utilisée pour optimiser                           */
/********************************************************************************************/

void dijkstra(Reseau reseau, int origine, int *predecesseurs, int *distances);
void relacher(int start, int end, int cout, int *predecesseurs, int *distances);
int trouver_trajet_dijkstra(Reseau reseau, int origine, int destination, int *duree, int *noeuds);
int duree_min(Reseau reseau, int origine, int destination);

// Voyages
void calculer_voyages(Reseau reseau, Call *appels, Voyage **voyages, int nombre_appels);
void trier_voyages(Voyage *voyages, int nombre_appels);
void afficher_voyages(Voyage *voyages, int nombre_appels);

// Positions initiales des véhicules
void definir_positions_initiales_vehicules(Reseau reseau, CaracteristiquesVehicules crt_vehicules, Vehicule **vehicules);
void afficher_positions_vehicules(CaracteristiquesVehicules crt_vehicules, Vehicule *velicules);

// Etats et charges
void init_etats_charges(CaracteristiquesVehicules crt_vehicules, Vehicule *vehicules);
void afficher_vehicules(Vehicule *vehicules, int nb_vehicules);

// Choix de vehicules
Vehicule *vehicules_qui_peuvent_servir(Reseau reseau, Call appel, CaracteristiquesVehicules crt_vehicules, Vehicule *vehicules, int *nb_peuvent);
Vehicule choisir_meilleur_vehicule(Reseau reseau, Call appel, Vehicule *vehicule_qui_peuvent, int nb_peuvent);

// Event file
Event *create_event(int time, enum EventType type, int vehicule, Call *appel);
Event *enfiler_event(Event *events, Event *event);
Event *defiler_event(Event **events);

// Charger initialement tous les appels
Event *init_call_events(Event *events, Call *appels, int nb_appel);

// Afficher events
void afficher_events(Event *events);
const char *event_type_tostring(enum EventType type);

// File d'attente de recharge
int *init_file_attente_recharge(int nb_vehicules);

// L'heure à la quelle les rechargement actuels seront finis
int get_debut_chargement(int *file_attente, int nb_vehicules);

// Surestimation du gain, cette fonction est e encore retravaillé
int surestimation_gain(Reseau reseau, Event *events, CaracteristiquesVehicules crt_vehicules);

#endif // STRUCTURES_H