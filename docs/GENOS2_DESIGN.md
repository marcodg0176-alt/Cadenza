# GENOS2 — direction et modèle Cadenza

11 octobre 2026. Designer et modeleur dédiés, intégration sous responsabilité du parent Cadenza.

## Usage et fidélité

Le musicien retrouve les zones du véritable GENOS2 et joue dans le moteur Cadenza existant. Les trois planches locales approuvées, `Yamaha-Genos2-von-oben.jpg`, `Yamaha-Genos2-Detail-5_1600y1200.jpg` et `Yamaha-Genos2-Detail-6_1600y1200.jpg`, sont les références visuelles. Elles restent externes au dépôt. Le modèle est reconstruit en géométrie, sans texture de photographie ni copie de l'écran Yamaha.

La silhouette prime : boîtier noir légèrement arrondi, large clavier clair de 76 touches E1–G7 (MIDI 28–103) occupant 84.5 % de la largeur, joystick unique à gauche, deux écrans distincts, six rotatifs et neuf curseurs à gauche de l'écran central, encodeur à droite, trois rangées compactes de commandes et les groupes style/registrations/OTS/parties au bord du clavier. L'écran central mesure environ 18.8 % de la largeur. Les petites fonctions Cadenza sont logées dans ces groupes ; le modèle ne prétend pas reproduire toutes les fonctions matérielles Yamaha.

## Composition et matériaux

Unités physiques normalisées : largeur 12.4, profondeur 4.45, x horizontal, y vertical, z profondeur, arrière négatif. Palette : corps #242A2D, panneaux #30373B, caoutchouc #101315, touches blanches #ECEBE5, LEDs bleu #4B9CEB et orange #FF9A30. Les boutons en fonction active ont une lueur légère ; pas de surfaces vitrées ajoutées au boîtier. Les flancs et le bord arrière portent des biseaux réels. Le modèle se lit en dessus et en perspective de face sans masquer les banques de commandes.

L'écran affiche une création Cadenza avec style, tempo, variation et volumes. Le contenu dynamique doit être dessiné par JUCE à l'intégration, sur le plan d'écran fourni. Les annotations portent des noms courts ASCII dans le modèle ; les aides françaises et accents se dessinent avec le système texte natif de JUCE.

## États et comportement à intégrer

Appui visible immédiatement ; clavier enfoncé pendant note-on, relâché sur note-off et perte de focus. Faders et rotatifs suivent la souris sans temporisation. Joystick : axes pitch/modulation, recentrage pitch au relâchement. Retour à l'interface native toujours accessible. Les petits boutons physiques nécessitent des aides lisibles et un accès clavier/toolbar ; leur miniaturisation fidèle ne suffit pas à l'accessibilité.

Apple Design est appliqué à la continuité spatiale et au retour immédiat des commandes, sans animation décorative : leurs positions ne bougent pas lors d'un changement d'état. UI UX Pro Max est appliqué au contraste des informations actives, aux aides et à la réserve d'un accès natif pour les fonctions denses. Ce livrable modélise les états visuels de repos ; les interactions effectives et l'accès clavier seront contrôlés dans l'application intégrée.

## Contrat portable

`assets/genos2/build_genos2.py` reconstruit la scène avec Blender 5.2 en instance indépendante. `genos2.blend`, `preview-perspective.png`, `preview-top.png` et `geometry.json` sont les sorties. JSON version 1 contient les commandes et les faces triangulées avec couleur ARGB, ID de commande ou chaîne vide, priorité de couche et orientation top. Les labels et surfaces d'écran sont séparés. Les faces correspondent à une vraie géométrie Blender, rendue par projection dans JUCE ; les PNG sont des preuves visuelles.

Les éléments physiques sans commande Cadenza sont explicitement décoratifs, avec un ID vide. Les commandes musicales utilisent les ID existants. Les registrations décoratives ne simulent aucune fonction non implémentée. La source ne contient ni secrets, ni profils utilisateur, ni dépendance Unity, ni image générée.

## Critères de revue

Comparer le dessus à la planche : largeur du clavier, proportions écran/corps, 76 touches, 9 faders, 6 rotatifs, un joystick et regroupements. Comparer la perspective : flancs arrondis, absence de masquage, lecture des états. Confirmer en application la projection, le redimensionnement, les notes appuyées/relâchées, style/play/variations, faders/rotatifs, éditeur et retour natif. Le rendu ne valide ni l'audio ni la connexion au clavier physique.

## Revue du modèle réel — 11 octobre 2026, 15:42 Europe/Paris

La scène a été reconstruite et rendue par Blender **5.2.1 LTS**, moteur Cycles, 24 échantillons avec débruitage. Les deux fichiers PNG ont été ouverts et examinés. Les premiers défauts (matières trop réfléchissantes, épaisseur du cadre masquant l'écran) ont été corrigés, puis les aperçus ont été rendus de nouveau. Le boîtier possède un rayon moulé réel ; les biseaux des touches et commandes sont de la géométrie.

| Élément | Planche de dessus | Modèle livré |
| --- | --- | --- |
| Clavier / largeur du corps | Environ 84 %, mesure visuelle | 10.48 / 12.4 = 84.5 % |
| Cadre d'écran / largeur du corps | Environ 18.8 % | 2.28 / 12.4 = 18.4 % |
| Profondeur des touches blanches / boîtier | Environ 32 % | 1.40 / 4.45 = 31.5 % |
| Touches | 76, E1–G7 | 76, MIDI 28–103 |
| Live Control | 6 rotatifs, 9 curseurs | 6 rotatifs, 9 curseurs |
| Contrôle pitch/modulation | Joystick gauche unique | Joystick gauche unique |

Export runtime : **11 620 triangles**, 130 commandes avec ID uniques. Les rails et LEDs des faders restent fixes (`moving:false`), mais portent l'ID de leur fader pour autoriser le clic sur toute la course ; la cap est mobile. Les labels sont exportés séparément. Les aperçus montrent les proportions, les groupes et la visibilité. Ils ne prouvent pas les interactions JUCE.

Limites de fidélité : approximation volontaire des petits groupes de boutons pour les 38 actions Cadenza disponibles ; la connectique arrière, le logo Yamaha et les commandes sans équivalent logiciel ne sont pas fonctionnels. L'encodeur, la molette master, les registrations et multipads représentés ont des noms `decor:*` et aucun ID de commande. Le rendu Blender reste plus éclairé que la photographie de référence ; le JSON conserve les couleurs sombres du matériau pour le rendu logiciel.

Preuves : `assets/genos2/preview-top.png` (1700 × 660), `preview-perspective.png` (1700 × 900), `genos2.blend`, source `build_genos2.py`, export `geometry.json`, provenance `provenance.json`. La reconstruction utilise une nouvelle instance CLI ; l'instance Blender interactive préexistante n'a pas été modifiée.
