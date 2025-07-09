# Guard & Towers — C++ KI-Projekt

Dieses Projekt enthält ein visuelles GUI-Schachspiel sowie leistungsfähige Konsolen-Tools zum Testen, Benchmarken und Spielen mit verschiedenen KI-Modellen (RandomAI, AlphaBetaAI usw.).

---

## 🔧 Setup

1. **Projekt klonen:**

```bash
git clone https://git.tu-berlin.de/mm22/guard-and-towers-ai.git
cd guard-and-towers-ai
```

2. **Eigenen Branch auschecken:**

```bash
git checkout <dein_name>s_branch
```

3. **Branch `bitboard` mergen (alle aktuellen Änderungen holen):**

```bash
git merge bitboard
```

---

## 🛠️ Build & Kompilieren

```bash
cmake -B build
cmake --build build
```

---

## 🧪 Tools & Ausführen

> Alle Tools befinden sich im `build/` Verzeichnis.

### ▶️ Konsolen-Programme:

- **Benchmark** MoveGenerator-Geschwindigkeit:
  ```bash
  ./bench_movegen
  ```

- **Unit-Tests** für MoveGenerator:
  ```bash
  ./test_movegen
  ```

- **Test für bestimmte Stellungen (FENs):**
  ```bash
  ./wag
  ```

- **Spielbare Konsolen-Version (PvP, PvE, AIvAI):**
  ```bash
  ./console_game
  ```

---

### 🖼️ GUI starten

1. Wechsle ins GUI-Verzeichnis:
   ```bash
   cd build/gui
   ```

2. Starte die grafische Oberfläche:
   ```bash
   ./guard_and_towers
   ```

> In der GUI kannst du mit verschiedenen KI-Modellen spielen und deine Tests visualisieren.

---

## 📁 Struktur (Kurzfassung)

- `gui/` – C++ GUI-Seiten & UI-Komponenten (SFML)
- `src/` – Zentrale Logik, KI, Engine
- `bench/` – Benchmarks
- `tests/` – Unit-Tests

---

## 📌 Hinweis

- `build/` & `.vscode/` sind aus dem Repo ausgeschlossen (`.gitignore`)
- Achte darauf, keine build-Dateien zu pushen

---

Viel Erfolg & Spaß beim Tüfteln mit KI! 🧠