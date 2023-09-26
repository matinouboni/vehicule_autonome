#include <stdio.h>
#include <stdlib.h>
#include "structure.h"

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Utilisation : ./programme fichier_reseau fichier_vehicules fichier_appels\n");
        return 1;
    }

    const char *fichier_reseau = argv[1];
    const char *fichier_vehicules = argv[2];
    const char *fichier_appels = argv[3];

    Reseau reseau;

    CaracteristiquesVehicules crt_vehicules;
    Vehicule *vehicules;
    Call *appels;
    int nombre_appels;
    Voyage *voyages;
    Event *events = NULL;        // File d'events
    Event *events_traite = NULL; // events déjà traité

    int heure = 0;       // L'heure actuelle en seconde
    int temps_total = 0; // De deplacement des vehicules
    int gain = 0;        // Le gain
    int nb_appels_rejetes = 0; 
    int nb_recharges = 0;
    int ub = 0;

/****************Recuperation des données******************/
    if (!lire_fichier_reseau(fichier_reseau, &reseau))
    {
        printf("Erreur lors de la lecture du fichier réseau\n");
        return 1;
    }

    if (!lire_fichier_vehicules(fichier_vehicules, &crt_vehicules))
    {
        printf("Erreur lors de la lecture du fichier des véhicules\n");
        free(reseau.rue);
        return 1;
    }

    if (!lire_fichier_appels(fichier_appels, &appels, &nombre_appels))
    {
        printf("Erreur lors de la lecture du fichier des appels\n");
        free(reseau.rue);
        return 1;
    }
    // Clients
    trier_clients(appels, nombre_appels);
    afficher_clients(appels, nombre_appels);

    // Voyages
    calculer_voyages(reseau, appels, &voyages, nombre_appels);
    trier_voyages(voyages, nombre_appels);
    afficher_voyages(voyages, nombre_appels);

    // Positions intiales vehicules
    definir_positions_initiales_vehicules(reseau, crt_vehicules, &vehicules);
    afficher_positions_vehicules(crt_vehicules, vehicules);

    // Etats et charges
    init_etats_charges(crt_vehicules, vehicules);
    printf("\n*************** Etat initial du systeme ****************\n");
    afficher_vehicules(vehicules, crt_vehicules.nbVehicules);

    // Test choix vehicule
    // Call appel = {866, "Mancini", 4, 10, 1056, 5767, 2915};
    // int nb_pv;
    // Vehicule *vhcs = vehicules_qui_peuvent_servir(reseau, appel, crt_vehicules, vehicules, &nb_pv);
    // printf("\nQui peuvent servir Mancini:");
    // afficher_vehicules(vhcs, nb_pv);
    // Vehicule vb = choisir_meilleur_vehicule(reseau, appel, vhcs, nb_pv);
    // printf("\nLa meilleur choix");
    // printf("\nVehicule %d, position : %d\n", vb.numero, vb.position);

    // test dijkstra
    // int org = 0, dst = 8, duree, *noeuds = (int *)malloc(reseau.nombre_noeuds * sizeof(int));
    // if (trouver_trajet_dijkstra(reseau, org, dst, &duree, noeuds))
    // {

    //     printf("Chemin optimal entre les nœuds %d et %d : ", org, dst);
    //     for (int i = 0; i < reseau.nombre_noeuds; i++)
    //     {
    //         if (noeuds[i] != -1)
    //             printf("%d ", noeuds[i]);
    //     }

    //     printf("\nDurée : %d\n", duree);
    // }
    //////

/**********Rechargement des appels dans la file d'evenements*************/
    events = init_call_events(events, appels, nombre_appels);

    // Surestimation du gain
    ub = surestimation_gain(reseau, events, crt_vehicules);
    /*
        Un tableau de taille nombre de vehicules pour gerer file d'attente de recharge
         - vehicule n n'est pas en charge ou dans la file d' attente => -1 à la position n-1
         - sinon on met son temps de fin de recharge à la position n-1
    */
    int *file_attente_recharge = init_file_attente_recharge(crt_vehicules.nbVehicules);
    if (file_attente_recharge == NULL)
    {
        printf("Erreur de création file d'attente de rechargement\n");
        return 1;
    }

    /************************* Simulation *********************************/
    /*cette section gère la simulation. Nous allons gerer les evenements  */
    /*on met a jour aussi la file pour prendre en compte les evenements   */
    /*engendré                                                            */
    /**********************************************************************/

    printf("\n********************* Simulation *********************\n");
    while (events != NULL)
    {

        // On defile un event
        Event *event = defiler_event(&events);

        if (event->type == CALL && event->time > crt_vehicules.intervalleService)
        {
            nb_appels_rejetes++;
        }
        else
        {

            heure = event->time;

            // On traite l'event
            switch (event->type)
            {
            case CALL:
                int nb_pv;
                Vehicule *vhcs = vehicules_qui_peuvent_servir(reseau, *event->appel, crt_vehicules, vehicules, &nb_pv);
                if (nb_pv == 0)
                {
                    // Appel rejeté
                    nb_appels_rejetes++;
                }
                else
                {
                    Vehicule vbest = choisir_meilleur_vehicule(reseau, *event->appel, vhcs, nb_pv);
                    // event->vehicule = vbest.numero;

                    int prix = 0;
                    // Duree aller à destination
                    int duree = duree_min(reseau, vbest.position, (*event->appel).origin) + duree_min(reseau, (*event->appel).origin, (*event->appel).destination);

                    // Ajouter le temps d'attente à l'origine s'il y a eu lieu
                    int duree_t = duree;
                    if (heure + duree_min(reseau, vbest.position, (*event->appel).origin) <= (*event->appel).minTime)
                    {
                        duree_t += (*event->appel).minTime - (heure + duree_min(reseau, vbest.position, (*event->appel).origin));
                        // Ajout de la prime forfaitaire pour la ponctualité
                        prix += (*event->appel).prime;
                    }

                    // Le prix payé par le client est proportionnel (pour simplifier, égal) à la durée du trajet calculé en secondes
                    prix += duree_min(reseau, (*event->appel).origin, (*event->appel).destination);
                    gain += prix;

                    temps_total += duree_t;

                    // Mettre à hout l'état du vehicule
                    for (int i = 0; i < crt_vehicules.nbVehicules; i++)
                    {
                        if (vehicules[i].numero == vbest.numero)
                        {
                            vehicules[i].etat = OCCUPE;
                            vehicules[i].charge -= duree;
                        }
                    }

                    // Ajout du nouvel événement d’arrivée à destination est ajouté à la collection.
                    Event *new_event = create_event(heure + duree, END_SERVICE, vbest.numero, event->appel);
                    events = enfiler_event(events, new_event);

                    // printf("\nVehicule %d, position : %d\n", vbest.numero, vbest.position);
                }

                break;
            case END_SERVICE:
                // Mettre à jour état du vehicule

                for (int i = 0; i < crt_vehicules.nbVehicules; i++)
                {
                    if (vehicules[i].numero == event->vehicule)
                    {
                        // test de 20% pour la batterie
                        if ((double)vehicules[i].charge / crt_vehicules.autonomieBatterie >= 0.2) 
                        {
                            vehicules[i].etat = LIBRE;
                            vehicules[i].position = (*event->appel).destination;
                        }
                        else
                        {
                            // retour pour recharge
                            int duree_retour = duree_min(reseau, (*event->appel).destination, 1); // 1 = noeud de recharge
                            temps_total += duree_retour;
                            vehicules[i].charge -= duree_retour;
                            // Ajout du nouvel événement retour au depôt pour recharge à la collection.
                            Event *new_event = create_event(heure + duree_retour, RETURN_CHARGE, vehicules[i].numero, NULL);
                            events = enfiler_event(events, new_event);
                        }
                    }
                }

                break;
            case RETURN_CHARGE:
                nb_recharges++;
                for (int i = 0; i < crt_vehicules.nbVehicules; i++)
                {
                    if (vehicules[i].numero == event->vehicule)
                    {
                        vehicules[i].position = 1;
                        int qt_a_recharger = crt_vehicules.autonomieBatterie - vehicules[i].charge;
                        // c'est la règle de 3 qui est utilisée
                        int duree_recharge = (int)(qt_a_recharger * ((double)crt_vehicules.tempsRecharge / crt_vehicules.autonomieBatterie));

                        int debut = get_debut_chargement(file_attente_recharge, crt_vehicules.nbVehicules), fin;

                        if (debut == -1) // rien à la charge
                        {
                            fin = event->time + duree_recharge;
                        }
                        else
                        {
                            fin = debut + duree_recharge;
                        }

                        file_attente_recharge[event->vehicule - 1] = fin;

                        // Ajout du nouvel événement fin de rechargement à la collection.
                        Event *new_event = create_event(fin, END_CHARGE, vehicules[i].numero, NULL);
                        events = enfiler_event(events, new_event);
                    }
                }
                break;
            case END_CHARGE:
                for (int i = 0; i < crt_vehicules.nbVehicules; i++)
                {
                    if (vehicules[i].numero == event->vehicule)
                    {
                        vehicules[i].charge = crt_vehicules.autonomieBatterie;
                        vehicules[i].etat = LIBRE;
                        file_attente_recharge[vehicules[i].numero - 1] = -1;
                    }
                }
                break;
            default:
                break;
            }
        }

        // On enfile event traité
        events_traite = enfiler_event(events_traite, event);
        // On affiche l'états du système à chaque iteration 
        afficher_vehicules(vehicules, crt_vehicules.nbVehicules);

    } // fin while

    afficher_events(events_traite);
    printf("\nAppels non desservis: %d\nRecharges: %d\nTemp total: %d\nGain: %d\nUB: %d\n", nb_appels_rejetes, nb_recharges, temps_total, gain, ub);

    
    printf("\n***************** l'Etat final du systeme ***************\n");
    afficher_vehicules(vehicules, crt_vehicules.nbVehicules);
    free(vehicules);
    free(voyages);
    free(reseau.rue);
    free(appels);

    return 0;
}
