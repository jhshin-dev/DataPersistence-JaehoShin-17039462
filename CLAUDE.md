# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**DataPersistence** is a C++ PoC for the "S-Semi" semiconductor sample production order management system.

**PoC scope**: Implement JSON-based data persistence (CRUD) — save and load domain data to/from a file or embedded DB.  
The broader system (multi-role console UI, production-line simulation, order-status transitions) is **out of scope** for this PoC.

## Build

**IDE**: Visual Studio 2022, solution file `DataPersistence.slnx`.

**Command-line build** (run from the repo root):

```powershell
# Debug x64 (default for development)
msbuild DataPersistence.slnx /p:Configuration=Debug /p:Platform=x64

# Release x64
msbuild DataPersistence.slnx /p:Configuration=Release /p:Platform=x64
```

**Run** the compiled binary:

```powershell
.\x64\Debug\DataPersistence.exe
```

**Compiler**: MSVC v145 (VS 2022 toolset), **C++20** (`/std:c++20`), Unicode character set, SDL checks enabled.  
No external build tool (CMake, vcpkg) is configured — all dependencies must be header-only or added via the `.vcxproj`.

## Domain Model (PoC scope)

### Core entities

| Entity | Key fields |
|---|---|
| `Sample` | id, name, spec, stock quantity |
| `Order` | id, sampleId, quantity, status, timestamps |

### Order status flow

```
RESERVED → (승인) → CONFIRMED (재고 충분) / PRODUCING (재고 부족)
         → (거절) → REJECTED
PRODUCING → (생산 완료) → CONFIRMED
CONFIRMED → RELEASED
```

`REJECTED` is a terminal error state and is excluded from normal monitoring.

### Roles (background context, not implemented in PoC)

- **고객(Customer)**: requests samples via email
- **주문 담당자(Order Manager)**: creates and tracks orders
- **생산 담당자(Production Manager)**: registers samples, approves/rejects orders

## Architecture for the PoC

The PoC demonstrates a **Repository pattern** over a JSON file:

```
main.cpp
  └─ Repository<T>          (generic CRUD interface)
       └─ JsonRepository<T> (reads/writes data.json via a JSON library)
            └─ SampleRepository  : JsonRepository<Sample>
            └─ OrderRepository   : JsonRepository<Order>
```

**Recommended JSON library**: `nlohmann/json` (single-header, no build configuration needed — drop `json.hpp` into the source tree and `#include` it).

### Key design constraints

- Each repository owns one JSON file (e.g., `samples.json`, `orders.json`).
- Load-on-open, save-on-write — no lazy flushing needed for the PoC.
- IDs are assigned by the repository (auto-increment or UUID string).
- `OrderStatus` is serialized as a string (`"RESERVED"`, `"PRODUCING"`, etc.).
