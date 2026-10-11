## Design System: Cadenza reference model import

### Design Dials
- **Variance:** 8/10 — Bold / Asymmetric
- **Motion:** 7/10 — Standard
- **Density:** 8/10 — Dense / Dashboard

### Pattern
- **Name:** Hero + Features + CTA
- **Conversion Focus:** Deep CTA placement. For CTA label text, verify at least 4.5:1 against the button fill; use 7:1 only when the product explicitly targets AAA normal-text contrast. Keep focus and component boundaries independently visible. Disable hero parallax under reduced motion and render its static final state.
- **CTA Placement:** Hero (sticky) + Bottom
- **Color Strategy:** Hero: Brand primary or vibrant. Features: Card bg #FAFAFA. CTA: Contrasting accent color
- **Sections:** Hero with headline/image > Value prop > Key features (3-5) > CTA section > Footer

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
- **Heading:** Outfit
- **Body:** Work Sans
- **Mood:** geometric, modern, clean, balanced, contemporary, versatile
- **Best For:** General purpose, portfolios, agencies, modern brands, landing pages
- **Google Fonts:** https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;500;600;700&family=Work+Sans:wght@300;400;500;600;700&display=swap
- **CSS Import:**
```css
@import url('https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;500;600;700&family=Work+Sans:wght@300;400;500;600;700&display=swap');
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

### Pre-Delivery Checklist
- [ ] No emojis as icons (use SVG: Heroicons/Lucide)
- [ ] cursor-pointer on all clickable elements
- [ ] Hover states with smooth transitions (150-300ms)
- [ ] Light mode: text contrast 4.5:1 minimum
- [ ] Focus states visible for keyboard nav
- [ ] prefers-reduced-motion respected
- [ ] Responsive: 375px, 768px, 1024px, 1440px


## Décision d’adaptation native

Demande du 11 octobre 2026 : utiliser le fichier Yamaha_Genos2.blend fourni par Marco. Le concept est de jouer et éditer un arrangement Cadenza à partir de cette façade physique fidèle. Références réellement vues : rendus/01_Perspective_hero.png (volumes, écran incliné, texture) et rendus/02_Dessus_orthographique.png (implantation et marquages). Ce sont les deux angles de la source choisie par Marco, pas une nouvelle proposition de marque.

Gardé de UIUX : densité assumée, retour immédiat, focus et commandes accessibles stables. Rejeté : landing Hero/CTA, Brutalism, couleur rouge dominante, Outfit/Work Sans et stagger GSAP, car cette demande importe un modèle dans un outil musical JUCE existant. Palette et marquages proviennent du modèle fourni. Les fontes natives existantes sont conservées pour ne pas changer le produit lors de cet import. Il n’y a ni DOM, ni CSS/font-display, ni React/Motion : mouvement directement piloté par les gestes (touches, curseurs, encodeurs, joystick, orientation et zoom), sans animation d’entrée qui retarderait l’interaction musicale. Une mise en page mobile390 n’est pas prévue pour ce binaire desktop; la taille minimale reste900×650.

21st mcp__21st__search a été appelé gratuitement le11octobre avec query=3D hardware synthesizer control panel texture, type=component, limit=2. Résultats métadonnées Color Panels17968 et Dither Button13542 : rejetés, composants React de décoration sans rapport avec le modèle Blender explicitement choisi. Aucun code payant récupéré, aucune bibliothèque web ajoutée. La revue designer compare le vrai composant et la fenêtre native à la source choisie.

Commandes : search.py --help puis search.py native desktop musical arranger faithful hardware model direct controls dense --design-system -p Cadenza reference model import --variance8 --motion7 --density8 -f markdown (arguments séparés dans l’appel réel). Sortie intégrale du générateur conservée ci-dessus.

Guide/source SHA256 : AGI87CB1062D3EC803EDBDF097D2D87E98046AA637E392D9A682DC8659DDA9FD317 ; direction-artistiqueCA94D02B33137B959888736FC44F03296D12B67353BAE6B74C1B4CE14FB40888 ; UIUXsearch.py8373E2DD2D560D9853EC116140DE0E0D5BEE45A6ABE5E041173C84E1C36A7F87. Versions AGI11octobre-v8 / direction11octobre-v1. Les guides de direction et leurs workflow/revue/outils ont été lus; UIUX et principes Apple native avaient déjà été lus lors de la façade précédente. Cette itération n’ajoute aucune police ni moteur web et n’emploie pas le modèle de site Atelier comme référence concurrente à l’asset explicite de Marco.
