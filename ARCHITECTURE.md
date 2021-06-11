Rapport du projet tsh - Groupe 12
===

## Identifiants

DEPREZ Hugo - 71802209 - @deprez

MEBARKI Clement - 71800676 - @mebarkic

DE SOUSA LIMA Fabio - 71802806 - @desousal

## Fonctionnalités

Notre programme est un shell permettant de considérer les tarball comme des dossiers, sans pour autant les désarchiver. Il possède les fonctionnalités suivantes :

* `cd` fonctionne sans argument ou avec un chemin
* `exit` fonctionne sans argument
* les commandes externes fonctionnent toutes si aucun tarball n\'est en jeu
* `pwd` fonctionne y compris si le \"répertoire courant\" (au sens tsh) implique un tarball
* `mkdir`, `rmdir` et `mv` fonctionnent y compris avec des chemins impliquant des tarball quand ils sont utilisés sans option
* `cp` et `rm` fonctionnent y compris avec des chemins impliquant des tarball quand ils sont utilisés sans option ou avec l\'option `-r`
* `ls` fonctionne y compris avec des chemins impliquant des tarball quand il est utilisé sans option ou avec l\'option `-l`
* `cat` fonctionne y compris avec des chemins impliquant des tarball quand il est utilisé sans option
* les combinaisons de commandes avec `|` fonctionnent
* certaines commandes peuvent s\'exécuter avec ou sans argument
* quand une commande attend un chemin, celui-ci peut être absolu ou relatif, faire intervenir un tarball ou pas
* les commandes fonctionnent que le \"répertoire courant\" (au sens tsh) soit un vrai répertoire ou quelque part dans un tarball
* un utilisateur qui possède les droits en exécution sur tsh peut s\'en servir
* les commandes externes fonctionnent toutes si aucun tarball n\'est en jeu
* lors d\'une erreur de la part de l\'utilisateur, un message adéquat explicite le problème

Cependant, nous n\'avons pas su implémenter les fonctionnalités suivantes (elles sont réalisées mais non fonctionnelles) :

* les redirections de l\'entrée, de la sortie et de la sortie erreur quand elles impliquent des tarball

## Découpage

#### \"Commands/\"

Le dossier `commands/` regroupe les fichiers sources de la plupart des commandes que nous avons implémentées (à l\'exception de `cd` et `exit` qui sont dans `tarm.c`) :

* `cat.c` : fichier source de la commande cat
* `cp.c` : fichier source de la commande cp
* `ls.c` : fichier source de la commande ls
* `mkdir.c` : fichier source de la commande mkdir
* `mv.c` : fichier source de la commande mv
* `pwd.c` : fichier source de la commande pwd
* `rm.c` : fichier source de la commande rm
* `rmdir.c` : fichier source de la commande rmdir

Ce répertoire contient aussi un dossier `bin/` servant à stocker les commandes compilées (binaires). Ce dossier est créé à la compilation (`make`) et détruit, lui et son contenu, au nettoyage (`make clean`).

#### \"Libs/\"

Le dossier `libs/` regroupe les fichiers sources et les en-têtes de nos librairies :

* `copy_move.c` : regroupe les fonctions de copies et de déplacements (pour `mv`, `cp`, etc...)
* `copy_move.h` : comporte les en-têtes des fonctions de `copy_move.c`, la documentation de celles-ci 
* `tar.c` : regroupe les fonctions de parcours de tarball, d\'ajout d\'éléments dans des tarball, de suppression d\'éléments dans des tarball etc...
* `tar.h` : comporte la déclaration du `struct posix_header`, les en-têtes des fonctions de `tar.c` et la documentation de celles-ci
* `tarm.c` : correspond au `main` du projet : gère les concaténations de commandes, les redirections, le parsing de commandes etc...
* `utils.c` : regroupe les fonctions dont nous nous servons partout dans le projet comme la simplification d\'un chemin, l\'existence d\'un fichier, la récupération de certaines parties d\'un chemin etc...
* `utils.h` : comporte la déclaration de nos macros (`BUFSIZE` par exemple), les en-têtes des fonctions de `utils.c`, la documentation de celles-ci et l\'ensemble des `include` du projet

Ce répertoire contient lui aussi un dossier `bin/` servant à stocker les librairies compilées (`utils`, `tar` et `copy_move`). Ce dossier est également créé à la compilation (`make`) et détruit, lui et son contenu, au nettoyage (`make clean`).

#### Racine

À la racine du projet se trouve les fichiers :

* `ARCHITECTURE.md` : le rapport que vous êtes probablement en train de lire
* `AUTHORS` : spécifie les auteurs du projet, et leur identité (requis)
* `Dockerfile` : contient les instructions à fournir à Docker pour la création de l\'image Docker
* `Makefile` : correspond au script de compilation du projet
* `README.md` : explicite les instructions de compilation et d\'exécution du projet

#### Misc :

Le fichier `.dockerignore` permet de spécifier à Docker les fichiers à ne pas prendre en compte lors de la création de l\'image Docker.
Le fichier `.gitignore` permet de spécifier à Git les fichiers à ignorer.
Après la compilation, vous trouverez à la racine un fichier `tsh` : c\'est l\'exécutable du projet (pour le lancer, voir `README.md`).

## Retour du premier rendu

Du **`rendu1`** sont ressortis essentiellement trois points sur lesquels nous avons travaillé :

* le code n\'était **pas assez documenté** :
    * nous avons documenté, un peu à la manière d\'une _Javadoc_, les prototypes de nos fonctions dans chacun des .h en résumant ce que la fonction réalisait, et en détaillant les valeurs de retour et les paramètres attendus
    * nous n\'avons pas eu le temps de documenter précisément l\'emsemble du code, mais les lignes qui nous semblaient _difficiles_ à comprendre bénéficient de précisions
* certaines fonctions étaient **trop longues** :
    * ça a été le point le plus dur à travailler, et certaines de nos fonctions restent assez conséquentes, mais la majorité ont été drastiquement réduites
    * nous pouvons prendre le cas de la fonction `cd_util`, qui est passée de **140** lignes au **`rendu1`**, à **70**, soit moitié moins (en prenant en compte que c\'est une fonction assez lourde et complexe et que la réduire davantage aurait été moins optimisé en terme de mémoire)
* nous n\'utilisions **pas assez de fonctions auxiliaires** pour éviter les **redondances** :
    * nous avons créé un répertoire de librairies, ne regroupant que des fonctions auxiliaires dont nous nous servons dans plusieurs commandes, soit environ une quarantaine de fonctions réparties sur trois fichiers source
    * quand ces fonctions auxiliaires ne sont utilisées que pour une seule commande, nous les écrivons directement dans le fichier source de celle-ci
    * grâce à ces fonctions auxiliaires, certaines commandes ont ainsi pu être écrite en quelques lignes seulement : par exemple `pwd`, qui fait à peine **21** lignes, `main()` et `#include` compris
 
D\'autres points sont également ressortis de ce retour, comme l\'absence de `Makefile`, la réécriture de commande système (pour les cas hors tarball), et d\'autres points qui se rapportent plus ou moins aux trois principaux énoncés et détaillés plus haut : ces erreurs ont toutes été discutées puis corrigées peu de temps après la réception de votre retour sur notre **`rendu1`**.

## Stratégies et choix de développement

#### Main

Comme explicité précédement, notre `main` (le fichier `tarm.c`), s\'occupe du parsing des commandes, des combinaisons de commandes, des redirections et des commandes `cd` et `exit`.

La commande `cd` est un peu particulière, et fera l\'objet d\'un point particulier plus bas.

Il nous semblait évident que la commande `exit` devait être gérée par la boucle principale de notre shell, puisque cela consistait uniquement à arrêter celle-ci. Nous l\'avons donc implémentée comme une commande interne, dans la boucle principale du `main` (shell).

Pour le reste des commandes, on parcourt le dossier `commands/bin/` à la recherche d\'une commande du même nom : si la command est trouvée, on `exec` notre binaire avec les arguments (récupérés lors du parsing), sinon on envoie la ligne de commande au système qui se charge de tout.

#### Commandes

Ce sont nos commandes qui se chargent de vérifier si l\'on doit utiliser la commande système ou notre version (est-ce qu\'un tarball est en jeu), afin d\'alléger le `main`.

Les arguments sont simplifiés (on retire les potentiels `.` et `..` du chemin), puis on vérifie qu\'aucun tar n\'est en jeu.

Pour les commandes possédant des options (`-r` pour `cp` et `rm`, et `-l` pour `ls`), la vérification se fait également à ce moment là.

#### cd et le répertoire courant

Un des plus gros problèmes a été de gérer le chemin courant, le stocker, le mettre à jour et le transmettre aux commandes. Au début, nous avions opté pour une sorte de variable globale \"améliorée\" via un **wrapper**.

Seulement, nous nous sommes rendu compte que lorsque nous exécutions nos commandes, ces données n\'étaient pas transmises ! Nous avons alors cherché et discuté de plusieurs solutions : un fichier temporaire, une variable gobale, un paramètre en plus dans l\'`exec` etc...

Mais rien de tout cela n\'était très propre, et nous avons fini par opter pour une solution plus simple, et adoptée dans plusieurs implémentations de bash : une variable d\'environnement.

Celle-ci résout la majorité de nos problèmes : les variables d\'environnement sont transmises au processus fils lors d\'un `fork`, donc transmises lors de l\'exécution; elles sont légères, facilement manipulables et tant que la variable n\'est pas exportée (ce qui n\'est jamais le cas dans notre projet), celle-ci sera supprimée à la fermeture du programme.

Cependant, un processus fils ne peut pas modifier l\'environnement de son père, et donc notre variable d\'environnement ne doit être modifiée que par le `main`. Heureusement pour nous, la seule commande qui modifie la valeur du répertoire courant est `cd`, c\'est pourquoi nous l\'avons, tout comme `exit`, implémentée comme une commande interne, dans la boucle principale du `main` (shell).

Nous tenons à bien préciser que même si seul le père peut **modifier** la valeur de la variable d\'environnement, ses fils peuvent tout à fait y **accéder** et la modifier **localement** (pour leur processus).

#### Cas de non-unicité

Dans des tarball, il se peut que deux fichiers puissent avoir le même nom (nous précisons au passage que les cas de tarball imbriqués n\'étaient pas demandés et que nos commandes réagissent très mal à ceux-ci).

Dans le cas où un fichier est unique :

* si c\'est un dossier (ou un tarball), le \'/\' peut-être précisé ou non
* dans les autres cas, si le \'/\' est utilisé, nous considérons que c\'est une erreur

Dans le cas où un fichier n\'est pas unique :

* certaines commandes prennent le dossier (en particulier `mv`)
* certaines commandes regardent si le \'/\' est précisé, et prennent le fichier ou le dossier en conséquence

#### À améliorer

Plusieurs points restent tout de même améliorables :

* le code de certaines fonctions peut être raccourci
* l\'implémentation des redirections d\'entrée, de sortie et d\'erreur
* permettre l\'utilisation de \"~\" dans les chemins
* remplacer le contenu de la variable \"HOME\" par \"~\" dans l\'affichage du chemin courant du tsh 
* implémenter l\'utilisation de la touche \"Tab\" pour faciliter l\'écriture des commandes
* l\'utilisation d\'un historique (sous forme de pile par exemple) pour utiliser les flèches directionnelles
* l\'utilisation de la touche \"Enter\" pour `cat` sans argument
* la factorisation de certaines parties du code (les fonctions `cd_util_file()` ou `check_unicity()` par exemple)

## Misc

Quelques petites précisions afin de faciliter vos tests :

* l\'utilisation de la touche \"Tab\" lors de l\'écriture d\'une commande semble, dans certains cas, modifier certains caractères, ce qui fait que celles-ci pensent que le chemin qu\'on leur a fourni est invalide... Pour éviter ce genre de comportement, veillez à ne pas appuyer sur cette touche

* l\'utilisation de \"`~`\" dans un chemin n\'est pas implémentée

* pour utiliser `cat` sans argument, utilisez \"Ctrl + d\" au lieu de la touche \"Enter\" 
