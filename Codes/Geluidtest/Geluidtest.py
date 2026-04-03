import argparse
import os
import re
import time
from pathlib import Path

os.environ.setdefault("PYGAME_HIDE_SUPPORT_PROMPT", "1")

import pygame
import serial
from serial.tools import list_ports

DEFAULT_BAUD_RATE = 9600
DEFAULT_SOUND_FILE = Path(__file__).with_name("test.wav")
BASE_VOLUME = 0.00
FULL_VOLUME_DISTANCE_CM = 15.0
MAX_VOLUME = 1.0
VOLUME_BOOST = 1.35
ARDUINO_HINTS = ("arduino", "uno", "nano", "ch340", "usb serial", "cp210", "ftdi")
TEXT_LINE_PATTERNS = (
    re.compile(
        r"Potmeter:\s*(?P<pot>\d+)\s*\|\s*"
        r"Bereik:\s*(?P<range>\d+)\s*cm\s*\|\s*"
        r"Sensor 1:\s*(?P<s1>\d+)\s*cm\s*\|\s*"
        r"Sensor 2:\s*(?P<s2>\d+)\s*cm",
        re.IGNORECASE,
    ),
    re.compile(
        r"Pot:\s*(?P<pot>\d+)\s*\|\s*"
        r"Bereik:\s*(?P<range>\d+)\s*cm\s*\|\s*"
        r"S1:\s*(?P<s1>\d+)\s*cm\s*\|\s*"
        r"S2:\s*(?P<s2>\d+)\s*cm",
        re.IGNORECASE,
    ),
)


def map_distance_to_volume(distance, max_range):
    if FULL_VOLUME_DISTANCE_CM <= 0:
        return 0.0

    distance = max(0.0, float(distance))
    linear = max(0.0, 1.0 - (distance / FULL_VOLUME_DISTANCE_CM))
    boosted = (linear**0.75) * MAX_VOLUME * VOLUME_BOOST
    return min(MAX_VOLUME, boosted)


def get_available_ports():
    return list(list_ports.comports())


def print_ports(ports):
    if not ports:
        print("Geen COM-poorten gevonden.", flush=True)
        return

    print("Beschikbare poorten:", flush=True)
    for port in ports:
        print(f"{port.device} - {port.description}", flush=True)


def pick_port(ports, requested_port=None):
    if not ports:
        raise SystemExit("Geen COM-poorten gevonden. Sluit eerst je Arduino aan.")

    if requested_port:
        requested_port = requested_port.upper()
        for port in ports:
            if port.device.upper() == requested_port:
                return port.device
        print_ports(ports)
        raise SystemExit(
            f"{requested_port} niet gevonden. Gebruik --list-ports om te kijken wat wel bestaat."
        )

    matches = []
    for port in ports:
        description = f"{port.description} {getattr(port, 'manufacturer', '')} {port.hwid}".lower()
        if any(hint in description for hint in ARDUINO_HINTS):
            matches.append(port)

    if matches:
        if len(matches) > 1:
            print(
                f"Meerdere Arduino-poorten gevonden, ik gebruik automatisch {matches[0].device}.",
                flush=True,
            )
        return matches[0].device

    if len(ports) == 1:
        print(
            f"Geen expliciete Arduino-naam gevonden, ik gebruik de enige poort: {ports[0].device}.",
            flush=True,
        )
        return ports[0].device

    print_ports(ports)
    raise SystemExit("Kon geen Arduino-poort kiezen. Start opnieuw met --port COMx.")


def open_serial_connection(port, baud_rate):
    try:
        ser = serial.Serial(port, baud_rate, timeout=1)
    except serial.SerialException as exc:
        raise SystemExit(
            f"Kan COM-poort {port} niet openen: {exc}\n"
            "Sluit Serial Monitor, Serial Plotter of andere programma's die de poort gebruiken."
        ) from exc

    # Een Uno reset meestal kort zodra de seriele poort open gaat.
    time.sleep(2)
    ser.reset_input_buffer()
    ser.readline()
    return ser


def init_audio(sound_file):
    sound_path = Path(sound_file)
    if not sound_path.exists():
        raise SystemExit(f"Geluidsbestand niet gevonden: {sound_path}")

    try:
        pygame.mixer.init(frequency=44100)
        pygame.mixer.set_num_channels(8)
        sound = pygame.mixer.Sound(str(sound_path))
    except pygame.error as exc:
        raise SystemExit(
            f"Kon pygame-audio niet starten: {exc}\n"
            "Controleer of je een actieve audio-uitvoer hebt en of pygame-ce is geinstalleerd."
        ) from exc

    base_channel = sound.play(loops=-1)
    sensor_channel = sound.play(loops=-1)

    if base_channel is None or sensor_channel is None:
        pygame.mixer.quit()
        raise SystemExit("Kon de audiokanalen niet starten.")

    base_channel.set_volume(BASE_VOLUME)
    sensor_channel.set_volume(0.0)
    return base_channel, sensor_channel


def run_speaker_test(base_channel, sensor_channel, seconds=2.0, volume=1.0):
    print(f"Luidsprekertest: {seconds:.1f}s op volume {volume:.2f}.", flush=True)
    base_channel.set_volume(volume)
    sensor_channel.set_volume(0.0)
    pygame.time.wait(int(seconds * 1000))
    base_channel.set_volume(BASE_VOLUME)
    sensor_channel.set_volume(0.0)


def parse_sensor_line(line):
    stripped = line.strip()
    if not stripped:
        return None

    parts = [part.strip() for part in stripped.split(",")]
    if len(parts) == 4:
        try:
            return tuple(int(part) for part in parts)
        except ValueError:
            pass

    for pattern in TEXT_LINE_PATTERNS:
        match = pattern.search(stripped)
        if match:
            return (
                int(match.group("pot")),
                int(match.group("range")),
                int(match.group("s1")),
                int(match.group("s2")),
            )

    return None


def run(
    port=None,
    baud_rate=DEFAULT_BAUD_RATE,
    sound_file=DEFAULT_SOUND_FILE,
    max_updates=None,
    speaker_test=False,
):
    ports = get_available_ports()
    print_ports(ports)
    chosen_port = pick_port(ports, port)
    print(f"Verbinden met {chosen_port} op {baud_rate} baud.", flush=True)

    ser = open_serial_connection(chosen_port, baud_rate)
    base_channel, sensor_channel = init_audio(sound_file)
    if speaker_test:
        run_speaker_test(base_channel, sensor_channel)
    valid_updates = 0

    try:
        while True:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if not line:
                continue

            print(f"SERIAL: {line}", flush=True)

            parsed = parse_sensor_line(line)
            if parsed is None:
                print(f"Regel overgeslagen: {line}", flush=True)
                continue

            pot_value, bereik, afstand1, afstand2 = parsed
            vol1 = map_distance_to_volume(afstand1, bereik)
            vol2 = map_distance_to_volume(afstand2, bereik)
            volume = vol2
            sensor_boost = max(0.0, volume - BASE_VOLUME)

            base_channel.set_volume(BASE_VOLUME)
            sensor_channel.set_volume(sensor_boost)
            valid_updates += 1

            print(
                f"pot={pot_value} bereik={bereik} s1={afstand1} v1={vol1:.2f} "
                f"s2={afstand2} v2={vol2:.2f} base={BASE_VOLUME:.2f} boost={sensor_boost:.2f} volume={volume:.2f} bron=sensor2",
                flush=True,
            )

            if max_updates is not None and valid_updates >= max_updates:
                print(
                    f"Testmodus klaar na {valid_updates} geldige updates.", flush=True
                )
                break
    except KeyboardInterrupt:
        print("Gestopt door gebruiker.", flush=True)
    finally:
        ser.close()
        pygame.mixer.quit()


def build_parser():
    parser = argparse.ArgumentParser(
        description="Lees Arduino-sensordata en stuur het volume van twee audiokanalen aan."
    )
    parser.add_argument(
        "--port", help="Bijvoorbeeld COM7. Laat leeg voor automatische detectie."
    )
    parser.add_argument(
        "--baud", type=int, default=DEFAULT_BAUD_RATE, help="Seriele baudrate."
    )
    parser.add_argument(
        "--sound",
        default=str(DEFAULT_SOUND_FILE),
        help="Pad naar het WAV-bestand dat in een lus moet spelen.",
    )
    parser.add_argument(
        "--max-updates",
        type=int,
        help="Stop automatisch na dit aantal geldige seriele updates. Handig om te testen.",
    )
    parser.add_argument(
        "--list-ports",
        action="store_true",
        help="Toon alleen de beschikbare COM-poorten en stop daarna.",
    )
    parser.add_argument(
        "--speaker-test",
        action="store_true",
        help="Speel test.wav eerst 2 seconden hoorbaar af om je audio-uitvoer te controleren.",
    )
    return parser


def main():
    parser = build_parser()
    args = parser.parse_args()

    if args.list_ports:
        print_ports(get_available_ports())
        print("test")
        return

    run(
        port=args.port,
        baud_rate=args.baud,
        sound_file=args.sound,
        max_updates=args.max_updates,
        speaker_test=args.speaker_test,
    )


if __name__ == "__main__":
    main()
