# Genos2 Personnel

Cette version personnelle de Cadenza démarre sur un instrument modélisé dans Blender.
La scène source et les rendus sont dans `assets/genos2`. Sa géométrie triangulée
est embarquée dans l'application et projetée avec un tampon de profondeur ;
l'écran contient l'état réel de Cadenza. Elle reprend les proportions et groupes
de commandes des planches locales Genos2 ; les fonctions
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

Les réglages sont conservés dans un profil local distinct, Genos2Personnel. Les
réglages et journaux d'une installation Cadenza existante ne sont pas remplacés.
Les touches sont relâchées lors d'un changement de vue, d'une perte de focus ou de
la fermeture du composant.

Les registrations, multipads, encodeur DATA et certaines petites commandes du
clavier représenté restent des détails visuels : seules les fonctions existantes
de Cadenza sont raccordées. Le bandeau français conserve les aides et états lisibles
lorsque les légendes physiques deviennent petites. Le rendu Blender est une
reconstruction de façade, pas une reproduction certifiée de l'instrument.

## Vérification

Les tests de géométrie vérifient les 76 touches, leur tessiture, la projection et
le repérage à plusieurs angles. Les tests d'interaction vérifient les appels des
commandes hôtes et la libération des notes. La qualité sonore et la compatibilité
avec un Genos2 physique doivent être contrôlées séparément.

L'application reste fondée sur Cadenza, sous GPLv3 ; elle n'est pas une application
officielle Yamaha et ne comprend pas les banques de sons propriétaires du Genos2.
