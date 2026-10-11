# Genos2 Personnel

Cette version personnelle de Cadenza démarre sur un instrument modélisé dans Blender.
La source actuelle est le fichier `Yamaha_Genos2.blend` fourni par Marco dans son
dossier de références Yamaha Genos 2. Une copie, l'export natif reproductible et
les textures sont dans `assets/genos2/reference-model` ; l'ancien modèle dans
`assets/genos2` est conservé comme historique. Sa géométrie triangulée et ses
textures sont embarquées et projetées avec un tampon de profondeur partagé par
le rendu et les clics. Les deux écrans inclinés contiennent l'état réel de Cadenza
à la place des photographies d'interface du modèle ; les fonctions
musicales et les sons sont ceux du moteur Cadenza et des instruments chargés.

## Utilisation

- Cliquer sur les touches pour jouer ; glisser sur le clavier pour un glissando.
- Le clavier d'ordinateur joue des notes avec Q S D F G H J K ; W X C V B N fournit
  une octave plus basse pour la zone d'accords. Espace commande la lecture ; Échap
  libère les notes tenues à l'écran.
- Les boutons de variation utilisent les sections réellement disponibles dans le
  style chargé. Les sections absentes sont assombries et ne sont pas déclenchées.
- Glisser les curseurs pour régler les parties ; le joystick à gauche commande
  pitch bend horizontalement et modulation verticalement. Le pitch bend revient
  au centre au relâchement et à la perte de focus.
- Glisser le fond pour orienter le modèle et utiliser la molette pour zoomer. Les
  boutons Vue de dessus et Vue 3D recentrent l'instrument.
- Style Creator ouvre l'éditeur existant de Cadenza avec une session éditable.
  L'édition se fait dans cet atelier ; cette version n'implémente pas tous les
  paramètres du Style Creator Yamaha ni un export SFF2 entièrement validé.
- Vue Cadenza affiche l'interface originale. Vue Genos2 3D revient à l'instrument.
- La fenêtre reste redimensionnable à partir de 900 × 650 pixels pour garder
  tous les accès essentiels visibles.

Le rendu de la façade utilise une image interne d'au moins 3840 pixels de large
aux tailles habituelles, avec suréchantillonnage et filtrage des contours. Il
s'adapte à la densité de pixels Windows, y compris un affichage 4K à 200 %, et
les écrans Cadenza sont dessinés à leur résolution projetée. Les captures 4K
fournies mesurent 3840 × 2160 pixels. La fenêtre conserve une taille adaptée à
l'écran ; un écran de résolution inférieure affiche une version réduite du
rendu. Les sérigraphies photographiques gardent leur définition source de
2000 × 2000, sans détails supplémentaires inventés.

Les réglages sont conservés dans un profil local distinct, Genos2Personnel. Les
réglages et journaux d'une installation Cadenza existante ne sont pas remplacés.
Les touches sont relâchées lors d'un changement de vue, d'une perte de focus ou de
la fermeture du composant.

Les registrations, multipads, encodeur DATA et certaines petites commandes du
clavier représenté restent des détails visuels : seules les fonctions existantes
de Cadenza sont raccordées aux commandes correspondantes. Les six encodeurs
commandent REV, COMP, VOL, LOW, MID, HIGH et les neuf curseurs les huit parties
puis le volume maître. L'écran LIVE CONTROL décrit ces affectations Cadenza.
Les fonctions sans bouton physique équivalent restent accessibles dans la vue
Cadenza et la barre d'outils (éditeur, MIDI, audio, changement de vue).
Le bandeau français conserve les aides et états lisibles
lorsque les légendes physiques deviennent petites. Le rendu Blender est une
reconstruction de façade, pas une reproduction certifiée de l'instrument.

## Vérification

Les tests de géométrie vérifient les 76 touches, leur tessiture, la projection et
le repérage à plusieurs angles. Les tests d'interaction vérifient les appels des
commandes hôtes et la libération des notes. La qualité sonore et la compatibilité
avec un Genos2 physique doivent être contrôlées séparément.

L'application reste fondée sur Cadenza, sous GPLv3 ; elle n'est pas une application
officielle Yamaha et ne comprend pas les banques de sons propriétaires du Genos2.
