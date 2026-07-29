# s0P0wn3d — C2 Framework

Projet academique EPITECH. Framework de Command & Control developpe pour simuler des intrusions furtives et evaluer les capacites de detection des SOC.

Usage exclusivement pedagogique et en environnement autorise.

## Architecture

- **Controller** : serveur central pour interagir avec les implants
- **Agent** : implant leger Windows, communication chiffree avec le controller
- **Web App** : interface de pilotage

## Fonctionnalites

- Shell interactif sans processus visible
- DLL Hijacking pour elevation de privileges
- Syscalls directs pour contournement EDR
- Chiffrement du trafic C2
- Keylogging, enumeration locale, exfiltration
- Pass-the-Hash, propagation laterale
- Brute force de hashs locaux

## Contraintes

Developpe sans framework d'exploitation automatise (pas de Metasploit, Mimikatz, Empire). Tout est implemente manuellement avec des librairies bas niveau (reseautage, cryptographie, APIs Windows).

## Environnement de test

Windows 10/11, Azure VM, lab isole.

## Avertissement

Projet strictement educatif. Ne pas deployer hors d'un environnement autorise.
