#!/bin/bash
# Script pour connecter DestriMidi au Mac
# Usage: ./connect_destrimidi.sh

echo "🎹 Connexion DestriMidi au Mac"
echo "================================"

# Ouvrir Audio MIDI Setup
echo "📱 Ouverture d'Audio MIDI Setup..."
open -a "Audio MIDI Setup"

# Attendre que l'app se lance
sleep 3

echo "✅ Audio MIDI Setup ouvert"
echo ""
echo "📋 ÉTAPES MANUELLES NÉCESSAIRES :"
echo "1. Dans Audio MIDI Setup, allez dans le menu :"
echo "   Fenêtre → Afficher la configuration MIDI Bluetooth"
echo ""
echo "2. Dans la fenêtre Bluetooth MIDI qui s'ouvre :"
echo "   👆 Cliquez sur 'DestriMidi' pour connecter"
echo ""
echo "💡 ASTUCES :"
echo "• Gardez cette fenêtre Bluetooth MIDI ouverte"
echo "• La reconnexion sera automatique aux prochains démarrages"
echo "• Votre DAW verra 'DestriMidi' comme source MIDI"
echo ""
echo "🔧 Si l'ESP32 n'apparaît pas :"
echo "• Vérifiez que le '1' s'affiche sur l'écran 7 segments"  
echo "• Redémarrez l'ESP32 si nécessaire"
echo ""
echo "🎵 Une fois connecté, testez dans votre DAW préféré !"