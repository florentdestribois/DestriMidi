#!/bin/bash
# Script pour créer une app macOS qui lance connect_destrimidi.sh
# Usage: ./create_dock_app.sh

APP_NAME="DestriMidi Connector"
APP_DIR="/Applications/${APP_NAME}.app"
SCRIPT_PATH="/Users/florentdestribois/Documents/Devellopement d'application/DestriMidi/connect_destrimidi.sh"

echo "🔧 Création de l'application ${APP_NAME}..."

# Créer la structure de l'app
sudo mkdir -p "${APP_DIR}/Contents/MacOS"
sudo mkdir -p "${APP_DIR}/Contents/Resources"

# Créer le fichier Info.plist
sudo tee "${APP_DIR}/Contents/Info.plist" > /dev/null <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>DestriMidiConnector</string>
    <key>CFBundleIconFile</key>
    <string>icon.icns</string>
    <key>CFBundleIdentifier</key>
    <string>com.destrimidi.connector</string>
    <key>CFBundleName</key>
    <string>${APP_NAME}</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>LSUIElement</key>
    <true/>
</dict>
</plist>
EOF

# Créer l'exécutable principal
sudo tee "${APP_DIR}/Contents/MacOS/DestriMidiConnector" > /dev/null <<EOF
#!/bin/bash
osascript -e 'tell app "Terminal" to do script "${SCRIPT_PATH}"'
EOF

# Rendre l'exécutable
sudo chmod +x "${APP_DIR}/Contents/MacOS/DestriMidiConnector"

# Créer une icône simple (optionnel)
sudo tee "${APP_DIR}/Contents/Resources/icon.icns" > /dev/null <<EOF
# Placeholder pour icône - vous pouvez remplacer par une vraie icône .icns
EOF

echo "✅ Application créée dans /Applications/"
echo "🎯 Vous pouvez maintenant :"
echo "1. Ouvrir le Finder → Applications"
echo "2. Glisser '${APP_NAME}' vers le Dock"
echo "3. Cliquer sur l'icône dans le Dock pour lancer la connexion"