# ft_irc

Implémentation d’un serveur IRC en C++98, conforme aux contraintes du projet 42.

## Objectif

Ce projet consiste à développer un serveur IRC minimal capable de :

- gérer plusieurs clients en simultané,
- traiter les connexions non bloquantes,
- parser les commandes IRC,
- appliquer une logique de canaux et de communication entre clients.

## Compilation

Prérequis :

- compilateur C++ compatible C++98,
- `make`.

Commandes :

```bash
make
```

Nettoyage :

```bash
make clean
make fclean
make re
```

## Exécution

```bash
./ircserv <port> <password>
```

Exemple :

```bash
./ircserv 6667 monpass
```

Contraintes validées par le programme :

- port numérique,
- port dans l’intervalle 1024–65535,
- mot de passe non vide.

## Fonctionnalités implémentées

- serveur TCP non bloquant,
- boucle d’événements avec `poll`,
- gestion des clients (connexion, déconnexion, nettoyage),
- bufferisation et extraction des lignes IRC,
- parsing des commandes IRC,
- dispatch des commandes côté serveur,
- gestion des canaux IRC.

### Commandes prises en charge

- `PASS`
- `NICK`
- `USER`
- `QUIT`
- `JOIN`
- `PART`
- `PRIVMSG`
- `NOTICE`
- `TOPIC`
- `KICK`
- `INVITE`
- `MODE`

## Structure du projet

- `includes/` : fichiers d’en-tête,
- `src/` : implémentation C++,
- `Makefile` : règles de compilation.

## Binaire généré

- `ircserv`

## Remarque

Le serveur est conçu pour être testé avec des clients IRC (ou via `nc`) selon les besoins du projet.
