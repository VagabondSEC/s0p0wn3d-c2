# 🦠 VIROLOGY – s0P0wn3d

> **Handle with care.**
> An academic offensive security project focused on stealthy intrusion simulation and SOC response evaluation.

---

## 📌 Overview

**s0P0wn3d** is an academic Command & Control (C2) framework developed as part of the **VIROLOGY** project at **EPITECH**.
The objective of this project is to design and implement a **stealth-oriented offensive intrusion tool** used exclusively for **authorized penetration testing and defensive readiness assessment**.

The tool simulates long-term covert intrusions in order to evaluate the detection, alerting, and response capabilities of Security Operations Centers (SOCs) within enterprise environments.

⚠️ **This project is strictly educational. Any misuse outside an explicit legal and written authorization is prohibited.**

---

## 🎯 Objectives

* Design a **resilient and stealthy C2 architecture**
* Maintain **low operational footprint** on target systems
* Support **long-lived intrusions**
* Avoid reliance on automated exploitation frameworks
* Demonstrate understanding of offensive techniques from first principles

---

## 🧠 Architecture

`s0P0wn3d` follows a classic **C2 model**:

* **Controller (Server)**
  Central command node used to monitor and interact with compromised hosts.

* **Agent (Client)**
  Lightweight implant running on the target system, communicating covertly with the controller.

The architecture emphasizes:

* Minimal process creation
* Discrete network activity
* Modular command handling
* Fault tolerance and persistence

---

## 🛠️ Core Features

The C2 system supports (or is designed to support) the following capabilities:

* Remote command execution
* Pseudo-shell interaction using built-in system commands
* Covert communication channels
* Credential access simulation
* File discovery and controlled exfiltration
* Local system enumeration
* Persistence and resilience mechanisms

---

## 📜 Supported Commands

Once operational, the framework may expose remote commands such as:

| Command           | Description                                             |
| ----------------- | ------------------------------------------------------- |
| `shell`           | Opens a pseudo-shell without spawning visible processes |
| `keylog`          | Start, stop, or dump keystroke logs                     |
| `rdp`             | Enable or disable Remote Desktop on the target          |
| `loot`            | Locate and exfiltrate sensitive configuration files     |
| `privesc`         | Scan for potential local privilege escalation vectors   |
| `propagate`       | Attempt controlled lateral movement                     |
| `crack`           | Launch local password hash cracking                     |
| `pth`             | Perform Pass-The-Hash authentication                    |
| `syscall`         | Execute system calls directly from userland             |
| `phish`           | Simulate phishing email propagation                     |
| `whateveryouwant` | Experimental / custom extensions                        |

> Commands may accept parameters and evolve as the project progresses.

---

## 🧪 Testing Environment

The project is validated using:

* **Target OS**: Windows
* **Infrastructure**: Microsoft Azure virtual machine
* **Context**: Authorized lab environment with provided credentials

All tests are conducted in compliance with the project’s ethical and legal constraints.

---

## 🚫 Restrictions

To ensure learning integrity and stealth discipline:

* ❌ No automated exploitation frameworks (Metasploit, Mimikatz, Empire, etc.)
* ❌ No third-party malware generators
* ✅ Low-level libraries are allowed (networking, cryptography, OS APIs)
* ✅ All functionality must be implemented manually

---

## ⚖️ Legal & Ethical Notice

This project is developed **strictly for educational purposes**.

You must **never**:

* Deploy this tool on systems you do not own
* Operate without explicit written authorization
* Use it outside legal penetration testing contexts

Violations may lead to severe legal consequences and professional sanctions.

---

## 📚 Educational Value

Through this project, students gain hands-on experience with:

* C2 design principles
* Stealth tradecraft
* Defensive evasion awareness
* Windows internals
* Offensive security methodology
* SOC detection challenges

---

## 🏁 Status

**Version:** v1.1
**Project:** VIROLOGY
**Institution:** EPITECH

---

Handle responsibly. Stay curious. Leave fewer traces than footprints in snow.
