# ⚡ xLog

> **An Unapologetically Over-Engineered Life Gamification Engine**

`xLog` is a fast, terminal-native life gamification engine written in C++20. It transforms personal growth into a structured RPG system by organizing life into 4 core Domains (each containing 4 Subdomains), dynamic task scheduling, flow-state difficulty calibration, and a harmonic mean rating system that penalizes domain neglect while rewarding consistency.

---

## 🏛️ Core Philosophy: 4 Domains × 4 Subdomains

Standard task managers treat life as a flat, unorganized checklist. `xLog` is built around holistic balance across four fundamental dimensions:

1. **4 Core Domains**: Broad categories defining your life pillars (default: *Faith*, *Intellect*, *Physique*, *Artistry*).
2. **4 Subdomains per Domain**: Specialized skills or focus areas within each domain (16 subdomains total, e.g., *Qur'an*, *Heuristics*, *Strength*, *Sketching*).
3. **Dual Subdomain Attribution**: Every task contributes **70% of its XP** to its Major Subdomain and **30% of its XP** to its Minor Subdomain.
4. **Harmonic Mean Global Rating**: Global Rating is computed using the **Harmonic Mean** of all 4 domain scores:
 
   $$\text{Rating} = \frac{4}{\sum_{i=1}^4 \frac{1}{\max(1.0, \text{Domain Score}_i)}}$$
   
   *Why Harmonic Mean?* The harmonic mean is heavily weighted toward your lowest score. Neglecting a single domain will pull down your overall rank, encouraging well-rounded development.

---

## 🛠️ Build & Quickstart Installation

### Prerequisites
- **C++20 Compiler** (GCC 11+, Clang 13+, or MSVC 2022+)
- **CMake** 3.24 or higher
- **SQLite3** development library (`libsqlite3-dev` on Debian/Ubuntu, `sqlite` on Arch/macOS)

### Building from Source

```bash
# 1. Clone the repository
git clone https://github.com/rbwkai/xLog.git
cd xLog

# 2. Configure with CMake
cmake -B build -S .

# 3. Build the binary
cmake --build build
```

The compiled binary will be placed at `./build/xlog`. You can symlink or copy it to your executable path:
```bash
cp build/xlog ~/.local/bin/xlog
```

---

## 📋 Task Types & Lifecycle

- **One-Time**: Single action items. Automatically archived upon completion.
- **Periodic**: Tasks that repeat after a designated interval of days (e.g., every 3 days).
- **Recurring**: Tasks scheduled for specific weekdays (e.g., Mon, Wed, Fri).
- **Hobby**: Optional side quests (`xlog bored`). Never generate overdue pressure or penalties.

### Task Statuses
- **`active`**: Scheduled and tracked normally by the engine.
- **`paused`**: Toggled via `xlog pause <task>`. Paused tasks:
  - Do **not appear** in `xlog today`.
  - Do **not accrue debt** or overdue priority penalties.
  - Can be resumed at any time without retroactive penalties.
- **`archived`**: Completed one-time tasks.

---

## ⚡ Mathematical XP Engine & Multipliers

### Base XP Formula
$$\text{Raw XP} = \min(\text{Time Spent in Minutes}, 3.0 \times \text{Difficulty})$$
*1 minute of intended work = 1 Base XP.*

### XP Multipliers
- **🐸 Eat the Frog Multiplier (1.5x – 4.0x)**: Completing your highest-priority daily task ("The Frog") rolls a triangular probability distribution yielding **1.5x to 4.0x XP**.
- **⭐ Critical Hit System (1.3x – 1.8x)**: Standard tasks have an 8% base chance for a Critical Hit. Features a soft pity system after 15 non-crits and hard pity at 30 non-crits.
- **🔥 Streak Multiplier ($1.0 + 0.12 \ln(1 + \text{Streak}/30)$)**: Logarithmically rewards daily consistency.
- **⚖️ Domain Balance Bonus (Up to 1.4x)**: Grants extra XP when completing tasks in domains lagging behind your global average.

---

## 📊 Ratings, Ranks & Decay

### Subdomain Score
$$\text{Score}_{\text{sub}} = 850 \times \log\left(1 + \frac{\text{XP}_{\text{eff}}}{1000}\right)$$

### Rank Tiers
| Rank | Rating Threshold | Badge Color |
| :--- | :--- | :--- |
| **Gray** | 0 – 499 | Gray |
| **Green** | 500 – 999 | Green |
| **Cyan** | 1000 – 1499 | Cyan |
| **Blue** | 1500 – 1999 | Blue |
| **Violet** | 2000 – 2499 | Violet |
| **Orange** | 2500 – 2999 | Orange |
| **Red** | 3000+ | Red |

### 🍂 Soft Rust Decay
Inactive subdomains experience exponential decay toward a 70% floor over a 90-day time constant:

$$\text{XP}_{\text{eff}} = \text{XP}_{\text{raw}} \times \max\left(0.70, e^{-\frac{\text{Days Inactive}}{90}}\right)$$

---

## 🎯 Dynamic Priority & Flow Calibration

- **Dynamic Priority**: Automatically elevates tasks based on base priority (70%), overdue debt (19%), domain imbalance (8%), and task staleness (3%).
- **Flow Channel Calibration**: Tracks task completion rate ($\text{CR}_{\text{ema}}$) targeting **70%–85%**. Difficulty dynamically adjusts to keep you challenged without causing burnout.

---

## 💻 CLI & TUI Command Reference

All screens strictly adhere to a visual **44-column Lavender boxed container UI standard**.

| Command | Usage | Description |
| :--- | :--- | :--- |
| `xlog prompt` | `xlog prompt` | Fastfetch-style status badge, rank progress & quote |
| `xlog today` | `xlog today` | View today's scheduled tasks |
| `xlog add` | `xlog add [name]` | Create task (`xlog add <name>` for defaults, or `xlog add` for interactive) |
| `xlog done` | `xlog done [name] [min]` | Mark task complete and earn XP |
| `xlog edit` | `xlog edit [name]` | Interactively edit task parameters & priority |
| `xlog pause` | `xlog pause [name]` | Pause or resume a task |
| `xlog tui` | `xlog tui` | Interactive prompt-based TUI menu |
| `xlog profile` | `xlog profile` | Ranks, domain scores & progress heatmap |
| `xlog bored` | `xlog bored` | View hobby & side quest tasks |
| `xlog quick` | `xlog quick` | View tasks under 15 minutes |
| `xlog why` | `xlog why` | Summary of engine math & mechanics |
| `xlog quote` | `xlog quote [add\|list]` | Display or manage motivational quotes |
| `xlog setup` | `xlog setup` | Re-run initial domain setup |
| `xlog help` | `xlog help` | Display 44-column boxed command reference |

---

## 📄 License

This project is open source under the [MIT License](LICENSE).
