#include "structure.h"
#include <limits.h>


/************************* Lecture données de simulation *****************************************/
// fonction de lecture du reseau routier
int lire_fichier_reseau(const char *fichier_reseau, Reseau *reseau)
{
    FILE *fichier = fopen(fichier_reseau, "r");
    if (fichier == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier %s\n", fichier_reseau);
        return 0;
    }

    if (fscanf(fichier, "%d %d", &(reseau->nombre_noeuds), &(reseau->nombre_rues)) != 2)
    {
        printf("Erreur lors de la lecture des données du fichier %s\n", fichier_reseau);
        return 0;
    }

    reseau->rue = (Street *)malloc(reseau->nombre_rues * sizeof(Street));
    if (reseau->rue == NULL)
    {
        printf("Erreur d'allocation mémoire\n");
        return 0;
    }

    for (int i = 0; i < reseau->nombre_rues; i++)
    {
        if (fscanf(fichier, "%d %d %d", &(reseau->rue[i].start), &(reseau->rue[i].end), &(reseau->rue[i].time)) != 3)
        {
            printf("Erreur lors de la lecture des données du fichier %s\n", fichier_reseau);
            free(reseau->rue);
            return 0;
        }
    }

    return 1;
}
// fonction de lecture des caractéristiques des vehicules 
int lire_fichier_vehicules(const char *fichier_vehicules, CaracteristiquesVehicules *vehicules)
{
    FILE *fichier = fopen(fichier_vehicules, "r");
    if (fichier == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier %s\n", fichier_vehicules);
        return 0;
    }

    if (fscanf(fichier, "%d %d %d %d", &(vehicules->nbVehicules), &(vehicules->intervalleService), &(vehicules->autonomieBatterie), &(vehicules->tempsRecharge)) != 4)
    {
        printf("Erreur lors de la lecture des données du fichier %s\n", fichier_vehicules);
        return 0;
    }

    return 1;
}
// fonction de lecture des differents appels 
int lire_fichier_appels(const char *fichier_appels, Call **appels, int *nombre_appels)
{
    FILE *fichier = fopen(fichier_appels, "r");
    if (fichier == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier %s\n", fichier_appels);
        return 0;
    }

    if (fscanf(fichier, "%d", nombre_appels) != 1)
    {
        printf("Erreur lors de la lecture des données du fichier %s\n", fichier_appels);
        return 0;
    }

    *appels = (Call *)malloc((*nombre_appels) * sizeof(Call));
    if (*appels == NULL)
    {
        printf("Erreur d'allocation mémoire\n");
        return 0;
    }

    for (int i = 0; i < *nombre_appels; i++)
    {
        if (fscanf(fichier, "%d %25s %d %d %d %d %d", &((*appels)[i].time), (*appels)[i].name, &((*appels)[i].origin), &((*appels)[i].destination), &((*appels)[i].minTime), &((*appels)[i].maxTime), &((*appels)[i].prime)) != 7)
        {
            printf("Erreur lors de la lecture des données du fichier %s\n", fichier_appels);
            free(*appels);
            return 0;
        }
    }

    return 1;
}

/************************   Trie des clients par ordre alphabetique ******************************/
// fonction de trie par ordre alphabetique
void trier_clients(Call *appels, int nombre_appels)
{
    // Trie les clients par ordre alphabétique
    for (int i = 0; i < nombre_appels - 1; i++)
    {
        for (int j = i + 1; j < nombre_appels; j++)
        {
            if (strcmp(appels[i].name, appels[j].name) > 0)
            {
                Call temp = appels[i];
                appels[i] = appels[j];
                appels[j] = temp;
            }
        }
    }
}
// fonction d'affichage de la liste des clients triés par ordre aphabetique
void afficher_clients(Call *appels, int nombre_appels)
{
    printf("\nClients:\n");
    for (int i = 0; i < nombre_appels; i++)
    {
        printf("%s\n", appels[i].name);
    }
}


/****************************** Algorithme de dijkstra pour la recherche de trajet optimal*******/
//sous fonction de dijkstra
void relacher(int start, int end, int cout, int *predecesseurs, int *distances)
{

    if (distances[start - 1] + cout < distances[end - 1])
    {
        distances[end - 1] = distances[start - 1] + cout;
        predecesseurs[end - 1] = start;
    }
}
//algo de dijkstra
void dijkstra(Reseau reseau, int origine, int *predecesseurs, int *distances) 
{
    // le tableau predecesseur va contenir tous les predecesseurs d'un noeud de notre graphe
    //le tableau distance, pour les durée du noeud à ses predecesseurs 
    int n = reseau.nombre_noeuds;
    int *couleurs = (int *)malloc(n * sizeof(int));

    int i;

    // Initialisation
    for (i = 0; i < n; i++)
    {
        distances[i] = INT_MAX;
        predecesseurs[i] = -1;
        couleurs[i] = BLANC;
    }

    distances[origine - 1] = 0; //on fait -1 pour respecter le decalage des indices de notre reseau par rapport au trableu
    couleurs[origine - 1] = GRIS;

    while (true)
    {

        int min_distance = INT_MAX;
        int s = -1;

        // Trouver le sommet gris avec la plus petite distance
        for (i = 0; i < n; i++)
        {
            if (couleurs[i] == GRIS && distances[i] < min_distance)
            {
                min_distance = distances[i];
                s = i + 1;
            }
        }

        if (s == -1) // pas de gris, l'algorithme est fini
            break;

        // Parcourir les successeurs du sommet s
        for (i = 0; i < reseau.nombre_rues; i++)
        {

            Street rue = reseau.rue[i];
        // on tient en compte du parcours dans les deux sens 
            if ((rue.start == s) || (rue.end == s))
            {
                int next = rue.start == s ? rue.end : rue.start;

                if (couleurs[next - 1] == BLANC || couleurs[next - 1] == GRIS)
                {
                    relacher(s, next, rue.time, predecesseurs, distances);
                    if (couleurs[next - 1] == BLANC)
                    {
                        couleurs[next - 1] = GRIS;
                    }
                }
            }
        }

        couleurs[s - 1] = NOIR;
    }
}

int trouver_trajet_dijkstra(Reseau reseau, int origine, int destination, int *duree, int *noeuds)
{
    int n = reseau.nombre_noeuds;
    int *distances = (int *)malloc(n * sizeof(int));
    int *predecesseurs = (int *)malloc(n * sizeof(int));

    // Calculer les chemins pour aller de l'origine aux autres noeud
    dijkstra(reseau, origine, predecesseurs, distances);

    int i, j;

    if (predecesseurs[destination - 1] == -1)
    {
        printf("Aucun chemin trouvé entre les nœuds %d et %d\n", origine, destination);
        return 0;
    }

    // durée
    *duree = distances[destination - 1];

    // determinons le chemin (les noeuds)
    int *chemin = (int *)malloc(reseau.nombre_noeuds * sizeof(int));
    int taille = 0;
    int noeud = destination;

    while (noeud != -1)
    {
        chemin[taille++] = noeud;
        noeud = predecesseurs[noeud - 1];
    }

    // Initialisation
    for (i = 0; i < n; i++)
    {
        noeuds[i] = -1;
    }

    for (i = taille - 1, j = 0; i >= 0; i--, j++)
    {
        noeuds[j] = chemin[i];
    }

    free(chemin);

    return 1;
}

int duree_min(Reseau reseau, int origine, int destination)
{
    int n = reseau.nombre_noeuds;
    int *distances = (int *)malloc(n * sizeof(int));
    int *predecesseurs = (int *)malloc(n * sizeof(int));

    // Calculer les chemins pour aller de l'origine aux autres noeud
    dijkstra(reseau, origine, predecesseurs, distances);

    return distances[destination - 1];
}

/*************************** Trie des appels par durée de voyage *****************************/
void calculer_voyages(Reseau reseau, Call *appels, Voyage **voyages, int nombre_appels)
{
    *voyages = (Voyage *)malloc((nombre_appels) * sizeof(Voyage));
    for (int i = 0; i < nombre_appels; i++)
    {

        int duree, *noeuds = (int *)malloc(reseau.nombre_noeuds * sizeof(int));
        trouver_trajet_dijkstra(reseau, appels[i].origin, appels[i].destination, &duree, noeuds);
        Voyage v = {
            .time = appels[i].time,
            .duree = duree,
            .noeuds = noeuds};
        strcpy(v.name, appels[i].name);
        (*voyages)[i] = v;
    }
}

void trier_voyages(Voyage *voyages, int nombre_appels)
{
    for (int i = 0; i < nombre_appels - 1; i++)
    {
        for (int j = i + 1; j < nombre_appels; j++)
        {
            if (voyages[i].duree < voyages[j].duree)
            {
                Voyage tmp = voyages[i];
                voyages[i] = voyages[j];
                voyages[j] = tmp;
            }
        }
    }
}

void afficher_voyages(Voyage *voyages, int nombre_appels)
{
    printf("\nVoyages:\n");
    for (int i = 0; i < nombre_appels; i++)
    {
        printf("%d %s %d | ", voyages[i].time, voyages[i].name, voyages[i].duree);
        for (int j = 0; voyages[i].noeuds[j] != -1; j++)
        {
            printf("%d ", voyages[i].noeuds[j]);
        }
        printf("\n");
    }
}

/************************** Positionnement des vehicules sur le reseau ***********************/
void definir_positions_initiales_vehicules(Reseau reseau, CaracteristiquesVehicules crt_vehicules, Vehicule **vehicules)
{
    *vehicules = (Vehicule *)malloc((crt_vehicules.nbVehicules) * sizeof(Vehicule));
    int n = reseau.nombre_noeuds;
    int *noeud_occupe = (int *)malloc((reseau.nombre_noeuds) * sizeof(int));
    for (int i = 0; i < reseau.nombre_noeuds; i++)
    {
        noeud_occupe[i] = -1;
    }

    // On met le premier véhicule (1) au premier noeud (0)
    (*vehicules)[0].position = 1;
    noeud_occupe[0] = 1;

    // Positionner les autres
    for (int i = 1; i < crt_vehicules.nbVehicules; i++)
    {
        int p = -1, dmax = -1;
        for (int j = 0; j < reseau.nombre_noeuds; j++)
        {
            // Si c'est pas occuper
            if (noeud_occupe[j] == -1)
            {
                // Calculer la distance aux autres dejà positionnés
                int *distances = (int *)malloc(n * sizeof(int));
                int *predecesseurs = (int *)malloc(n * sizeof(int));
                int s = 0;
                /*la fonction duree_min n'etait pas optimal ici donc 
                on a encore utilisé jijkstra*/
                dijkstra(reseau, j + 1, predecesseurs, distances);

                for (int k = 0; k < reseau.nombre_noeuds; k++)
                {
                    if (noeud_occupe[k] != -1)
                    {
                        s += distances[k];
                    }
                }

                if (s > dmax)
                {
                    dmax = s;
                    p = j + 1;
                }
            }
        }

        (*vehicules)[i].position = p;
        noeud_occupe[p - 1] = p; // pour dire déjà occupé (intialement -1)
    }
}

void afficher_positions_vehicules(CaracteristiquesVehicules crt_vehicules, Vehicule *vehicules)
{
    printf("\nPositons: ");
    for (int i = 0; i < crt_vehicules.nbVehicules; i++)
    {
        printf("%d ", vehicules[i].position);
    }
    printf("\n");
}

/******************* initialisation de l'autonomie d'un vehicule ****************************/
void init_etats_charges(CaracteristiquesVehicules crt_vehicules, Vehicule *vehicules)
{

    for (int i = 0; i < crt_vehicules.nbVehicules; i++)
    {
        vehicules[i].numero = i + 1;
        vehicules[i].etat = LIBRE;
        vehicules[i].charge = crt_vehicules.autonomieBatterie;
    }
}

/************************** Etat d'un vehicule à un instant donné ******************************/
void afficher_vehicules(Vehicule *vehicules, int nb_vehicules)
{
    printf("\nVehicules:\t");
    for (int i = 0; i < nb_vehicules; i++)
    {
        printf("%d\t", vehicules[i].numero);
    }
    printf("\nPositons:\t");
    for (int i = 0; i < nb_vehicules; i++)
    {
        printf("%d\t", vehicules[i].position);
    }
    printf("\nEtats:\t\t");
    for (int i = 0; i < nb_vehicules; i++)
    {
        printf(vehicules[i].etat == LIBRE ? "LIBRE\t" : "OCCUPE\t");
    }
    printf("\nCharges:\t");
    for (int i = 0; i < nb_vehicules; i++)
    {
        printf("%d\t", vehicules[i].charge);
    }
    printf("\n");
}

/*************************** Choix de vehicule à reception d'un appel *************************/
Vehicule *vehicules_qui_peuvent_servir(Reseau reseau, Call appel, CaracteristiquesVehicules crt_vehicules, Vehicule *vehicules, int *nb_peuvent)
{
    int heure = appel.time;
    Vehicule *vehicules_qui_peuvent = (Vehicule *)malloc((crt_vehicules.nbVehicules) * sizeof(Vehicule));
    int k = 0;
    for (int i = 0; i < crt_vehicules.nbVehicules; i++)
    {
        Vehicule v = vehicules[i];
        // Duree aller destination
        int duree = duree_min(reseau, v.position, appel.origin) + duree_min(reseau, appel.origin, appel.destination);
        // Duree aller et retour vers point de recharge
        int duree_total = duree + duree_min(reseau, appel.destination, 1);

        if (v.etat == LIBRE && heure + duree <= appel.maxTime && duree_total < v.charge)
        {
            vehicules_qui_peuvent[k++] = v;
        }
    }
    *nb_peuvent = k;
    return vehicules_qui_peuvent;
}

Vehicule choisir_meilleur_vehicule(Reseau reseau, Call appel, Vehicule *vehicules_qui_peuvent, int nb_peuvent)
{
    int heure = appel.time;
    Vehicule v_best;
    int heure_min = INT_MAX;
    // Choisir le vehicule qui arrive le premier au point de destination
    for (int i = 0; i < nb_peuvent; i++)
    {
        Vehicule v = vehicules_qui_peuvent[i];
        // Duree aller et retour vers point de recharge
        int heure_arrivee = heure + duree_min(reseau, v.position, appel.origin) + duree_min(reseau, appel.origin, appel.destination);
        if (heure_arrivee < heure_min)
        {
            heure_min = heure_arrivee;
            v_best = v;
        }
    }

    // En cas de parité, celui qui arrive le plus tard au point d’origine est choisi
    for (int i = 0; i < nb_peuvent; i++)
    {
        Vehicule v = vehicules_qui_peuvent[i];
        // Duree aller et retour vers point de recharge
        int heure_arrivee = heure + duree_min(reseau, v.position, appel.origin) + duree_min(reseau, appel.origin, appel.destination);
        if (heure_arrivee == heure_min || duree_min(reseau, v.position, appel.origin) > duree_min(reseau, v_best.position, appel.origin))
        {
            v_best = v;
        }
    }

    return v_best;
}

/************************ Gestion des differents evenements  **********************************/
/*Ici nous avons choisi les files pour la gestion des evenements, sachant que nous avons 4    */
/*evenements et que un evenement peut engendrer un autre. Donc on a commencer par definir les */
/*fonctions de base de gestion de files : enfiler, defiler, etc                               */
/**********************************************************************************************/
Event *create_event(int time, enum EventType type, int vehicule, Call *appel)
{
    Event *event = (Event *)malloc(sizeof(Event));

    event->time = time;
    event->type = type;
    event->vehicule = vehicule;
    event->appel = appel;
    event->next = NULL;
    event->prev = NULL;
    return event;
}

Event *enfiler_event(Event *events, Event *event)
{
    // Cas où la file est vide
    if (events == NULL)
    {
        event->next = NULL;
        event->prev = NULL;
        return event;
    }

    // Cas où le nouvel événement doit être inséré en tête de file
    if (event->time < events->time)
    {
        event->next = events;
        events->prev = event;
        event->prev = NULL;
        return event;
    }

    // Parcours de la file pour trouver l'emplacement d'insertion
    Event *current = events;
    while (current->next != NULL && (event->time > current->next->time ||
                                     (event->time == current->next->time && event->type > current->next->type)))
    {
        current = current->next;
    }

    // Insertion du nouvel événement dans la file
    event->next = current->next;
    event->prev = current;
    if (current->next != NULL)
    {
        current->next->prev = event;
    }
    current->next = event;

    return events;
}

Event *defiler_event(Event **events)
{
    if (*events == NULL)
    {
        return NULL; // La file est vide, rien à retirer
    }

    Event *firstEvent = *events;
    *events = (*events)->next;

    if (*events != NULL)
    {
        (*events)->prev = NULL;
    }

    firstEvent->next = NULL;
    firstEvent->prev = NULL;

    return firstEvent; // Retourner l'événement défiler
}

Event *init_call_events(Event *events, Call *appels, int nb_appel)
{

    for (int i = 0; i < nb_appel; i++)
    {
        Event *evt = create_event(appels[i].time, CALL, 0, &(appels[i]));

        events = enfiler_event(events, evt);
    }
    return events;
}

const char *event_type_tostring(enum EventType type)
{
    switch (type)
    {
    case CALL:
        return "CALL";
    case END_SERVICE:
        return "END_SERVICE";
    case RETURN_CHARGE:
        return "RETURN_CHARGE";
    case END_CHARGE:
        return "END_CHARGE";
    default:
        return "UNKNOWN";
    }
}

void afficher_events(Event *events)
{
    Event *current = events;

    printf("\nEvénements :\n");
    while (current != NULL)
    {
        printf("%d %s %d", current->time, event_type_tostring(current->type), current->vehicule);

        if (current->type == CALL || current->type == END_SERVICE)
        {
            printf(" %s", current->appel->name);
        }
        printf("\n");
        current = current->next;
    }
}
/*********************** Gestion de la file d'attente de recharge ***************************/
int *init_file_attente_recharge(int nb_vehicules)
{
    int *file = (int *)malloc(sizeof(int) * nb_vehicules);

    if (file == NULL)
    {
        printf("Erreur d'allocation mémoire\n");
        return NULL;
    }
    else
    {
        for (int i = 0; i < nb_vehicules; i++)
        {
            file[i] = -1;
        }
        return file;
    }
}

int get_debut_chargement(int *file_attente, int nb_vehicules)
{
    int fin_max = -1;
    for (int i = 0; i < nb_vehicules; i++)
    {
        if (file_attente[i] > fin_max)
        {
            fin_max = file_attente[i];
        }
    }

    return fin_max;
}

/**********************  Surestimation du gain  *********************************************/
int surestimation_gain(Reseau reseau, Event *events, CaracteristiquesVehicules crt_vehicules)
{
    int capacite = crt_vehicules.nbVehicules * crt_vehicules.intervalleService, cap = 0, cap_t = 0, gain = 0, gain_t = 0;

    Event *event = events;
    cap = duree_min(reseau, (*event->appel).origin, (*event->appel).destination);
    gain = cap + (*event->appel).prime;

    while (cap_t + cap <= capacite && event != NULL)
    {
        
        gain_t += gain;
        cap_t += cap;

        event = event->next;
        if (event != NULL)
        {
            cap = duree_min(reseau, (*event->appel).origin, (*event->appel).destination);
            gain = cap + (*event->appel).prime;
        }
    }

    return gain_t;
}




