Inscructions de lancement du projet tsh - Groupe 12
===

## Compilation et exécution

#### Docker

Le projet dispose d\'une image Docker personnalisée (`Dockerfile`) que vous pouvez utiliser en suivant les instructions ci-dessous :

0. Avant toute chose, vous devez installer Docker sur votre machine (pour plus d\'informations, rendez-vous [ici](https://docs.docker.com/docker-hub/))

Pour tester votre installation, vous pouvez entrer la commande (Si un message d\'erreur apparait, c\'est probablement que votre installation n\'est pas bonne) :
```
docker run hello-world
```

1. Ouvrez un terminal, placez vous dans la racine du projet, et tapez la commande
```
docker build -t img_projet .
```

2. Une fois que vous avez repris la main sur votre terminal, vous n\'avez plus qu\'à démarrer l\'image Docker avec la commande
```
docker run -ti img_projet
```

3. Une fois vos tests terminés, vous pouvez sortir du Docker en tapant simplement
```
exit
```

#### Compilation

Le projet dispose également d\'un `Makefile`, facilitant la compilation, et dont les détails d\'utilisation sont eux aussi spécifiés ci-dessous :

1. Pour compiler le projet, rien de plus simple : 
```
make
```

2. Le `Makefile` dispose également d\'une option de nettoyage :
```
make clean
```

#### Exécution

Une fois le projet compilé, vous pouvez l\'exécuter en tapant simplement :
```
./tsh
```
