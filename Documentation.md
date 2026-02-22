## MorphinGrid C2 Framework: Technical Documentation
This documentation covers the architecture, features, and operational workflows of the MorphinGrid C2 system, comprising the C++ Stealth Agent (Client) and the Django Secure C2 (Server).


## 1. Architecture Overview
The system is designed as a covert Command & Control framework utilizing modern cryptography and web standards to bypass network defenses.

- Communication Protocol: HTTPS (Port 443) using WinHTTP.
- Cryptography: * Key Exchange: ECDH (Elliptic Curve Diffie-Hellman) over curve secp256r1.
- Encryption: AES-256-GCM (Authenticated Encryption).
- Database: MariaDB (Dockerized) for persistent agent and command storage.


## 2. C2 Client (C++ Agent) Features

### Stealth & Evasion
- Process Hiding: Automatically hides the console window and detaches from the parent terminal using ShowWindow(SW_HIDE) and FreeConsole().
- Sandbox Evasion: Implements a randomized initial sleep (10–40s) to outlast automated sandbox monitoring.
- Themed Traffic: Uses Power Rangers-themed URIs and high-reputation User-Agents (Chrome/Windows 10) to blend into legitimate web traffic.

### System Discovery
Upon execution, the agent gathers the following metadata:

- Unique Agent ID: A SHA-256 hash of the ComputerName + Username.
- OS Intelligence: Queries the Windows Registry for the exact ProductName and CurrentBuild.
- Privilege Detection: Checks if the process is running with Administrative tokens.
- Network Mapping: Enumerates all active IP addresses (IPv4/v6) and DNS domains.

### Cryptographic Integrity
- Dynamic Key Cycling: The agent automatically discards its AES key and performs a new ECDH handshake every 6 heartbeats.
- SSL Bypass: Configured to ignore "Unknown CA" errors, allowing it to communicate securely with self-signed C2 certificates.


## 3. C2 Server (Django) Features

### Operational Dashboard (Admin UI)
- Agent Management: Provides a real-time list of all compromised machines, including their hostname, last seen timestamp, and jitter settings.
- Automatic Merging: Sophisticated backend logic merges temporary handshake records (HS_...) into permanent agent records to prevent dashboard clutter.
- Command Queuing: A centralized interface to stage commands for agents to pick up during their next check-in.

### Heartbeat Handler
- Stateless Initial Handshake: Handles ECDH public key exchanges and returns a DER-encoded server public key.
- Encrypted Processing: Decrypts incoming JSON blobs using the agent-specific AES-256-GCM secret.
- Automatic Discovery Updates: Updates the database automatically if the agent reports a change in system state (e.g., new IP, privilege escalation).



## 4. Operational Commands

Currently supported commands via the CommandQueue interface:

- Command : update_jitter
- Description : Changes the base frequency of heartbeats.
- ValueExample : 10 (sets to 10 seconds)
















