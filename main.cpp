#include "raylib.h"
#include <cmath>
#include <vector>

// Konstanten
const int screenWidth = 800;
const int screenHeight = 450;
const int sampleRate = 44100;
const int sampleSize = 255; // Größe des Samples für die Audioausgabe
const float speedOfLight = 299792458.0f; // Lichtgeschwindigkeit in m/s

struct Impulse {
    float amplitude;
    float frequency; // in Hz
    float offset;
    float decayRate;
    float phase;
};

std::vector<Impulse> impulses; // Vektor zur Speicherung der Impulse

// Farben mit ihren Wellenlängen
struct ColorInfo {
    Color color;
    float minWavelength; // in nm
    float maxWavelength; // in nm
};

ColorInfo colors[] = {
    { { 148, 0, 211, 255 }, 380.0f, 450.0f },  // Violett
    { { 75, 0, 130, 255 }, 450.0f, 495.0f },  // Indigo (nicht immer als eigene Farbe betrachtet)
    { { 0, 0, 255, 255 }, 450.0f, 495.0f },   // Blau
    { { 0, 255, 0, 255 }, 495.0f, 570.0f },   // Grün
    { { 255, 255, 0, 255 }, 570.0f, 590.0f }, // Gelb
    { { 255, 165, 0, 255 }, 590.0f, 620.0f }, // Orange
    { { 255, 0, 0, 255 }, 620.0f, 750.0f }    // Rot
};

// Wellenförmigen Impuls mit Dämpfung erzeugen
void GenerateDampedWaveImpulse(float* buffer, int frames, float frequency, float amplitude, float& phase, float decayRate) {
    for (int i = 0; i < frames; i++) {
        float damping = expf(-decayRate * i / frames); // Dämpfungsrate anpassen
        buffer[i] = amplitude * sinf((2.0f * PI * frequency * i / sampleRate) + phase) * damping;
    }
    phase += (2.0f * PI * frequency * frames / sampleRate);
    if (phase > 2.0f * PI) phase -= 2.0f * PI;
}

Color GetColorForWavelength(float wavelength) {
    for (const auto& color : colors) {
        if (wavelength >= color.minWavelength && wavelength <= color.maxWavelength) {
            return color.color;
        }
    }
    return WHITE; // Standardfarbe, falls keine passende Wellenlänge gefunden wird
}

int main() {
    // Raylib initialisieren
    InitWindow(screenWidth, screenHeight, "Wave Animation with Impulses and Sound");
    InitAudioDevice(); // Audiogerät initialisieren

    // Audio-Stream für Impuls-Ton
    AudioStream stream = LoadAudioStream(sampleRate, 32, 1);
    PlayAudioStream(stream);

    // Buffer für den Impuls
    float buffer[sampleSize] = { 0 };

    // Parameter für die Wellen
    float wave1Amplitude = 0.5f;
    float wave1Frequency = 0.01f;
    float wave1Speed = 5.0f;
    float wave1Offset = 0.0f;

    SetTargetFPS(60); // Bildwiederholrate auf 60 FPS setzen

    // Hauptschleife
    while (!WindowShouldClose()) {
        // Wellen-Offsets aktualisieren
        wave1Offset += wave1Speed * GetFrameTime();

        // Mausklick überprüfen und Impuls erzeugen
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            impulses.push_back({ 100.0f, 6.05e14f, 0.0f, 3.0f, 0.0f }); // Beispiel-Impulsparameter (Frequenz im sichtbaren Bereich)
        }

        // Impulse aktualisieren und in den Audio-Stream laden
        for (auto& impulse : impulses) {
            impulse.offset += wave1Speed * GetFrameTime();
            impulse.amplitude *= expf(-impulse.decayRate * GetFrameTime()); // Amplitude dämpfen

            GenerateDampedWaveImpulse(buffer, sampleSize, impulse.frequency, impulse.amplitude, impulse.phase, impulse.decayRate);
            UpdateAudioStream(stream, buffer, sampleSize);
        }

        // Abgeschlossene Impulse entfernen
        impulses.erase(std::remove_if(impulses.begin(), impulses.end(), [](const Impulse& impulse) {
            return impulse.amplitude < 0.1f; // Schwellenwert zum Entfernen der Impulse
            }), impulses.end());

        // Zeichnen
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Aktuelle Farbe basierend auf der Wellenlänge bestimmen
        float currentWavelength = 0.0f;
        if (!impulses.empty()) {
            currentWavelength = (speedOfLight / impulses.back().frequency) * 1e9f; // Frequenz in Wellenlänge umrechnen (nm)
        }
        Color boxColor = GetColorForWavelength(currentWavelength);

        // Box mit aktueller Farbe zeichnen
        DrawRectangle(10, screenHeight - 60, screenWidth - 20, 50, boxColor);
        DrawText(TextFormat("Wellenlänge: %.2f nm", currentWavelength), 10, screenHeight - 80, 20, BLACK);
        DrawText("Klicken Sie, um einen gedämpften wellenförmigen Impuls abzuspielen", 10, 10, 20, DARKGRAY);

        // Impulse zeichnen
        for (const auto& impulse : impulses) {
            for (int x = 0; x < screenWidth - 1; x++) {
                float y = screenHeight / 2 + impulse.amplitude * sin(impulse.frequency * x + impulse.offset);
                float yNext = screenHeight / 2 + impulse.amplitude * sin(impulse.frequency * (x + 1) + impulse.offset);
                DrawLine(x, y, x + 1, yNext, BLACK);
            }
        }

        EndDrawing();
    }

    // Aufräumen
    UnloadAudioStream(stream);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
