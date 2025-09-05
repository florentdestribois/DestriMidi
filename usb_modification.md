# Modification câble USB pour test batterie

## Solution : Désactiver l'alimentation USB

### Méthode 1 - Câble USB modifié
1. **Prendre un câble USB-C** vers USB-A jetable
2. **Couper le fil rouge** (5V) à l'intérieur du câble
3. **Garder les fils** :
   - Vert/Blanc : Communication série (D+/D-)
   - Noir : GND (commun)
   - Rouge : COUPÉ (pas d'alimentation)

### Méthode 2 - Modification temporaire
1. **Ouvrir** le boîtier ESP32 (si accessible)
2. **Déconnecter** temporairement le VIN du régulateur USB
3. **Alimenter** directement par VIN avec votre alim variable

### Méthode 3 - Adaptateur USB
1. **Acheter un adaptateur USB** avec coupure 5V
2. **Ou utiliser un hub USB** avec interrupteur d'alimentation