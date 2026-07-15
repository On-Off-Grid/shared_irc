# ft_irc — Internet Relay Chat

**Résumé :** Ce projet consiste à créer votre propre serveur IRC.
Vous utiliserez un vrai client IRC pour vous connecter à votre serveur et le tester.
L’Internet est régi par des protocoles standards solides qui permettent aux ordinateurs connectés d’interagir.
Comprendre ces protocoles est toujours bénéfique.

**Version :** 9.1

## I. Introduction
Internet Relay Chat (IRC) est un protocole de communication textuel sur Internet.
Il offre une messagerie en temps réel, publique ou privée. Les utilisateurs peuvent échanger des messages directs et rejoindre des canaux de groupe.
Les clients IRC se connectent aux serveurs IRC pour rejoindre des canaux. Les serveurs IRC sont interconnectés pour former un réseau.

## II. Règles générales
- Votre programme ne doit jamais crasher (même en cas de manque de mémoire) et ne doit pas quitter de manière inattendue.
  Si cela arrive, votre projet sera considéré comme non fonctionnel et votre note sera 0.
- Vous devez rendre un **Makefile** qui compile vos fichiers sources. Il ne doit pas relinker inutilement.
- Votre Makefile doit contenir au minimum les règles : `$(NAME)`, `all`, `clean`, `fclean`, `re`.
- Compilez votre code avec `c++` en utilisant les flags `-Wall -Wextra -Werror`.
- Votre code doit respecter le standard **C++98**. Il doit aussi compiler si vous ajoutez le flag `-std=c++98`.
- Essayez toujours d’utiliser les fonctionnalités C++ quand elles existent (par exemple, privilégiez `<cstring>` à `<string.h>`).
  Les fonctions C sont autorisées, mais préférez systématiquement leurs versions C++ si possible.
- Toute bibliothèque externe et Boost sont **interdites**.

## III. Instructions IA
### Contexte
Pendant votre apprentissage, l’IA peut vous aider sur de nombreuses tâches. Prenez le temps d’explorer les capacités des outils IA et la manière dont ils peuvent vous soutenir.
Cependant, restez prudent et évaluez les résultats de manière critique. Qu’il s’agisse de code, de documentation, d’idées ou d’explications techniques, vous ne pouvez jamais être totalement sûr que la question était bien posée ou que le contenu généré est exact.
Vos pairs sont une ressource précieuse pour éviter erreurs et angles morts.

### Message principal
- Utilisez l’IA pour réduire les tâches répétitives ou pénibles.
- Développez vos compétences de prompt — techniques et non techniques — utiles pour votre carrière.
- Apprenez comment fonctionnent les systèmes d’IA pour mieux anticiper et éviter les risques, biais et enjeux éthiques.
- Continuez à développer vos compétences techniques et relationnelles avec vos pairs.
- N’utilisez que du contenu généré par l’IA que vous comprenez pleinement et dont vous pouvez assumer la responsabilité.

### Règles pour l’apprenant
- Prenez le temps d’explorer les outils IA et de comprendre leur fonctionnement afin de les utiliser de manière éthique et de réduire les biais potentiels.
- Réfléchissez au problème avant de prompter — cela aide à écrire des prompts plus clairs, détaillés et pertinents, avec le vocabulaire approprié.
- Développez l’habitude de vérifier, relire, questionner et tester tout ce qui est généré.
- Cherchez toujours une relecture par vos pairs — ne vous fiez pas uniquement à votre propre validation.

### Résultats de phase
- Développer des compétences de prompt générales et spécifiques au domaine.
- Améliorer votre productivité avec un usage efficace des outils IA.
- Renforcer la pensée computationnelle, la résolution de problèmes, l’adaptabilité et la collaboration.

### Commentaires et exemples
- Vous rencontrerez régulièrement des situations (examens, évaluations, etc.) où vous devez démontrer une vraie compréhension.
- Expliquer votre raisonnement et débattre avec vos pairs révèle souvent des lacunes. Faites de l’apprentissage entre pairs une priorité.
- Les outils IA manquent souvent de contexte et produisent des réponses génériques. Vos pairs, qui partagent votre environnement, offrent des retours plus pertinents.
- Là où l’IA propose la réponse la plus probable, vos pairs apportent des alternatives et des nuances utiles.

**Bonne pratique :**
> Je demande à l’IA : « Comment tester une fonction de tri ? » Elle me donne quelques idées. Je les essaye et je revois les résultats avec un pair. On affine l’approche ensemble.

**Mauvaise pratique :**
> Je demande à l’IA d’écrire toute une fonction, je la copie-colle. En évaluation, je ne peux pas expliquer ce qu’elle fait ni pourquoi. Je perds en crédibilité et je rate le projet.

**Bonne pratique :**
> J’utilise l’IA pour aider à concevoir un parseur. Puis je passe en revue la logique avec un pair. On repère deux bugs et on réécrit ensemble — mieux, plus propre, et compris.

**Mauvaise pratique :**
> Je laisse Copilot générer du code pour une partie clé de mon projet. Ça compile, mais je ne peux pas expliquer comment il gère les pipes. À l’évaluation, je n’arrive pas à justifier et je rate.

## IV. Partie obligatoire
**Nom du programme :** `ircserv`

**Fichiers à rendre :** `Makefile`, `*.{h, hpp}`, `*.cpp`, `*.tpp`, `*.ipp`, et un fichier de configuration optionnel

**Makefile :** `NAME`, `all`, `clean`, `fclean`, `re`

**Arguments :**
- `port` : le port d’écoute
- `password` : mot de passe de connexion

**Fonctions externes autorisées :** (C++98 uniquement)
`socket`, `close`, `setsockopt`, `getsockname`,
`getprotobyname`, `gethostbyname`, `getaddrinfo`,
`freeaddrinfo`, `bind`, `connect`, `listen`, `accept`,
`htons`, `htonl`, `ntohs`, `ntohl`, `inet_addr`, `inet_ntoa`,
`inet_ntop`, `send`, `recv`, `signal`, `sigaction`,
`sigemptyset`, `sigfillset`, `sigaddset`, `sigdelset`,
`sigismember`, `lseek`, `fstat`, `fcntl`, `poll` (ou équivalent)

**Libft autorisée :** n/a

**Description :** Un serveur IRC en C++98

Vous devez développer un serveur IRC en C++98.
Vous ne devez **pas** développer de client IRC.
Vous ne devez **pas** implémenter la communication serveur-à-serveur.

Votre exécutable sera lancé ainsi :
`./ircserv <port> <password>`

- `port` : numéro de port sur lequel votre serveur écoute les connexions IRC.
- `password` : mot de passe requis par tout client IRC qui tente de se connecter.

> Même si `poll()` est mentionné dans le sujet et l’échelle d’évaluation, vous pouvez utiliser un équivalent comme `select()`, `kqueue()` ou `epoll()`.

### IV.1. Exigences
- Le serveur doit gérer plusieurs clients simultanément sans bloquer.
- **Fork interdit.** Tous les I/O doivent être **non bloquants**.
- **Un seul `poll()` (ou équivalent)** peut être utilisé pour toutes les opérations (lecture, écriture, écoute, etc.).

Comme vous utilisez des descripteurs non bloquants, il est possible d’appeler `read/recv` ou `write/send` sans `poll()` (ou équivalent) et votre serveur ne bloquera pas.
Cependant, cela consomme plus de ressources.
**Donc, si vous tentez de lire/écrire sur un FD sans `poll()` (ou équivalent), votre note sera 0.**

- Plusieurs clients IRC existent. Vous devez en choisir un comme **client de référence**.
- Votre client de référence doit pouvoir se connecter à votre serveur sans erreur.
- La communication client/serveur doit se faire via **TCP/IP (v4 ou v6)**.
- Utiliser votre client de référence avec votre serveur doit être similaire à un serveur IRC officiel. Cependant, vous n’avez à implémenter que :
  - Authentification, pseudo, username, rejoindre un canal, messages privés.
  - Tout message envoyé à un canal doit être relayé à tous les clients du canal.
  - Opérateurs et utilisateurs classiques.
  - Commandes spécifiques aux opérateurs :
    - `KICK` : éjecter un client d’un canal
    - `INVITE` : inviter un client dans un canal
    - `TOPIC` : changer ou voir le topic
    - `MODE` : changer le mode du canal :
      - `i` : canal sur invitation
      - `t` : réserver `TOPIC` aux opérateurs
      - `k` : mot de passe du canal
      - `o` : donner/retirer le statut opérateur
      - `l` : limite d’utilisateurs

- Vous êtes bien sûr attendu d’écrire un code propre.

### IV.2. MacOS uniquement
Comme MacOS n’implémente pas `write()` de la même manière que d’autres Unix, vous êtes autorisé à utiliser `fcntl()`.
Vous devez mettre les FDs en **non‑blocking** pour un comportement équivalent aux autres OS.
Vous êtes autorisé à utiliser `fcntl()` **uniquement** ainsi :
`fcntl(fd, F_SETFL, O_NONBLOCK);`
Tout autre flag est interdit.

### IV.3. Exemple de test
Vérifiez toutes les erreurs possibles (données partielles, faible bande passante, etc.).
Pour garantir que votre serveur traite correctement toutes les données, vous pouvez utiliser ce test simple avec `nc` :

```
$> nc -C 127.0.0.1 6667
com^Dman^Dd
$>
```

Utilisez `Ctrl+D` pour envoyer la commande en plusieurs parties : `com`, puis `man`, puis `d\n`.
Pour traiter une commande, vous devez d’abord **agréger les paquets reçus** afin de la reconstruire.

## V. Bonus
Fonctionnalités additionnelles possibles :
- Transfert de fichiers.
- Un bot.

La partie bonus sera évaluée **uniquement si la partie obligatoire est parfaite**.
Parfait signifie que la partie obligatoire est intégralement réalisée et fonctionne sans dysfonctionnement.
Si vous n’avez pas validé toutes les exigences obligatoires, le bonus ne sera pas évalué.

## VI. Rendu et évaluation par les pairs
Rendez votre travail sur votre dépôt Git comme d’habitude. Seul ce qui est dans le dépôt sera évalué.
Vérifiez bien les noms de fichiers.

Vous êtes encouragé à créer des programmes de test (non rendus ni notés) : ils peuvent être très utiles pendant la soutenance, ainsi que pour évaluer le projet d’un pair.
Vous êtes libres d’utiliser tous les tests que vous voulez pendant l’évaluation.

Votre client de référence sera utilisé lors de l’évaluation.

Pendant l’évaluation, on pourra parfois vous demander une petite modification (changement de comportement, quelques lignes à écrire/réécrire, ou une fonctionnalité facile à ajouter).
Cette étape sert à vérifier votre compréhension réelle d’une partie du projet.
La modification peut être faite dans l’environnement que vous voulez et doit être faisable en quelques minutes, sauf délai spécifique indiqué dans les guidelines.

Vous pouvez, par exemple, devoir mettre à jour une fonction, modifier un affichage, ou ajuster une structure de données pour stocker de nouvelles informations, etc.
Les détails (portée, cible, etc.) seront précisés dans les guidelines d’évaluation et peuvent varier selon l’évaluation.
