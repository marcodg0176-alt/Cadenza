# GENOS2 — import du modèle final fourni

Direction du designer, 11 octobre 2026. Concept métier : jouer dans Cadenza en utilisant la géométrie détaillée du véritable instrument fournie par Marco, avec son écran Cadenza vivant et les commandes existantes.

## Références et aperçu avant intégration

1. Source approuvée : `C:/Users/marco/Desktop/AI/PLANCHES DE REFERENCES/Martial/Yamaha Genos 2/blender/Yamaha_Genos2.blend`, SHA256 `09D140A95059B1A05504D4BD60E96ECD581D2854F34729947407095E938ED553`. L'inventaire Blender indépendant confirme 801 meshes produit, 205 760 triangles après modificateurs, textures packed et 76 notes MIDI. Apport : silhouette galbée, écrans inclinés, touches profilées, sérigraphie, connectique et détail des commandes. Le fichier source reste intégralement inchangé ; le dépôt reçoit une copie binaire.
2. Les planches photographiques `Yamaha-Genos2-von-oben.jpg` et `Yamaha-Genos2-Detail-6_1600y1200.jpg`, déjà inspectées, fixent la comparaison en dessus et perspective. Apport : emplacement des groupes et densité ; elles ne deviennent pas une image de fond remplaçant la géométrie.

Les vrais rendus source `rendus/01_Perspective_hero.png`, `02_Dessus_orthographique.png` et `05_Detail_commandes.png` ont été ouverts et examinés avant export. Ils constituent l'aperçu approuvé pour cette importation, en remplacement de la reconstruction précédente du projet. Aucune nouvelle façade n'est reconstruite. Les photos/UV de marquages du modèle fourni sont préservés ; seul le contenu des deux LCD est remplacé au runtime par les informations Cadenza.

## Direction native et adaptation des guides

Variance 8 et mouvement 7 ont été interrogés par le générateur UIUX, mais la référence physique approuvée prime sur sa proposition générique de page Brutalism. Gardé : contraste, accès clavier, état lisible et retour immédiat des gestes. Rejeté : nouvelle palette rouge/crème, nouvelle police Righteous/Poppins, hero/CTA, stagger GSAP et rupture arbitraire de disposition ; ils altéreraient le modèle demandé.

Le panneau reste asymétrique par sa véritable disposition : commandes Live Control à gauche de l'écran incliné, grand clavier clair contrastant avec la coque sombre. Le logiciel conserve le texte natif JUCE hors de la texture, car cette application Windows n'utilise ni fontes web ni DOM. Le remplacement par Motion/React, la maquette HTML et les effets hero/scroll sont écartés : ils ajouteraient une autre interface alors que la demande porte sur l'import du modèle dans l'application native existante. Les mouvements utiles sont le clavier appuyé/relâché, les faders, la rotation des knobs et le joystick, tous pilotés par l'état musical ; caméra et zoom suivent les gestes. Aucun retard d'entrée ni animation autonome ne doit être ajouté.

Apport Apple/direct manipulation conservé : la reprise d'un fader ou joystick part de sa valeur courante. UIUX : les petites légendes physiques ne remplacent pas les aides lisibles et l'accès natif. Tester les tailles Windows 1280 × 800, 1600 × 900 et 900 × 650 ; une largeur mobile390 n'est pas une cible supportée par ce clavier desktop.

MCP gratuit 21st `get_usage` consulté : plan free, 2/2 récupérations code restantes, IA désactivée, crédit zéro. Aucun composant récupéré ni achat ; catalogue React sans apport utile à ce modèle JUCE. Aucun outil Motion/magicui pertinent exposé. Cette consultation ne vaut pas intégration de bibliothèque.

## Contrat d'import

Axes source mètres : x droite, y arrière, z haut. Axes runtime : `[x*10,z*10,-y*10]` ; largeur 12.34, profondeur4.56. Tous les modificateurs sont évalués ; aucune décimation globale ni suppression des détails ou des textures. Collections01–06 seulement ; studio et photographies de référence masquées exclus. Les deux dalles LCD originales (quatre triangles) sont remplacées par leurs quads dynamiques, aux coins3D exacts et avec un petit décalage normal pour la profondeur. La copie BLEND conserve les dalles et toutes les textures.

`geometry.json` contient `triangleResource:'mesh_bin'`, `textures:[{resource:'panel_png'},{resource:'rear_png'}]`, commandes, écrans et métadonnées. `mesh.bin` : signature ASCII `GNOS2M01`, count uint32 little endian, puis records `<15fIiiI` de76octets : neuf floats XYZ, six floats UV (u droite/v bas), couleurARGB de modulation, index commande (−1 décor), index texture (−1 solide), flags bit0=moving. PNG complets2000 × 2000 ; aucun recadrage des marquages.

Chaque commande expose un `hitPoint` réellement sur sa surface visible ; faders avec vecteur `travel` suivant la pente et `neutralValue` calculée sur la position source. Le repère et la cap bougent, rail/fente/LED restent fixes et pickables. Knobs avec pivot et axe vertical ; touches individuelles portent les MIDI source. Les meshes sans équivalent Cadenza restent décoratifs. Les réglages/imports/éditeur/retour natif continuent par la barre et l'interface native, sans attribuer une fausse fonction à une inscription Yamaha incompatible.

## Guides et traçabilité

- AGI v8, SHA256 `87CB1062D3EC803EDBDF097D2D87E98046AA637E392D9A682DC8659DDA9FD317` → périmètre natif séparé, source préservée.
- direction-artistique v1, SHA256 `CA94D02B33137B959888736FC44F03296D12B67353BAE6B74C1B4CE14FB40888` → aperçu source observé avant export, exceptions desktop explicites ci-dessus.
- UIUX, SHA256 `8B827FE03B349AEEA99A7D6CF4CCD892E6065B672859FDB11E13A1919453EDDC` → aides/continuité/contraste à contrôler dans le runtime. Script search SHA256 `8373E2DD2D560D9853EC116140DE0E0D5BEE45A6ABE5E041173C84E1C36A7F87`; flags vérifiés via `--help`.

Limites conservées de la source : reconstruction extérieure à partir de photographies, petites cotes et dessous estimés, éclairage déjà présent dans les textures. Un rendu/capture ne prouve ni audio, ni export, ni compatibilité du Yamaha physique. La nouvelle intégration exige sa propre revue du composant JUCE ; les 47 tests et captures du modèle précédent restent historiques.

## Export livré et contrôlé

Source et copie `assets/genos2/reference-model/Yamaha_Genos2.blend` ont le même SHA256 `09D140A95059B1A05504D4BD60E96ECD581D2854F34729947407095E938ED553`. Le modèle du Bureau n'a été ni réécrit ni modifié. Le script d'import s'exécute en instance Blender CLI isolée et fournit toutes les ressources locales ; son traitement ne reconstruit pas de nouvelles pièces.

BIN vérifié en lecture indépendante : signature `GNOS2M01`, 205 756 records, **15 637 468 octets**, exactement `12 + 76 × 205756`. Les 205 760 triangles du produit sont conservés à l'exception des quatre triangles des deux LCD remplacés au runtime. Les textures complètes `panel.png` et `rear.png`, 2000 × 2000, ont été ouvertes et inspectées ; les marquages sont présents. Les références et les rendus source copiés restent des preuves de direction, pas des captures du nouveau composant.

Les **122 commandes uniques** comprennent 76 touches, six rotatifs Live Control, un MASTER, neuf faders, un joystick et 29 boutons. Liste hors clavier : `knob0..5`, `master`, `fader0..8`, `pitch`, `style`, `ots0..3`, `left`, `right0..2`, `rec`, `acmp`, `autofill`, `break`, `endingA..C`, `introA..C`, `mainA..D`, `syncstop`, `play`, `tempo-`, `tempo+`, `transpose-`, `transpose+`. Les correspondances nominatives exactes sont dans `control-mapping.json`, et chaque tranche de faces dans `mesh-inventory.json`.

`accessOutsideModel` distingue les accès par barre (`creator`, `midi`, `audio`, `native`) et les fonctions restantes de l'interface native (`import`, `soundfont`, `save`, `fill`, `memory`). Les registrations, pads, commandes micro et autres boutons sans callback adapté restent décoratifs. Les rotatifs numérotés Live Control pilotent les six paramètres Cadenza déjà existants, indiqués par le LCD dynamique et les aides ; les noms physiques ne sont pas falsifiés.

Fader0 exemple : `neutralValue=0.9992000184`, `travel=[0,0.0185974822,-0.3799999692]` ; le point d'action se situe réellement sur la cap, pas au milieu du rail. Les quatre coins du main LCD portent des hauteurs runtime de1.504448 à1.141371, et ceux du live LCD de1.271675 à1.181579. Cette inclinaison est conservée pour la projection et la profondeur.

`provenance.json` consigne source, copie, hashs des livrables, compteurs et limites. À ce point, aucun résultat de build ou capture JUCE de cette nouvelle importation n'est revendiqué par le designer. Les sources et rendus du modèle précédent restent préservés séparément.

## Sortie UIUX consultée

## Design System: Cadenza GENOS2 reference import

### Design Dials
- **Variance:** 8/10 — Bold / Asymmetric
- **Motion:** 7/10 — Standard

### Pattern
- **Name:** Product Demo + Features
- **Conversion Focus:** Use an interactive demo only when it explains value better than static media. Provide captions, transcript, visible play/pause controls, and a non-video fallback; do not autoplay under reduced motion. Pause media when offscreen or hidden and keep the final product state available as static content.
- **CTA Placement:** Video center + CTA right/bottom
- **Color Strategy:** Video surround: Brand color overlay. Features: Icon color #0080FF. Text: Dark #222
- **Sections:** Hero > Product video/mockup (center) > Feature breakdown per section > Comparison (optional) > CTA

### Style
- **Name:** Brutalism
- **Mode Support:** Light supported | Dark supported
- **Keywords:** Raw, unpolished, stark, high contrast, plain text, default fonts, visible borders, asymmetric, anti-design
- **Best For:** Design portfolios, artistic projects, counter-culture brands, editorial/media sites, tech blogs
- **Performance:** cost:low|drivers:none | **Accessibility:** risk:low|requires:contrast-text-4.5,keyboard,visible-focus,reduced-motion

### Colors
| Role | Hex | CSS Variable |
|------|-----|--------------|
| Primary | `#DC2626` | `--color-primary` |
| On Primary | `#FFFFFF` | `--color-on-primary` |
| Secondary | `#9A3412` | `--color-secondary` |
| On Secondary | `#FFFFFF` | `--color-on-secondary` |
| Accent/CTA | `#D97706` | `--color-accent` |
| On Accent/CTA | `#000000` | `--color-on-accent` |
| Background | `#FFFBEB` | `--color-background` |
| Foreground | `#0F172A` | `--color-foreground` |
| Card | `#FFFFFF` | `--color-card` |
| Card Foreground | `#0F172A` | `--color-card-foreground` |
| Muted | `#FCF1F1` | `--color-muted` |
| Muted Foreground | `#475569` | `--color-muted-foreground` |
| Border | `#FAE4E4` | `--color-border` |
| Destructive | `#DC2626` | `--color-destructive` |
| On Destructive | `#FFFFFF` | `--color-on-destructive` |
| Ring | `#DC2626` | `--color-ring` |

*Notes: Musical red + warm amber*

### Typography
- **Heading:** Righteous
- **Body:** Poppins
- **Mood:** music, entertainment, fun, energetic, bold, performance
- **Best For:** Music platforms, entertainment, events, festivals, performers
- **Google Fonts:** https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;500;600;700&family=Righteous&display=swap
- **CSS Import:**
```css
@import url('https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;500;600;700&family=Righteous&display=swap');
```

### Key Effects
No smooth transitions (instant), sharp corners (0px), bold typography (700+), visible grid, large blocks

### Motion
**Stagger List** (Standard) — Trigger: load or scroll | Duration: 300-450ms | Easing: `back.out(1.4)`
```js
gsap.from('.grid-item', { opacity: 0, scale: 0.92, y: 16, duration: 0.4, stagger: { each: 0.06, from: 'start', grid: 'auto' }, ease: 'back.out(1.4)' });
```
*Framework notes: grid: 'auto' lets GSAP infer rows/columns from a CSS grid layout for a natural wave stagger; Use matchMedia('(prefers-reduced-motion: reduce)') to skip non-essential motion and render the final state immediately*
- ✅ Combine with from: 'center' for a bento-grid layout to draw the eye inward first
- ❌ Don't use back.out on dense data tables; the overshoot reads as sloppy on informational UI

### Avoid (Anti-patterns)
- Muted colors
- Low energy

### Pre-Delivery Checklist
- [ ] No emojis as icons (use SVG: Heroicons/Lucide)
- [ ] cursor-pointer on all clickable elements
- [ ] Hover states with smooth transitions (150-300ms)
- [ ] Light mode: text contrast 4.5:1 minimum
- [ ] Focus states visible for keyboard nav
- [ ] prefers-reduced-motion respected
- [ ] Responsive: 375px, 768px, 1024px, 1440px
