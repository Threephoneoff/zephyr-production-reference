# Zephyr Production Reference

> A production-ready reference project demonstrating modern embedded firmware development with Zephyr RTOS.

---

## Why this project?

The goal of this repository is **not to build a single embedded application**, but to provide a practical reference for engineers who want to learn how to design, implement, and maintain production-quality firmware using Zephyr RTOS.

Most Zephyr examples demonstrate one subsystem in isolation. This project takes a different approach by integrating those subsystems into a cohesive application that resembles a real embedded product.

The application itself is intentionally secondary. It may begin as a simple data logger, but it will evolve over time as new Zephyr concepts are introduced. The primary objective is to demonstrate *how* production firmware is built rather than *what* it does.

Whether you are coming from bare-metal development, FreeRTOS, or a vendor SDK, this repository aims to bridge the gap between learning Zephyr and confidently using it in production.

---

## Goals

- Learn Zephyr by building a realistic application.
- Demonstrate production-ready firmware architecture.
- Explain core Zephyr concepts through practical examples.
- Follow Zephyr best practices whenever possible.
- Contribute improvements back to the Zephyr community.
- Document the entire learning journey.
- Serve as a reference for embedded engineers adopting Zephyr.

---

## Learning Roadmap

Each milestone focuses on a Zephyr concept rather than adding arbitrary application features.

| Phase | Topic |
|-------|-------|
| 1 | Zephyr Fundamentals |
| 2 | Devicetree |
| 3 | Kconfig |
| 4 | Logging |
| 5 | Shell |
| 6 | Settings |
| 7 | File System (FatFS) |
| 8 | Sensor Framework |
| 9 | Writing a Sensor Driver |
| 10 | MCUboot & Sysbuild |
| 11 | MCUmgr & Device Management |
| 12 | Networking |
| 13 | Testing & CI |

Each phase adds another building block commonly found in production embedded firmware.

---

## Planned Features

The exact application will evolve, but the repository is expected to demonstrate:

- Modular firmware architecture
- Devicetree
- Kconfig
- Zephyr Threads
- Message Queues
- Logging subsystem
- Shell subsystem
- Settings subsystem
- FatFS
- SD Card support
- Sensor API
- Custom sensor driver
- MCUboot
- Sysbuild
- MCUmgr
- Bluetooth and/or Ethernet
- Firmware updates
- Unit testing
- Continuous Integration

---

## Architecture

The project is organized around independent modules and services rather than a single monolithic application.

As the project evolves, each subsystem will become reusable and loosely coupled, demonstrating how larger embedded applications can be structured.

---

## Philosophy

This repository is organized by **concepts rather than features**.

Every milestone introduces a Zephyr subsystem, explains why it exists, demonstrates how it is used, and integrates it into a realistic firmware architecture.

The objective is to help embedded engineers understand not only *how* to use Zephyr, but also *why* its architecture enables maintainable and production-ready firmware.

---

## Getting Started

Documentation and setup instructions will be added as the project progresses.

---

## Documentation

Future documentation will cover topics such as:

- Architecture
- Devicetree
- Kconfig
- Logging
- Shell
- Settings
- File System
- Sensor Drivers
- MCUboot
- Sysbuild
- MCUmgr
- Testing

---

## Contributing

Contributions, feedback, bug reports, and discussions are welcome.

If this project helps you learn Zephyr or inspires improvements, feel free to open an issue or submit a pull request.

---

## Vision

My long-term goal is for this repository to become a practical reference for engineers adopting Zephyr RTOS, while documenting my own journey toward contributing to the Zephyr ecosystem, speaking at conferences, and helping grow the Zephyr community.