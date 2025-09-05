# SCHÉMA COMPLET - ESP32 MIDI PEDAL AVEC RÉGULATEUR LDO

```
                    USB Type-C
                        │
                       5V
                        │
    ┌─────────────────────────────────────┐
    │         TC4056 MODULE (4-PIN)       │
    │                                     │
    │  DC 5V+──── 5V                     │
    │                                     │
    │  DC 5V-──── GND                    │
    │                                     │
    │  OUT+────┬──────────────────────────│───┐
    │          │                          │   │
    │  OUT-────┼──────────────────────────│───┼─► GND commun
    │          │                          │   │
    │  B+──────┘                          │   │
    │                                     │   │
    │  B-─────────────────────────────────│───┘
    └─────────────────────────────────────┘
              │         
              │         
        ┌─────┴─────┐
        │  BATTERIE │ 3.7V LiPo
        │  2000mAh  │ JST Connector
        │    18650  │
        └───────────┘

RÉGULATEUR LDO MCP1700-3302E:

TC4056 OUT+ (3.7V-4.2V) ──┬─► C1 (10µF) ──┬─► MCP1700 IN
                          │               │
                          │               │
                          │    ┌──────────────────┐
                          │    │   MCP1700-3302E  │
                          │    │                  │
                          │    │  IN    OUT  GND  │
                          │    └──┬────────┬───┬──┘
                          │       │        │   │
                          │       │        │   └─► GND commun
                          │       │        │
                          │       │        └─► C2 (10µF) ──► ESP32 3.3V
                          │       │                           (ALIMENTATION)
                          │       │                           Performance BLE
                          │       │                           optimale !
                          │       │
                          │       └─► Non connecté
                          │
DIVISEUR DE TENSION ──────┘
POUR MONITORING:

TC4056 OUT+ (tension batterie directe)
    │
   ┌─────┐
   │ R1  │ 10kΩ (1/4W, 5%)
   │     │
   └─────┘
    │
    ├──────────► ESP32 GPIO34 (ADC input)
    │             Lecture sécurisée
    │             Vout = Vbat ÷ 2
   ┌─────┐        
   │ R2  │ 10kΩ (1/4W, 5%)
   │     │        Exemples:
   └─────┘        4.2V batt → 2.1V GPIO34
    │             3.7V batt → 1.85V GPIO34
    │             3.0V batt → 1.5V GPIO34
   GND ──────────► ESP32 GND
```

## CONNEXIONS PHYSIQUES

### TC4056 → Régulateur → ESP32
```
TC4056 OUT+  →  MCP1700 IN (+ condensateur 10µF)
MCP1700 OUT  →  ESP32 pin 3.3V (+ condensateur 10µF)
MCP1700 GND  →  ESP32 GND
TC4056 OUT-  →  GND commun
```

### Diviseur de tension (monitoring batterie)
```
TC4056 OUT+  →  R1 (10kΩ)  →  Point milieu  →  ESP32 GPIO34
                Point milieu  →  R2 (10kΩ)  →  ESP32 GND
```

### Batterie
```
Batterie JST Rouge  →  TC4056 B+
Batterie JST Noir   →  TC4056 B-
```

### USB
```
USB-C 5V  →  TC4056 DC 5V+
USB-C GND →  TC4056 DC 5V-
```

## DÉTECTION DE CHARGE
- **Pas de pin CHRG** sur ce module 4-pin
- **Détection logicielle** : tension >4.0V stable = charge probable
- **LED verte clignote** pendant la charge

## COMPOSANTS NÉCESSAIRES
- 1x TC4056 Module 4-pin avec USB-C
- 1x **MCP1700-3302E** régulateur LDO 3.3V (SOT-23 ou TO-92)
- 2x **Condensateurs 10µF** électrolytiques (stabilisation)
- 1x Batterie LiPo 3.7V 2000mAh avec connecteur JST
- 2x Résistances 10kΩ 1/4W 5% tolérance (diviseur tension)
- Fils de connexion (rouge/noir + couleurs)
- Breadboard ou PCB pour montage

## SÉCURITÉ ET AVANTAGES
✅ **SOLUTION OPTIMALE** : Régulateur MCP1700-3302E
- **Tension stable 3.3V** : performance BLE maximale
- **Protection ESP32** : pas de surtension 
- **Low-dropout** : fonctionne dès 3.7V batterie
- **500mA** : ampérage suffisant pour ESP32 + BLE
- **Efficacité** : >95% à faible charge

⚠️ **DIVISEUR DE TENSION OBLIGATOIRE** pour monitoring :
- Sans diviseur : 4.2V direct sur GPIO34 = **DANGER**
- Avec diviseur : 2.1V sur GPIO34 = **SÉCURISÉ**

## TENSIONS DE RÉFÉRENCE
| Batterie | GPIO34 | État |
|----------|--------|------|
| 4.2V     | 2.1V   | Pleine |
| 3.7V     | 1.85V  | Normale |
| 3.0V     | 1.5V   | Faible |
| <3.0V    | <1.5V  | Critique |