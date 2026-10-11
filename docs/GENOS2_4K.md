# Rendu UHD de la façade — 11 octobre 2026

Demande de Marco : une résolution 4K, le rendu précédent étant trop bas.
Direction conservée : modèle Yamaha_Genos2.blend et marquages fournis, disposition
native et commandes existantes. Cette correction concerne la résolution et le
lissage, sans nouvelle direction graphique. AGI/direction-artistique restent
appliqués avec les exceptions JUCE détaillées dans GENOS2_REFERENCE_UIUX.md.

## Résolution réelle et DPI

Le raster précédent suivait les dimensions logiques de la fenêtre. À 1280×800,
la scène était calculée à1256×596 puis agrandie par Windows sur un écran dense.
Le nouveau raster calcule la géométrie à une largeur minimale de3840 pixels aux
tailles desktop usuelles, avec au moins deux échantillons par pixel physique
quand la taille permet de rester dans l'enveloppe7680×4320. Cette limite commune
borne les buffers couleur, profondeur et commandes ; le logiciel reste un
raster CPU et ne garantit pas une cadence musicale à4K.

La projection reste dans le repère logique original, puis les points sont
transformés vers le raster. Le dessin filtré et le picking utilisent exactement
la même transformation ; les coordonnées de commandes ne sont pas doublées par
erreur avec le DPI. Le facteur provient du contexte graphique JUCE et un changement
de facteur invalide le cache.

Les deux LCD sont recréés à au moins deux fois leur grille600×310/600×64 et
s'adaptent à leur taille projetée. Textes, jauges et états sont donc rendus à
nouveau, plutôt qu'agrandis depuis une vieille capture. Les textures2000×2000
du modèle source restent intactes et bornent la précision de ses sérigraphies.
Il n'y a aucun upscale IA ni modification du fichier Blender du Bureau.

## Vérifications prévues dans la CI Windows

Captures réelles du composant : `genos2-4k.png` et `genos2-4k-top.png`, chacune
3840×2160. `genos2-4k-hidpi.png` capture une fenêtre logique1920×1080 avec un
facteur physique2. Les tests contrôlent la taille des buffers, le retour du DPI2
au DPI1 et le picking de toutes les122 commandes, en plus des trois tailles
existantes. Les chronos4K forcent un état LCD différent pour inclure un calcul
complet de géométrie et distinguer les simples copies de cache.

La revue indépendante doit ouvrir les fichiers à leur taille native, comparer
LCD et diagonales avec620a3e4, contrôler le layout entier et nommer les limites
des textures originales. Un PNG4K ne prouve pas une fenêtre physique4K sur un
écran de résolution inférieure ; revue native, audio et tests Yamaha sont des
preuves distinctes. Résultats définitifs consignés dans le rapport versionné
sous `outputs/GENOS2-4K-2026-10-11/` après compilation et revue.
