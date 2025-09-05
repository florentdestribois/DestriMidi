# Créer une icône Dock avec Automator

## Méthode simple avec Automator :

1. **Ouvrir Automator** (Applications → Utilitaires → Automator)

2. **Choisir "Application"** comme type de document

3. **Ajouter une action** :
   - Dans la bibliothèque (à gauche), chercher "Exécuter un script shell"
   - Glisser cette action dans la zone de travail

4. **Configurer le script** :
   ```bash
   cd "/Users/florentdestribois/Documents/Devellopement d'application/DestriMidi"
   ./connect_destrimidi.sh
   ```

5. **Enregistrer** :
   - Fichier → Enregistrer
   - Nom : "DestriMidi Connector"
   - Emplacement : Bureau ou Applications

6. **Ajouter au Dock** :
   - Glisser l'app créée vers le Dock
   - L'icône sera maintenant cliquable !

## Résultat :
✅ Une icône dans le Dock qui lance automatiquement la connexion DestriMidi