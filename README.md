Scan Processing
===============


## Traitement d'image de scans avec OpenCV

Prérequis :

```bash
  sudo apt-get install cmake libopencv-dev libcv-dev
```

Instruction de compilation :

```bash
  cmake .
  make [mon-exe]
  ./bin/mon-exe [argv]
```

## Détection de rectangles

```bash
  cmake .
  make detect_rectangles
  ./bin/detect_rectangles [chemin de l'image] [largeur du rectangle] [hauteur du rectangle] [nombre de rectangles à trouver]
```
Ajouter `-d` en dernier argument pour activer le mode débug

exemple avec le mode debug :

```bash
  ./bin/detect_rectangles velvet_KRYS\&VOUS_20150924.TXT_10002363.jpg 19 19 72 -d
```
Résultat : 

```bash
  (163,627)->43
  Attention : 2 rectangles n'ont pas été trouvé.
```

`(163,627)` : coordonnées du sommet en haut à gauche du rectangle

`43` : nombre de pixel noir

exemple sans le mode debug :

```bash
  ./bin/detect_rectangles velvet_KRYS\&VOUS_20150924.TXT_10002363.jpg 19 19 72
```
Résultat : 

```bash
  163,627,43|125,325,12|
```
