import argparse
import os
import time
from pathlib import Path

os.environ.setdefault("PYGAME_HIDE_SUPPORT_PROMPT", "1")

import pygame
import serial
from serial.tools import list_ports

BAUD_RATE = 9600
SOUND_FOLDER = Path(__file__).parent

ARDUINO_HINTS = ("arduino", "uno", "nano", "ch340", "usb serial", "cp210", "ftdi")

SENSOR_NOTES = [
    ["C4", "E4", "G4", "C5"],
    ["C5", "E5", "G5", "C6"],
    ["C6", "E6", "G6", "C7"],
]


def print_ports(ports):
    print("Beschikbare poorten:")
    for port in ports:
        print(f"{port.device} - {port.description}")


def kies_poort(requested_port=None):
    ports = list(list_ports.comports())

    if not ports:
        raise SystemExit("Geen COM-poorten gevonden. Sluit je Arduino aan.")

    print_ports(ports)

    if requested_port:
        return requested_port

    for port in ports:
        description = f"{port.description} {port.hwid}".lower()
        if any(hint in description for hint in ARDUINO_HINTS):
            return port.device

    if len(ports) == 1:
        return ports[0].device

    raise SystemExit(
        "Geen Arduino-poort automatisch gevonden. Gebruik bijvoorbeeld: python script.py --port COM4"
    )


def open_serial(port):
    try:
        ser = serial.Serial(port, BAUD_RATE, timeout=1)
    except serial.SerialException as exc:
        raise SystemExit(
            f"Kan {port} niet openen.\n"
            "Sluit Serial Monitor, Serial Plotter of andere programma's die de poort gebruiken.\n"
            f"Fout: {exc}"
        )

    time.sleep(2)
    ser.reset_input_buffer()
    return ser


def load_sounds():
    pygame.mixer.init(frequency=44100)
    pygame.mixer.set_num_channels(16)

    sounds = {}

    for notes in SENSOR_NOTES:
        for note in notes:
            if note in sounds:
                continue

            path = SOUND_FOLDER / f"{note}.wav"

            if not path.exists():
                raise SystemExit(f"Geluidsbestand ontbreekt: {path}")

            sounds[note] = pygame.mixer.Sound(str(path))

    return sounds


def parse_line(line):
    parts = line.strip().split(",")

    if len(parts) != 5:
        return None

    try:
        pot = int(parts[0])
        bereik = int(parts[1])
        s1 = int(parts[2])
        s2 = int(parts[3])
        s3 = int(parts[4])
        return pot, bereik, s1, s2, s3
    except ValueError:
        return None


def afstand_naar_noot_index(afstand, bereik, aantal_noten):
    if bereik < 1:
        bereik = 1

    afstand = max(0, min(afstand, bereik))

    index = round((1 - afstand / bereik) * (aantal_noten - 1))

    return max(0, min(index, aantal_noten - 1))


def run(port=None):
    gekozen_poort = kies_poort(port)
    print(f"Verbinden met {gekozen_poort}...")

    ser = open_serial(gekozen_poort)
    sounds = load_sounds()

    current_notes = [None, None, None]
    channels = [None, None, None]

    try:
        while True:
            line = ser.readline().decode("utf-8", errors="ignore").strip()

            if not line:
                continue

            parsed = parse_line(line)

            if parsed is None:
                print(f"Overgeslagen: {line}")
                continue

            pot, bereik, s1, s2, s3 = parsed
            afstanden = [s1, s2, s3]

            for i in range(3):
                afstand = afstanden[i]

                # Niets gedetecteerd of te ver weg = geen geluid
                if afstand >= 70:
                    if channels[i] is not None:
                        channels[i].stop()
                        channels[i] = None
                        current_notes[i] = None
                    continue

                note_index = afstand_naar_noot_index(
                    afstand, bereik, len(SENSOR_NOTES[i])
                )
                note = SENSOR_NOTES[i][note_index]

                if note != current_notes[i]:
                    if channels[i] is not None:
                        channels[i].stop()

                    channels[i] = sounds[note].play(loops=-1)
                    current_notes[i] = note

                if channels[i] is not None:
                    channels[i].set_volume(1.0)

            print(
                f"Bereik={bereik} cm | "
                f"S1={s1} cm -> {current_notes[0]} | "
                f"S2={s2} cm -> {current_notes[1]} | "
                f"S3={s3} cm -> {current_notes[2]}"
            )

    except KeyboardInterrupt:
        print("Gestopt.")

    finally:
        ser.close()
        pygame.mixer.quit()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", help="Bijvoorbeeld COM4")
    args = parser.parse_args()

    run(args.port)


if __name__ == "__main__":
    main()
